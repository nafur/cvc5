(set-logic QF_AUFBV)
(set-info :status sat)
(declare-const arr-414726656414253458_-8019474897564551692-0 (Array (_ BitVec 8) Bool))
(declare-const bv_8-1 (_ BitVec 8))
(assert (= (store arr-414726656414253458_-8019474897564551692-0 (bvadd bv_8-1 bv_8-1) true) arr-414726656414253458_-8019474897564551692-0))
(check-sat)