; EXPECT: sat
(set-option :incremental false)
(set-option :sets-ext true)
(set-logic ALL)
(declare-sort Atom 0)
(declare-fun C32 () (Set (Tuple Atom)))
(declare-fun C2 () (Set (Tuple Atom)))
(declare-fun C4 () (Set (Tuple Atom)))
(declare-fun ATOM_UNIV () (Set (Tuple Atom)))
(declare-fun V1 () Atom)
(assert (= C32 (intersection (complement C2) (complement C4))))
(assert (member (tuple V1) (complement C32)))
(assert (= ATOM_UNIV (as univset (Set (Tuple Atom)))))
(assert (member (tuple V1) ATOM_UNIV))
(assert (member (tuple V1) (complement C2)))
(check-sat)