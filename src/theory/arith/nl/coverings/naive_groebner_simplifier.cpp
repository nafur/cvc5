#include "theory/arith/nl/coverings/naive_groebner_simplifier.h"

#ifdef CVC5_POLY_IMP

#include "base/check.h"
#include "base/output.h"
#include "theory/arith/nl/coverings/cocoa_converter.h"
#include "theory/arith/nl/poly_conversion.h"
#include "util/bitvector.h"

namespace cvc5::theory::arith::nl::coverings {

struct NaiveGroebnerSimplifier::NGSState
{
  VariableMapper d_vm;

  CoCoAConverter d_converter;

  poly::Polynomial getPolynomial(TNode assertion)
  {
    return as_poly_polynomial(assertion[0], d_vm)
           - as_poly_polynomial(assertion[1], d_vm);
  }

  auto computeGBasis(const std::vector<Node>& equalities)
  {
    std::vector<poly::Polynomial> polys;
    poly::VariableCollector vc;
    for (const auto& eq : equalities)
    {
      poly::Polynomial p = getPolynomial(eq);
      polys.emplace_back(p);
      vc(p);
      Trace("nl-cov::ngs") << "Extracted polynomial " << p << std::endl;
    }

    CoCoA::ring ring = d_converter.mkPolyRing(vc.get_variables());
    Trace("nl-cov::ngs") << "Built ring " << ring << std::endl;

    std::vector<CoCoA::RingElem> cpolys;
    for (const auto& p : polys)
    {
      cpolys.emplace_back(d_converter(p, ring));
      Trace("nl-cov::ngs") << p << " -> " << cpolys.back() << std::endl;
    }
    auto gbasis = CoCoA::ReducedGBasis(CoCoA::ideal(cpolys));
    return std::make_pair(ring, gbasis);
  }
};

NaiveGroebnerSimplifier::NaiveGroebnerSimplifier(
    Env& env, const std::vector<Node>& inputs)
    : EnvObj(env), d_inputs(inputs), d_state(std::make_unique<NGSState>())
{
  simplify();
}

NaiveGroebnerSimplifier::~NaiveGroebnerSimplifier() {}

std::pair<std::vector<Node>, std::vector<Node>>
NaiveGroebnerSimplifier::splitAssertions(const std::vector<Node>& inputs)
{
  std::vector<Node> equalities;
  std::vector<Node> nonequalities;
  for (const auto& input : inputs)
  {
    if (input.getKind() == Kind::EQUAL)
    {
      equalities.emplace_back(input);
    }
    else
    {
      nonequalities.emplace_back(input);
    }
  }
  return std::make_pair(equalities, nonequalities);
}

void NaiveGroebnerSimplifier::simplify()
{
  auto* nm = NodeManager::currentNM();
  auto [equalities, nonequalities] = splitAssertions(d_inputs);

  auto [ring, gbasis] = d_state->computeGBasis(equalities);

  Trace("nl-cov::ngs") << "GBasis = " << gbasis << std::endl;

  Node allEqs = nm->mkAnd(equalities);
  for (const auto& cpoly : gbasis)
  {
    poly::Polynomial p = d_state->d_converter(cpoly);
    Node n = as_cvc_polynomial(p, d_state->d_vm);
    Node eq = rewrite(nm->mkNode(Kind::EQUAL, n, nm->mkConstReal(0)));
    if (eq.isConst())
    {
      if (eq.getConst<bool>())
      {
        Trace("nl-cov::ngs") << "Obtained 0=0, ignoring" << std::endl;
      }
      else
      {
        d_conflict = allEqs;
        return;
      }
    }
    if (std::find(d_inputs.begin(), d_inputs.end(), eq) == d_inputs.end())
    {
      d_lemmaSubstitutions.emplace(eq, allEqs);
      Trace("nl-cov::ngs") << "subst " << eq << " -> " << allEqs << std::endl;
    }
    d_simplified.emplace_back(eq);
  }

  Trace("nl-cov::ngs") << "Reducing inequalities now" << std::endl;
  for (auto neq : nonequalities)
  {
      Trace("nl-cov::ngs") << neq << "..." << std::endl;
      bool negated = neq.getKind() == Kind::NOT;
      if (negated) neq = neq[0];

      poly::Polynomial poly = d_state->getPolynomial(neq);
      CoCoA::RingElem p = d_state->d_converter(poly, ring);
      Trace("nl-cov::ngs") << (negated ? "not " : "") << neq << " -> " << p << std::endl;


  }
}

}  // namespace cvc5::theory::arith::nl::coverings

#endif
