#include "theory/arith/nl/coverings/equation_simplifier.h"

#include "base/check.h"
#include "base/output.h"
#include "expr/node_algorithm.h"
#include "smt/env.h"
#include "theory/arith/arith_utilities.h"
#include "theory/arith/nl/coverings/cocoa_converter.h"
#include "theory/arith/nl/poly_conversion.h"
#include "theory/theory.h"
#include "util/bitvector.h"

namespace cvc5::theory::arith::nl::coverings {

#if defined(CVC5_USE_POLY) && defined(CVC5_USE_COCOA)

namespace {
struct ShouldTraverse : public SubstitutionMap::ShouldTraverseCallback
{
  bool operator()(TNode n) const override
  {
    switch (theory::kindToTheoryId(n.getKind()))
    {
      case TheoryId::THEORY_BOOL:
      case TheoryId::THEORY_BUILTIN: return true;
      case TheoryId::THEORY_ARITH: return !isTranscendentalKind(n.getKind());
      default: return false;
    }
  }
};
}  // namespace

struct EquationSimplifier::GroebnerState
{
  /** Converter between libpoly and CoCoA */
  CoCoAConverter d_converter;
  /** Maps variables between cvc5 and libpoly */
  VariableMapper d_vm;
  /** Collects all libpoly variables we have seen */
  poly::VariableCollector d_vc;

  /** Stores the polynomials from all equalities. */
  std::vector<poly::Polynomial> d_equalities;

  /**
   * Stores the original assertion, polynomials and kind of all inequalities.
   * If the assertion is negated, the negation is put into the kind.
   */
  std::vector<std::tuple<Node, poly::Polynomial, Kind>> d_inequalities;

  /**
   * Initialize the members of this class from the input assertions.
   * Afterwards, d_vc has all the variables, all equalities are in
   * d_equalities and all inequalities are in
   * d_inequalities.
   */
  void loadAssertions(const std::vector<Node>& inputs)
  {
    for (const auto& input : inputs)
    {
      Trace("nl-cov::ngs::debug")
          << "Load input assertion " << input << std::endl;
      auto polyc = as_poly_constraint(input, d_vm);
      d_vc(polyc.first);
      Kind kind = Kind::EQUAL;
      switch (polyc.second)
      {
        case poly::SignCondition::LT: kind = Kind::LT; break;
        case poly::SignCondition::LE: kind = Kind::LEQ; break;
        case poly::SignCondition::EQ: kind = Kind::EQUAL; break;
        case poly::SignCondition::NE: kind = Kind::DISTINCT; break;
        case poly::SignCondition::GE: kind = Kind::GEQ; break;
        case poly::SignCondition::GT: kind = Kind::GT; break;
        default: Assert(false); break;
      }

      if (kind == Kind::EQUAL)
      {
        d_equalities.emplace_back(polyc.first);
        Trace("nl-cov::ngs::debug")
            << "-> equality " << polyc.first << " = 0" << std::endl;
      }
      else
      {
        d_inequalities.emplace_back(input, polyc.first, kind);
        Trace("nl-cov::ngs::debug") << "-> inequality " << polyc.first << " "
                                    << kind << " 0" << std::endl;
      }
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

EquationSimplifier::EquationSimplifier(Env& env,
                                       const std::vector<Node>& inputs)
    : EnvObj(env), d_inputs(inputs)
{
  d_state = std::make_unique<GroebnerState>();
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

EquationSimplifier::~EquationSimplifier() {}

void EquationSimplifier::setConflict()
{
  std::vector<Node> dummy{NodeManager::currentNM()->mkConst(false)};
  Assert(d_atomOrigins.find(dummy[0]) != d_atomOrigins.end());
  d_conflict =
      NodeManager::currentNM()->mkAnd(resolveAtomOrigins(dummy)).notNode();
  Trace("nl-cov::ngs") << "Found conflict " << *d_conflict << std::endl;
}

void EquationSimplifier::simplify()
{
  Trace("nl-cov::ngs") << "Simplify()" << std::endl << *this << std::endl;
  // Initialize the state
  std::vector<Node> equalities;
  std::vector<Node> inequalities;

  for (const auto& input : d_inputs)
  {
    if (input.getKind() == Kind::EQUAL)
    {
      equalities.emplace_back(input);
    }
    else
    {
      inequalities.emplace_back(input);
    }
  }

  if (equalities.empty())
  {
    Trace("nl-cov::ngs") << "No equalities present, we can't do anything."
                         << std::endl;
    d_simplified = d_inputs;
    return;
  }

  simplifyByTermElimination(equalities, inequalities);
  Trace("nl-cov::ngs") << "... eliminated variables" << std::endl
                       << *this << std::endl;
  if (hasConflict()) return;

  simplifyByGroebnerReduction(equalities, inequalities);
  Trace("nl-cov::ngs") << "... did groebner reduction" << std::endl
                       << *this << std::endl;
  if (hasConflict()) return;

  d_simplified = equalities;
  d_simplified.insert(
      d_simplified.end(), inequalities.begin(), inequalities.end());
  Trace("nl-cov::ngs") << "... done" << std::endl << *this << std::endl;
}

void EquationSimplifier::simplifyByTermElimination(
    std::vector<Node>& equalities, std::vector<Node>& inequalities)
{
  // utility to avoid recursing into non-arithmetic terms
  const ShouldTraverse stc;

  size_t last_size = 0;
  while (equalities.size() != last_size)
  {
    last_size = equalities.size();

    std::vector<Node> next;

    // look for appropriate substitutions in assertion list
    for (const auto& orig : equalities)
    {
      Assert(orig.getKind() == Kind::EQUAL);
      // apply already known substitutions and track which of those were used
      std::set<TNode> tracker;
      d_termSubstitutions.invalidateCache();
      Node o =
          d_termSubstitutions.apply(orig, d_env.getRewriter(), &tracker, &stc);

      if (o != orig)
      {
        addToAtomOrigins(o) << orig << tracker;
      }
      // it may simplify to true or false
      if (o.isConst())
      {
        if (o.getConst<bool>())
        {
          // simplifies to true, just drop it
          continue;
        }
        else
        {
          // simplifies to false, directly use as conflict
          setConflict();
          return;
        }
      }
      Assert(o.getKind() == Kind::EQUAL);
      Assert(o.getNumChildren() == 2);
      for (size_t i = 0; i < 2; ++i)
      {
        const auto& l = o[i];
        const auto& r = o[1 - i];
        if (l.isConst()) continue;
        if (!Theory::isLeafOf(l, TheoryId::THEORY_ARITH)) continue;
        if (d_termSubstitutions.hasSubstitution(l)) continue;
        if (expr::hasSubterm(r, l)) continue;
        d_termSubstitutions.invalidateCache();
        if (expr::hasSubterm(
                d_termSubstitutions.apply(r, nullptr, nullptr, &stc), l))
          continue;
        Trace("nl-eqs") << "Found substitution " << l << " -> " << r
                        << std::endl
                        << " from " << o << " / " << orig << std::endl;
        d_termSubstitutions.addSubstitution(l, r);
        d_termOrigins.emplace(l, o);
        break;
      }
      next.emplace_back(o);
    }
    equalities = std::move(next);
  }

  for (auto& ineq : inequalities)
  {
    // apply known substitutions and track which of those were used
    std::set<TNode> tracker;
    d_termSubstitutions.invalidateCache();
    Node simp =
        d_termSubstitutions.apply(ineq, d_env.getRewriter(), &tracker, &stc);
    if (simp == ineq)
    {
      // nothing changed
      continue;
    }

    addToAtomOrigins(simp) << ineq << tracker;
    // it may simplify to true or false
    if (simp.isConst() && !simp.getConst<bool>())
    {
      // simplifies to false, directly use as conflict
      setConflict();
      return;
    }
    ineq = simp;
  }
}

void EquationSimplifier::simplifyByGroebnerReduction(
    std::vector<Node>& equalities, std::vector<Node>& inequalities)
{
  d_state->loadAssertions(equalities);
  d_state->loadAssertions(inequalities);

  auto [ring, ideal, gbasis] = d_state->computeGBasis();

  std::vector<Node> inputEqs;
  std::swap(inputEqs, equalities);
  inequalities.clear();

  // Now store the simplified equalities. Take all polynomials from the Gröbner
  // basis, construct p = 0 and use it as simplification.
  auto* nm = NodeManager::currentNM();
  for (const auto& poly : gbasis)
  {
    Node p = as_cvc_polynomial(d_state->d_converter(poly), d_state->d_vm);
    Node eq = rewrite(nm->mkNode(Kind::EQUAL, p, nm->mkConstReal(0)));

    if (eq.isConst())
    {
      // 0 should never be in the gbasis, so const can only be a conflict
      Assert(!eq.getConst<bool>());
      addToAtomOrigins(eq) << inputEqs;
      setConflict();
      return;
    }
    if (std::find(inputEqs.begin(), inputEqs.end(), eq) == inputEqs.end())
    {
      // only add origins if the equality was actually simplified
      addToAtomOrigins(eq) << inputEqs;
    }
    equalities.emplace_back(eq);
  }

  // Prepare for simplification of inequalities: build quotient ring
  // QQ[...]/<GB>. Simplification is done by mapping polys from QQ[...] into the
  // quotient ring (using hom) and back (using CoCoA::CanonicalRepr).
  auto qring = CoCoA::NewQuotientRing(ring, ideal);
  auto hom = CoCoA::QuotientingHom(qring);

  for (const auto& ineq : d_state->d_inequalities)
  {
    // unpack ineq
    TNode assertion = std::get<0>(ineq);
    poly::Polynomial poly = std::get<1>(ineq);
    Kind kind = std::get<2>(ineq);
    Trace("nl-cov::ngs::debug") << "Simplifying " << assertion << std::endl;
    Trace("nl-cov::ngs::debug")
        << "Unpacked to " << poly << " " << kind << " 0" << std::endl;

    // Convert to CoCoA, map into quotient ring and back
    CoCoA::RingElem p = d_state->d_converter(poly, ring);
    CoCoA::RingElem reduced = CoCoA::CanonicalRepr(hom(p));
    poly::Polynomial red = d_state->d_converter(reduced);

    // Add as simplification
    Node newneq = rewrite(nm->mkNode(
        kind, as_cvc_polynomial(red, d_state->d_vm), nm->mkConstReal(0)));
    if (newneq.isConst())
    {
      if (newneq.getConst<bool>())
      {
        // simplified to true
        continue;
      }
      else
      {
        addToAtomOrigins(newneq) << assertion << inputEqs;
        setConflict();
        return;
      }
    }
    if (newneq != assertion)
    {
      addToAtomOrigins(newneq) << assertion << inputEqs;
    }
    inequalities.emplace_back(newneq);
  }
}

EquationSimplifier::AtomOriginBuilder::~AtomOriginBuilder()
{
  d_target.d_atomOrigins.emplace(
      d_simplified, std::vector<Node>(d_origins.begin(), d_origins.end()));
}

EquationSimplifier::AtomOriginBuilder&
EquationSimplifier::AtomOriginBuilder::operator<<(TNode n)
{
  // check whether n is a term that was used in a substitution
  auto toit = d_target.d_termOrigins.find(n);
  if (toit != d_target.d_termOrigins.end())
  {
    n = toit->second;
  }
  // check whether n is an atom used for simplification
  auto aoit = d_target.d_atomOrigins.find(n);
  if (aoit != d_target.d_atomOrigins.end())
  {
    d_origins.insert(aoit->second.begin(), aoit->second.end());
  }
  else
  {
    d_origins.insert(n);
  }
  return *this;
}

EquationSimplifier::AtomOriginBuilder&
EquationSimplifier::AtomOriginBuilder::operator<<(const Node& n)
{
  return (*this) << static_cast<TNode>(n);
}

EquationSimplifier::AtomOriginBuilder&
EquationSimplifier::AtomOriginBuilder::operator<<(const std::vector<Node>& n)
{
  for (const auto& a : n) (*this) << a;
  return *this;
}

EquationSimplifier::AtomOriginBuilder&
EquationSimplifier::AtomOriginBuilder::operator<<(const std::set<TNode>& n)
{
  for (const auto& a : n) (*this) << a;
  return *this;
}

EquationSimplifier::AtomOriginBuilder EquationSimplifier::addToAtomOrigins(
    TNode simplified)
{
  return AtomOriginBuilder{*this, simplified, {}};
}

std::vector<Node> EquationSimplifier::resolveAtomOrigins(
    const std::vector<Node>& origins) const
{
  std::vector<Node> res;
  for (const auto& a : origins)
  {
    auto it = d_atomOrigins.find(a);
    if (it != d_atomOrigins.end())
    {
      res.insert(res.end(), it->second.begin(), it->second.end());
    }
    else
    {
      res.emplace_back(a);
    }
  }
  return res;
}

#else

struct EquationSimplifier::GroebnerState
{
};

EquationSimplifier::EquationSimplifier(Env& env) : EnvObj(env) {}

EquationSimplifier::~EquationSimplifier() {}

void EquationSimplifier::reset(const std::vector<Node>& inputs)
{
  d_simplified = inputs;
}

void EquationSimplifier::simplify() {}

bool EquationSimplifier::addSimplification(TNode simplified, TNode origins)
{
  return true;
}

#endif

std::ostream& operator<<(std::ostream& os, const EquationSimplifier& ngs)
{
  os << "Simplifier Input:" << std::endl;
  for (const auto& i : ngs.d_inputs)
  {
    os << "\t" << i << std::endl;
  }
  os << "Term substitutions:" << std::endl;
  for (const auto& s : ngs.d_termSubstitutions)
  {
    auto it = ngs.d_termOrigins.find(s.first);
    Assert(it != ngs.d_termOrigins.end());
    os << "\t" << s.first << " -> " << s.second << " (origin " << it->second
       << ")" << std::endl;
  }
  os << "Atom origins:" << std::endl;
  for (const auto& ao : ngs.d_atomOrigins)
  {
    os << "\t" << ao.first << " -> " << ao.second << std::endl;
  }
  os << "Simplified:" << std::endl;
  for (const auto& s : ngs.d_simplified)
  {
    os << "\t" << s << std::endl;
  }
  if (ngs.d_conflict)
  {
    os << "Conflict: " << *ngs.d_conflict << std::endl;
  }
  else
  {
    os << "No conflict" << std::endl;
  }
  return os;
}

}  // namespace cvc5::theory::arith::nl::coverings
