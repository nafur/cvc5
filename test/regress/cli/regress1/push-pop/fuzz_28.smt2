; COMMAND-LINE: --incremental
; EXPECT: sat
; EXPECT: sat
; EXPECT: sat
; EXPECT: unsat
; EXPECT: sat
; EXPECT: unsat
; EXPECT: unsat
; EXPECT: sat
(set-logic QF_LIA)
(declare-fun x0 () Bool)
(declare-fun x1 () Bool)
(declare-fun x2 () Bool)
(declare-fun x3 () Bool)
(declare-fun x4 () Bool)
(declare-fun x5 () Bool)
(declare-fun x6 () Bool)
(declare-fun x7 () Bool)
(declare-fun x8 () Bool)
(declare-fun x9 () Bool)
(assert (not (and (or (and (or (or (not (not (or (not x3) (or x9 x2)))) (and (not (or (and x8 x5) (and x0 x4))) (not (and (not x6) (not x1))))) (or (not (or (and (or x2 x9) (and x1 x9)) (not (not x2)))) (not (not (and (or x8 x0) (not x8)))))) (or (and (and (and (or (not x2) (not x5)) (not (or x1 x6))) (not (not (or x2 x4)))) (or (not (and (not x7) (or x1 x6))) (and (or (or x6 x7) (or x8 x7)) (not (and x0 x8))))) (and (not (and (and (and x2 x1) (and x0 x5)) (not (or x8 x8)))) (not (and (and (not x2) (or x5 x8)) (not (not x1))))))) (and (and (not (or (and (or (and x3 x3) (or x2 x5)) (and (or x8 x9) (not x2))) (and (or (not x5) (not x1)) (and (or x6 x7) (not x1))))) (not (and (and (and (or x1 x5) (and x4 x8)) (not (and x6 x4))) (and (or (or x0 x8) (and x3 x5)) (not (and x6 x1)))))) (not (and (and (not (or (or x0 x4) (or x9 x7))) (or (and (and x8 x4) (not x4)) (or (and x0 x5) (or x1 x9)))) (not (and (and (and x3 x5) (and x8 x6)) (and (or x9 x8) (or x2 x7)))))))) (or (or (or (or (or (not (or (or x8 x3) (or x3 x7))) (not (and (not x7) (or x1 x0)))) (and (or (and (or x5 x7) (or x3 x0)) (and (and x0 x6) (not x2))) (not (not (or x4 x3))))) (or (and (not (or (and x2 x4) (or x0 x6))) (not (and (and x6 x7) (not x3)))) (not (and (and (and x7 x5) (and x3 x0)) (and (not x8) (and x6 x1)))))) (and (not (not (not (or (not x2) (not x3))))) (or (or (not (and (not x8) (not x5))) (and (or (or x0 x1) (and x6 x1)) (not (not x1)))) (not (not (or (or x1 x5) (not x0))))))) (and (or (not (and (or (or (not x6) (not x7)) (and (not x3) (and x7 x3))) (and (not (not x0)) (and (not x1) (or x4 x6))))) (or (and (not (and (and x8 x7) (and x9 x7))) (not (or (not x2) (and x0 x6)))) (or (not (not (or x3 x3))) (not (not (or x9 x7)))))) (and (not (not (not (or (and x2 x2) (or x9 x7))))) (or (and (and (or (or x5 x7) (and x2 x0)) (and (or x8 x4) (not x7))) (not (not (and x6 x9)))) (and (not (not (not x0))) (or (and (not x9) (or x1 x7)) (not (or x9 x7)))))))))))
(assert (not (and (not (not (or x9 x8))) (or (not (not x9)) (or (and x4 x6) (or x3 x8))))))
(assert (or (or (not (or (and (and (and x9 x5) (or x3 x6)) (not (not x9))) (and (and (and x7 x6) (and x8 x3)) (or (and x1 x8) (and x0 x9))))) (not (not (or (and (and x8 x7) (or x9 x2)) (and (not x1) (not x1)))))) (and (or (and (and (not (or x7 x2)) (not (or x6 x5))) (and (or (and x8 x6) (and x8 x7)) (or (not x6) (not x7)))) (or (or (not (not x3)) (and (and x0 x5) (not x4))) (and (not (or x7 x2)) (not (and x1 x9))))) (or (not (and (or (and x5 x4) (or x1 x0)) (or (and x6 x2) (not x1)))) (and (or (not (not x1)) (and (or x2 x7) (or x6 x1))) (not (or (and x2 x0) (not x4))))))))
(check-sat)
(push 1)
(check-sat)
(push 1)
(assert (or (or (or (not (not (not (or (and (or (not x2) (and x1 x3)) (not (not x3))) (or (not (not x8)) (or (or x9 x7) (and x3 x3))))))) (not (or (and (not (or (or (and x6 x6) (not x6)) (and (not x1) (not x4)))) (and (not (not (and x9 x7))) (and (and (and x6 x9) (and x1 x2)) (not (or x3 x1))))) (or (or (or (not (not x9)) (not (and x4 x4))) (or (not (and x4 x0)) (or (not x0) (or x2 x0)))) (and (and (and (or x1 x1) (not x9)) (or (or x7 x7) (not x2))) (and (or (and x1 x7) (or x2 x9)) (and (not x2) (not x2)))))))) (and (or (not (and (not (and (not (or x5 x6)) (not (or x3 x8)))) (or (not (not (or x6 x3))) (and (or (or x6 x1) (not x9)) (not (or x8 x2)))))) (not (and (not (or (or (or x7 x1) (not x3)) (or (or x6 x9) (and x8 x1)))) (not (not (not (and x6 x7))))))) (or (not (not (and (and (or (not x3) (not x0)) (or (or x3 x3) (or x4 x3))) (or (and (and x8 x6) (or x7 x7)) (not (or x4 x4)))))) (and (and (and (not (or (or x2 x7) (not x6))) (and (or (not x2) (or x3 x6)) (and (not x8) (not x9)))) (and (not (and (or x1 x5) (and x6 x1))) (or (not (or x6 x1)) (or (or x0 x8) (not x5))))) (and (or (not (not (and x5 x3))) (or (not (or x8 x1)) (not (or x7 x4)))) (and (and (and (or x3 x0) (or x1 x4)) (and (or x6 x7) (not x1))) (or (or (not x0) (and x6 x5)) (not (not x4))))))))) (and (or (and (and (and (not (not (not (and x3 x6)))) (and (or (not (not x6)) (not (not x3))) (or (not (not x6)) (and (or x4 x9) (not x0))))) (and (or (not (not (or x5 x6))) (or (and (and x8 x2) (and x5 x2)) (or (or x0 x4) (or x4 x6)))) (or (and (or (not x0) (or x6 x4)) (not (and x5 x6))) (and (or (and x8 x7) (not x7)) (or (and x2 x5) (and x7 x5)))))) (or (and (not (not (or (not x7) (or x7 x8)))) (not (not (or (and x0 x9) (or x4 x6))))) (and (and (or (not (and x1 x4)) (not (and x5 x4))) (or (or (or x9 x0) (and x9 x9)) (not (or x7 x7)))) (or (not (or (not x3) (or x8 x0))) (or (not (or x3 x4)) (not (not x1))))))) (or (not (and (not (or (not (or x0 x6)) (or (not x9) (not x9)))) (or (not (not (and x6 x9))) (not (and (not x9) (and x8 x2)))))) (not (or (or (and (and (not x6) (or x7 x2)) (and (and x8 x5) (and x0 x8))) (and (not (not x7)) (or (not x6) (or x8 x9)))) (or (and (and (and x5 x6) (and x4 x2)) (and (or x1 x2) (not x4))) (not (and (and x4 x8) (and x7 x7)))))))) (or (and (and (or (or (or (and (and x5 x5) (or x8 x2)) (not (not x8))) (and (not (not x3)) (or (or x9 x8) (not x2)))) (and (or (and (and x4 x8) (and x7 x7)) (and (not x3) (and x8 x0))) (and (not (and x2 x5)) (and (and x9 x4) (and x5 x3))))) (not (and (or (not (not x7)) (and (not x1) (or x5 x1))) (not (not (and x6 x5)))))) (or (not (not (or (and (not x1) (or x1 x0)) (and (and x6 x8) (and x1 x0))))) (or (or (and (not (or x9 x2)) (or (and x5 x6) (and x1 x2))) (not (not (and x7 x2)))) (or (not (not (and x0 x3))) (or (not (and x8 x0)) (and (not x5) (not x7))))))) (or (or (or (and (and (or (and x9 x0) (or x1 x9)) (not (and x4 x4))) (and (and (not x8) (not x1)) (and (or x0 x9) (not x2)))) (not (not (not (not x2))))) (not (and (not (not (or x5 x1))) (and (or (not x9) (not x6)) (not (or x1 x6)))))) (not (or (or (and (and (or x5 x1) (not x1)) (and (not x4) (not x2))) (or (and (and x6 x2) (or x4 x3)) (and (and x1 x7) (and x8 x0)))) (or (or (not (or x9 x4)) (or (and x7 x8) (or x1 x6))) (or (or (or x0 x7) (and x6 x2)) (or (not x5) (not x0)))))))))))
(assert (not (and (and (and (and (or (and (or x7 x6) (and x4 x6)) (not (or x4 x2))) (and (and (or x7 x6) (or x9 x1)) (or (not x1) (and x1 x8)))) (not (not (and (or x7 x1) (not x7))))) (not (or (not (and (or x7 x2) (and x2 x2))) (or (and (or x5 x1) (and x8 x1)) (or (and x2 x8) (not x6)))))) (or (or (not (not (or (not x2) (or x2 x8)))) (not (and (not (not x9)) (not (and x9 x6))))) (and (or (or (not (and x1 x3)) (not (not x3))) (and (and (and x1 x4) (and x1 x9)) (not (or x1 x7)))) (not (not (or (not x2) (not x0)))))))))
(check-sat)
(push 1)
(assert (and (not (and (and (or (and (and (or (not (or x0 x2)) (not (and x3 x8))) (or (and (not x3) (or x1 x7)) (and (or x0 x3) (or x0 x0)))) (and (not (and (not x6) (not x9))) (or (not (and x9 x4)) (and (or x1 x3) (not x7))))) (or (not (and (and (not x9) (not x4)) (not (not x1)))) (not (or (or (or x7 x9) (and x0 x1)) (not (and x6 x3)))))) (or (not (not (or (and (not x5) (not x0)) (and (and x1 x9) (and x7 x1))))) (or (and (or (and (not x3) (or x9 x8)) (not (and x0 x6))) (not (not (not x5)))) (and (and (and (and x7 x6) (and x7 x2)) (and (not x9) (and x8 x1))) (and (not (not x4)) (not (or x5 x7))))))) (not (not (or (and (and (or (and x5 x6) (not x1)) (or (not x7) (not x6))) (and (or (and x3 x6) (or x4 x3)) (or (and x7 x3) (not x5)))) (or (not (not (or x7 x4))) (and (or (not x6) (or x4 x6)) (or (or x1 x3) (not x2))))))))) (or (and (not (and (or (and (not (not (and x3 x8))) (and (not (and x4 x9)) (not (not x0)))) (or (and (or (or x7 x5) (not x1)) (or (and x8 x0) (not x1))) (and (and (and x8 x9) (or x3 x2)) (not (not x9))))) (or (or (or (and (or x2 x6) (not x1)) (or (not x6) (not x4))) (or (or (not x4) (or x6 x7)) (and (and x8 x0) (and x3 x5)))) (or (and (not (or x1 x6)) (not (not x0))) (not (not (not x0))))))) (or (not (not (and (or (or (and x6 x5) (and x5 x6)) (not (or x9 x2))) (not (not (and x8 x0)))))) (or (not (and (not (not (and x1 x4))) (and (and (not x9) (and x4 x1)) (or (not x9) (not x7))))) (and (or (and (not (or x4 x1)) (not (and x9 x1))) (and (not (and x0 x4)) (and (or x8 x0) (not x5)))) (or (not (and (or x7 x1) (not x9))) (not (not (not x0)))))))) (not (or (or (not (or (and (not (not x3)) (and (or x5 x0) (not x1))) (not (not (not x8))))) (and (and (or (and (and x3 x3) (or x2 x3)) (not (or x5 x3))) (and (and (and x3 x2) (or x6 x8)) (and (or x8 x1) (not x9)))) (or (and (and (and x7 x0) (and x7 x4)) (and (and x8 x3) (not x4))) (not (not (not x8)))))) (and (not (and (not (and (or x6 x4) (or x9 x9))) (or (and (or x8 x3) (or x0 x0)) (not (or x0 x7))))) (or (and (or (and (and x5 x9) (or x7 x2)) (and (not x2) (not x4))) (and (or (not x8) (or x4 x4)) (not (or x9 x4)))) (or (not (not (and x0 x5))) (or (and (not x9) (not x5)) (not (or x7 x7)))))))))))
(assert (and (not (or x6 x8)) (or (or x4 x6) (or x4 x6))))
(assert (or (or (not (not (and (or x3 x4) (or x4 x9)))) (or (not (and (not x2) (and x9 x7))) (and (and (and x4 x9) (not x2)) (not (and x8 x5))))) (or (or (and (not (not x3)) (or (and x9 x9) (and x1 x8))) (or (and (and x7 x3) (and x4 x8)) (not (not x4)))) (and (not (not (or x4 x5))) (and (or (or x7 x0) (and x2 x6)) (not (or x7 x7)))))))
(check-sat)
(pop 1)
(check-sat)
(push 1)
(assert (not (and (or (not (and (or (or (or (not (not x9)) (not (and x2 x7))) (not (or (or x3 x5) (not x6)))) (not (and (and (and x9 x5) (and x2 x4)) (and (or x0 x9) (not x7))))) (and (or (not (and (not x8) (or x6 x7))) (not (and (or x7 x6) (or x3 x5)))) (and (and (and (and x4 x9) (or x0 x8)) (not (not x9))) (not (not (and x6 x1))))))) (or (not (and (and (and (not (or x0 x7)) (not (or x1 x7))) (and (or (or x4 x1) (or x3 x3)) (or (not x7) (or x3 x0)))) (or (and (and (not x6) (and x5 x0)) (not (or x8 x2))) (and (and (or x7 x0) (and x9 x8)) (and (and x0 x2) (not x2)))))) (and (not (not (not (and (not x2) (or x6 x1))))) (and (and (and (and (and x9 x7) (or x2 x0)) (and (or x5 x5) (and x6 x8))) (or (and (and x6 x9) (or x4 x1)) (not (and x5 x5)))) (and (not (and (not x0) (or x9 x1))) (not (not (or x0 x3)))))))) (or (not (and (not (and (or (not (or x2 x6)) (and (or x2 x6) (not x8))) (or (or (and x0 x9) (and x0 x0)) (or (or x3 x0) (and x3 x3))))) (not (not (and (or (or x2 x1) (and x5 x9)) (or (or x1 x1) (and x2 x5))))))) (and (not (or (and (not (or (or x4 x0) (and x6 x9))) (or (not (and x5 x5)) (and (and x0 x7) (or x7 x7)))) (not (not (not (not x9)))))) (or (and (or (and (and (and x2 x6) (and x6 x9)) (or (not x8) (and x3 x0))) (and (not (not x6)) (and (not x1) (or x3 x1)))) (not (or (and (and x8 x7) (and x0 x4)) (or (not x1) (not x0))))) (or (not (or (or (and x7 x1) (and x8 x7)) (and (or x3 x1) (or x6 x2)))) (and (not (not (not x0))) (not (and (or x2 x3) (or x5 x3)))))))))))
(check-sat)
(pop 1)
(assert (and x5 x7))
(check-sat)
(pop 1)
(check-sat)