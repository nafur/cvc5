(set-logic QF_RDL)
(set-info :status unsat)
(set-info :notes | Simple test, based on simple-rdl.smt2, of define-sort and define-fun |)
(declare-fun x () Real)
(declare-fun y () Real)
(declare-sort U 0)
(define-sort A (x y) y)
(define-sort F (x) (A x x))
(declare-fun x2 () (F Real))
(define-fun minus ((x Real) (z Real)) (A (A U Bool) (A (F U) Real)) (- x z))
(define-fun less ((x Real) (z Real)) Bool (< x z))
(define-fun foo ((x (F Real)) (z (A U Real))) (F (F Bool)) (less x z))
(assert (not (=> (foo (minus x y) 0) (less x y))))
(check-sat)
(exit)