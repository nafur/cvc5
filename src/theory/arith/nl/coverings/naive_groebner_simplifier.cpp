#include "theory/arith/nl/coverings/naive_groebner_simplifier.h"

#include "base/check.h"
#include "base/output.h"
#include "theory/arith/nl/coverings/cocoa_converter.h"
#include "theory/arith/nl/poly_conversion.h"
#include "util/bitvector.h"

namespace cvc5::theory::arith::nl::coverings {

#if defined(CVC5_USE_POLY) && defined(CVC5_USE_COCOA)

struct NaiveGroebnerSimplifier::NGSState
{
  /** Converter between libpoly and CoCoA */
  CoCoAConverter d_converter;
  /** Maps variables between cvc5 and libpoly */
  VariableMapper d_vm;
  /** Collects all libpoly variables we have seen */
  poly::VariableCollector d_vc;

  /**
   * Stores all equalities from the input. Used to easily construct all lemma
   * substitutions, as the set of all equalities is the premise for all
   * simplifications.
   */
  std::vector<Node> d_allEqualities;

  /** Stores the polynomials from all equalities. */
  std::vector<poly::Polynomial> d_equalities;

  /**
   * Stores the original assertion, polynomials and kind of all inequalities.
   * If the assertion is negated, the negation is put into the kind.
   */
  std::vector<std::tuple<Node, poly::Polynomial, Kind>> d_inequalities;

  /**
   * Retrieve the libpoly polynomial from an assertion. For any `lhs ~ rhs`
   * (with `~` being any relational operator) return `lhs - rhs` converted to a
   * libpoly polynomial.
   */
  poly::Polynomial getPolynomial(TNode assertion)
  {
    return as_poly_polynomial(assertion[0], d_vm)
           - as_poly_polynomial(assertion[1], d_vm);
  }

  /**
   * Initialize the members of this class from the input assertions.
   * Afterwards, d_vc has all the variables, all equalities are in both
   * d_allEqualities and d_equalities and all inequalities are in
   * d_inequalities.
   */
  void loadAssertions(const std::vector<Node>& inputs)
  {
    for (const auto& input : inputs)
    {
      Trace("nl-cov::ngs::debug")
          << "Load input assertion " << input << std::endl;
      if (input.getKind() == Kind::EQUAL)
      {
        // equalities are simple
        d_allEqualities.emplace_back(input);
        d_equalities.emplace_back(getPolynomial(input));
        d_vc(d_equalities.back());
        Trace("nl-cov::ngs::debug")
            << "-> equality " << d_equalities.back() << " = 0" << std::endl;
        continue;
      }
      bool negate = input.getKind() == Kind::NOT;
      TNode a = (negate ? input[0] : input);
      Kind kind = a.getKind();
      // push negation into the kind
      if (negate)
      {
        switch (kind)
        {
          case Kind::LT: kind = Kind::GEQ; break;
          case Kind::LEQ: kind = Kind::GT; break;
          case Kind::EQUAL: kind = Kind::DISTINCT; break;
          case Kind::GEQ: kind = Kind::LT; break;
          case Kind::GT: kind = Kind::LEQ; break;
          default: Assert(false); break;
        }
      }
      d_inequalities.emplace_back(input, getPolynomial(a), kind);
      d_vc(std::get<1>(d_inequalities.back()));
      Trace("nl-cov::ngs::debug")
          << "-> inequality " << std::get<1>(d_inequalities.back()) << " " << kind
          << " 0" << std::endl;
    }
  }

  /**
   * Compute a reduced Gröbner basis from the equalities in d_equalities.
   * First builds the polynomial ring QQ[...] with all variables from d_vc,
   * then constructs the ideal I from all equalities and finally compute the
   * reduced Gröbner basis of the ideal. Returns the tuple of the ring, the
   * ideal and the Gröbner basis.
   */
  auto computeGBasis()
  {
    CoCoA::ring ring = d_converter.mkPolyRing(d_vc.get_variables());
    Trace("nl-cov::ngs::debug") << "Working in " << ring << std::endl;

    std::vector<CoCoA::RingElem> cpolys;
    for (const auto& p : d_equalities)
    {
      Trace("nl-cov::ngs::debug") << "Adding equality " << p << std::endl;
      cpolys.emplace_back(d_converter(p, ring));
    }
    auto ideal = CoCoA::ideal(cpolys);
    Trace("nl-cov::ngs::debug") << "Modulo " << ideal << std::endl;
    auto gbasis = CoCoA::ReducedGBasis(ideal);
    Trace("nl-cov::ngs::debug") << "With GB " << gbasis << std::endl;
    return std::make_tuple(ring, ideal, gbasis);
  }
};

NaiveGroebnerSimplifier::NaiveGroebnerSimplifier(Env& env) : EnvObj(env) {}

NaiveGroebnerSimplifier::~NaiveGroebnerSimplifier() {}

void NaiveGroebnerSimplifier::reset(const std::vector<Node>& inputs)
{
  d_inputs = inputs;
  d_simplified.clear();
  d_conflict.reset();
  d_lemmaSubstitutions.clear();
  d_state = std::make_unique<NGSState>();
  try
  {
    simplify();
  }
  catch (const CoCoA::ErrorInfo& ei)
  {
    Trace("nl-cov::ngs") << "CoCoAError: " << ei << std::endl;
    Assert(false);
  }
  d_state.reset();
}

void NaiveGroebnerSimplifier::simplify()
{
  // Initialize the state
  d_state->loadAssertions(d_inputs);

  if (d_state->d_equalities.empty())
  {
    Trace("nl-cov::ngs") << "No equalities present, we can't do anything." << std::endl;
    d_simplified = d_inputs;
    return;
  }

  auto [ring, ideal, gbasis] = d_state->computeGBasis();

  // Now store the simplified equalities. Take all polynomials from the Gröbner
  // basis, construct p = 0 and use it as simplification.
  auto* nm = NodeManager::currentNM();
  Node allEqs = nm->mkAnd(d_state->d_allEqualities);
  for (const auto& poly : gbasis)
  {
    Node p = as_cvc_polynomial(d_state->d_converter(poly), d_state->d_vm);
    Node eq = nm->mkNode(Kind::EQUAL, p, nm->mkConstReal(0));
    if (!addSimplification(eq, allEqs))
    {
      // Found a conflict
      return;
    }
  }

  // Prepare for simplification of inequalities: build quotient ring
  // QQ[...]/<GB>. Simplification is done by mapping polys from QQ[...] into the
  // quotient ring (using hom) and back (using CoCoA::CanonicalRepr).
  auto qring = CoCoA::NewQuotientRing(ring, ideal);
  auto hom = CoCoA::QuotientingHom(qring);

  // Copy origins, reserve last entry for the inequality
  std::vector<Node> origins = d_state->d_allEqualities;
  origins.emplace_back();

  for (const auto& ineq : d_state->d_inequalities)
  {
    // unpack ineq
    TNode assertion = std::get<0>(ineq);
    poly::Polynomial poly = std::get<1>(ineq);
    Kind kind = std::get<2>(ineq);

    // Convert to CoCoA, map into quotient ring and back
    CoCoA::RingElem p = d_state->d_converter(poly, ring);
    CoCoA::RingElem reduced = CoCoA::CanonicalRepr(hom(p));
    poly::Polynomial red = d_state->d_converter(reduced);

    if (red == poly)
    {
      // Nothing changed
      d_simplified.emplace_back(assertion);
      continue;
    }
    // Add as simplification
    Node newneq = nm->mkNode(
        kind, as_cvc_polynomial(red, d_state->d_vm), nm->mkConstReal(0));
    origins.back() = assertion;
    addSimplification(newneq, nm->mkAnd(origins));
  }
}

bool NaiveGroebnerSimplifier::addSimplification(TNode simplified, TNode origins)
{
  Node simp = rewrite(simplified);
  if (simp.isConst())
  {
    if (simp.getConst<bool>())
    {
      Trace("nl-cov::ngs::debug")
          << "Ignoring true from " << simplified << std::endl;
      return true;
    }
    else
    {
      d_conflict = origins.notNode();
      Trace("nl-cov::ngs") << "Found conflict " << *d_conflict << std::endl;
      return false;
    }
  }
  d_simplified.emplace_back(simp);
  if (std::find(d_inputs.begin(), d_inputs.end(), simp) != d_inputs.end())
  {
    return true;
  }
  Trace("nl-cov::ngs") << "Found simplification " << simp << std::endl;
  Trace("nl-cov::ngs") << "From " << origins << std::endl;
  d_lemmaSubstitutions.emplace(simp, origins);
  return true;
}

#else

struct NaiveGroebnerSimplifier::NGSState
{
};

NaiveGroebnerSimplifier::NaiveGroebnerSimplifier(Env& env) : EnvObj(env) {}

NaiveGroebnerSimplifier::~NaiveGroebnerSimplifier() {}

void NaiveGroebnerSimplifier::reset(const std::vector<Node>& inputs)
{
  d_simplified = inputs;
}

void NaiveGroebnerSimplifier::simplify() {}

bool NaiveGroebnerSimplifier::addSimplification(TNode simplified, TNode origins)
{
  return true;
}

#endif

}  // namespace cvc5::theory::arith::nl::coverings
