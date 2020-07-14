/*********************                                                        */
/*! \file theory_nl_white.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Gereon Kremer
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Unit tests for the nonlinear arithmetic
 **
 ** Unit tests for the nonlinear arithmetic.
 **/

#include <cxxtest/TestSuite.h>

#include <iostream>
#include <memory>
#include <vector>

#include "theory/arith/nl/cad_solver.h"
#include "theory/arith/nl/cad/cdcac.h"
#include "theory/arith/nl/cad/projections.h"
#include "theory/arith/nl/poly_conversion.h"

#include <poly/polyxx.h>
#include "util/poly_util.h"

using namespace CVC4;
using namespace CVC4::theory::arith;
using namespace CVC4::theory::arith::nl;

poly::AlgebraicNumber get_ran(std::initializer_list<long> init, int lower, int upper)
{
  return poly::AlgebraicNumber(poly::UPolynomial(init), poly::DyadicInterval(lower, upper));
}



  NodeManager* d_nodeManager;

  Node operator==(const Node& a, const Node& b) {
    return d_nodeManager->mkNode(Kind::EQUAL, a, b);
  }
  Node operator>=(const Node& a, const Node& b) {
    return d_nodeManager->mkNode(Kind::GEQ, a, b);
  }
  Node operator+(const Node& a, const Node& b) {
    return d_nodeManager->mkNode(Kind::PLUS, a, b);
  }
  Node operator*(const Node& a, const Node& b) {
    return d_nodeManager->mkNode(Kind::NONLINEAR_MULT, a, b);
  }
  Node operator!(const Node& a) {
    return d_nodeManager->mkNode(Kind::NOT, a);
  }
  Node make_real_variable(const std::string& s) {
    return d_nodeManager->mkSkolem(s, d_nodeManager->realType(), "", NodeManager::SKOLEM_EXACT_NAME);
  }
  Node make_int_variable(const std::string& s) {
    return d_nodeManager->mkSkolem(s, d_nodeManager->integerType(), "", NodeManager::SKOLEM_EXACT_NAME);
  }

class TheoryArithNLCADWhite : public CxxTest::TestSuite
{
  NodeManagerScope* d_scope;


 public:
  TheoryArithNLCADWhite() { Trace.on("cad-check");
   }

  void setUp() override
  {
    d_nodeManager = new NodeManager(nullptr);
    d_scope = new NodeManagerScope(d_nodeManager);
  }

  void tearDown() override {
    delete d_scope;
    delete d_nodeManager;
  }

  Node dummy(int i) const { return d_nodeManager->mkConst(Rational(i)); }

  void test_univariate_isolation()
  {
    poly::UPolynomial poly({-2, 2, 3, -3, -1, 1});
    auto roots = poly::isolate_real_roots(poly);

    TS_ASSERT(roots.size() == 4);
    TS_ASSERT(roots[0] == get_ran({-2, 0, 1}, -2, -1));
    TS_ASSERT(roots[1] == poly::Integer(-1))
    TS_ASSERT(roots[2] == poly::Integer(1));
    TS_ASSERT(roots[3] == get_ran({-2, 0, 1}, 1, 2));
  }

  void test_multivariate_isolation()
  {
    poly::Variable x("x");
    poly::Variable y("y");
    poly::Variable z("z");

    poly::Assignment a;
    a.set(x, get_ran({-2, 0, 1}, 1, 2));
    a.set(y, get_ran({-2, 0, 0, 0, 1}, 1, 2));

    poly::Polynomial poly = (y * y + x) - z;

    auto roots = poly::isolate_real_roots(poly, a);

    TS_ASSERT(roots.size() == 1);
    TS_ASSERT(roots[0] == get_ran({-8, 0, 1}, 2, 3));
  }

  void test_univariate_factorization()
  {
    poly::UPolynomial poly({-24, 44, -18, -1, 1, -3, 1});

    auto factors = square_free_factors(poly);
    std::cout << "Factors of " << poly << std::endl;
    for (const auto& f : factors)
    {
      std::cout << "---> " << f << std::endl;
    }
  }

  void test_projection()
  {
    // Gereon's thesis, Ex 5.1
    poly::Variable x("x");
    poly::Variable y("y");

    poly::Polynomial p = (y + 1) * (y + 1) - x * x * x + 3 * x - 2;
    poly::Polynomial q = (x + 1) * y - 3;

    std::cout << "Executing McCallum" << std::endl;
    auto res = cad::projection_mccallum({p, q});
    for (const auto& r : res)
    {
      std::cout << "-> " << r << std::endl;
    }
  }

  void test_cdcac_1()
  {
    cad::CDCAC cac;
    poly::Variable x = cac.get_constraints().var_mapper()(make_real_variable("x"));
    poly::Variable y = cac.get_constraints().var_mapper()(make_real_variable("y"));

    cac.get_constraints().add_constraint(
        4 * y - x * x + 4, poly::SignCondition::LT, dummy(1));
    cac.get_constraints().add_constraint(
        4 * y - 4 + (x - 1) * (x - 1), poly::SignCondition::GT, dummy(2));
    cac.get_constraints().add_constraint(
        4 * y - x - 2, poly::SignCondition::GT, dummy(3));
    
    cac.compute_variable_ordering();

    auto cover = cac.get_unsat_cover();
    TS_ASSERT(cover.empty());
    std::cout << "SAT: " << cac.get_model() << std::endl;
  }

  void test_cdcac_2()
  {
    cad::CDCAC cac;
    poly::Variable x = cac.get_constraints().var_mapper()(make_real_variable("x"));
    poly::Variable y = cac.get_constraints().var_mapper()(make_real_variable("y"));
    
    cac.get_constraints().add_constraint(
        y - pow(-x - 3, 11) + pow(-x - 3, 10) + 1,
        poly::SignCondition::GT,
        dummy(1));
    cac.get_constraints().add_constraint(
        2 * y - x + 2, poly::SignCondition::LT, dummy(2));
    cac.get_constraints().add_constraint(
        2 * y - 1 + x * x, poly::SignCondition::GT, dummy(3));
    cac.get_constraints().add_constraint(
        3 * y + x + 2, poly::SignCondition::LT, dummy(4));
    cac.get_constraints().add_constraint(
        y * y * y - pow(x - 2, 11) + pow(x - 2, 10) + 1,
        poly::SignCondition::GT,
        dummy(5));
    
    cac.compute_variable_ordering();

    auto cover = cac.get_unsat_cover();
    TS_ASSERT(!cover.empty());
    std::cout << "UNSAT:" << std::endl;
    for (const auto& c : cover)
    {
      std::cout << "-> " << c.mInterval << std::endl;
    }
    auto nodes = cad::collect_constraints(cover);
    std::cout << "MIS:" << std::endl;
    for (const auto& c : nodes)
    {
      std::cout << "-> " << c << std::endl;
    }
  }

  void test_cdcac_3()
  {
    cad::CDCAC cac;
    poly::Variable x = cac.get_constraints().var_mapper()(make_real_variable("x"));
    poly::Variable y = cac.get_constraints().var_mapper()(make_real_variable("y"));
    poly::Variable z = cac.get_constraints().var_mapper()(make_real_variable("z"));

    cac.get_constraints().add_constraint(
        x * x + y * y + z * z - 1, poly::SignCondition::LT, dummy(1));
    cac.get_constraints().add_constraint(
        4 * x * x + (2 * y - 3) * (2 * y - 3) + 4 * z * z - 4,
        poly::SignCondition::LT,
        dummy(2));

    cac.compute_variable_ordering();

    auto cover = cac.get_unsat_cover();
    TS_ASSERT(cover.empty());
    std::cout << "SAT: " << cac.get_model() << std::endl;
  }

  void test_cdcac_4()
  {
    cad::CDCAC cac;
    poly::Variable x = cac.get_constraints().var_mapper()(make_real_variable("x"));
    poly::Variable y = cac.get_constraints().var_mapper()(make_real_variable("y"));
    poly::Variable z = cac.get_constraints().var_mapper()(make_real_variable("z"));

    cac.get_constraints().add_constraint(
        -z * z + y * y + x * x - 25, poly::SignCondition::GT, dummy(1));
    cac.get_constraints().add_constraint(
        (y - x - 6) * z * z - 9 * y * y + x * x - 1,
        poly::SignCondition::GT,
        dummy(2));
    cac.get_constraints().add_constraint(
        y * y - 100, poly::SignCondition::LT, dummy(3));
    
    cac.compute_variable_ordering();

    auto cover = cac.get_unsat_cover();
    TS_ASSERT(cover.empty());
    std::cout << "SAT: " << cac.get_model() << std::endl;
  }

  void test_delta(const std::vector<Node>& a)
  {   
    cad::CDCAC cac;
    cac.reset();
    //std::cout << "Constraints:" << std::endl;
    for (const Node& n : a)
    {
      //std::cout << " " << n << std::endl;
      cac.get_constraints().add_constraint(n);
    }
    cac.compute_variable_ordering();

    std::vector<NlLemma> lems;
    // Do full theory check here

    auto covering = cac.get_unsat_cover();
    if (covering.empty()) {
      std::cout << "SAT: " << cac.get_model() << std::endl;
    } else {
      auto mis = cad::collect_constraints(covering);
      std::cout << "Collected MIS: " << mis << std::endl;
      for (auto& n: mis) {
        n = n.negate();
      }
      Assert(!mis.empty()) << "Infeasible subset can not be empty";
      if (mis.size() == 1) {
        lems.emplace_back(mis.front());
      } else {
        lems.emplace_back(d_nodeManager->mkNode(Kind::OR, mis));
      }
      Notice() << "UNSAT with MIS: " << lems.back().d_lemma << std::endl;
    } 

  }

  void test_delta_one() {
    std::vector<Node> a;
    Node zero = d_nodeManager->mkConst(Rational(0));
    Node one = d_nodeManager->mkConst(Rational(1));
    Node mone = d_nodeManager->mkConst(Rational(-1));
    Node fifth = d_nodeManager->mkConst(Rational(1, 2));
    Node g = make_real_variable("g");
    Node l = make_real_variable("l");
    Node q = make_real_variable("q");
    Node s = make_real_variable("s");
    Node u = make_real_variable("u");

    a.emplace_back(l == mone);
    a.emplace_back(!(g*s == zero));
    a.emplace_back(q*s == zero);
    a.emplace_back(u == zero);
    a.emplace_back(q == (one + (fifth*g*s)));
    a.emplace_back(l == u + q*s + (fifth*g*s*s));

    test_delta(a);
  }

  void test_delta_two() {
    std::vector<Node> a;
    Node zero = d_nodeManager->mkConst(Rational(0));
    Node one = d_nodeManager->mkConst(Rational(1));
    Node mone = d_nodeManager->mkConst(Rational(-1));
    Node fifth = d_nodeManager->mkConst(Rational(1, 2));
    Node g = make_real_variable("g");
    Node l = make_real_variable("l");
    Node q = make_real_variable("q");
    Node s = make_real_variable("s");
    Node u = make_real_variable("u");

    a.emplace_back(l == mone);
    a.emplace_back(!(g*s == zero));
    a.emplace_back(u == zero);
    a.emplace_back(q*s == zero);
    a.emplace_back(q == (one + (fifth*g*s)));
    a.emplace_back(l == u + q*s + (fifth*g*s*s));

    test_delta(a);
  }

  void test_ran_conversion() {
    RealAlgebraicNumber ran(std::vector<Rational>({-2, 0, 1}), Rational(1, 3), Rational(7, 3));
    {
      Node x = make_real_variable("x");
      Node n = nl::ran_to_node(ran, x);
      RealAlgebraicNumber back = nl::node_to_ran(n, x);
      TS_ASSERT(ran == back);
    }
  }
};
