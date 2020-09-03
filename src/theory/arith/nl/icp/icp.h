#ifndef CVC4__THEORY__ARITH__ICP__ICP_H
#define CVC4__THEORY__ARITH__ICP__ICP_H

#include "expr/node.h"
#include "expr/node_algorithm.h"
#include "theory/arith/arith_msum.h"
#include "theory/arith/normal_form.h"

#include "theory/rewriter.h"

#include <poly/polyxx.h>
#include "util/poly_util.h"
#include "theory/arith/theory_arith.h"
#include "theory/arith/nl/poly_conversion.h"
#include "theory/arith/nl/nl_lemma_utils.h"

#include "theory/arith/nl/icp/candidate.h"
#include "theory/arith/nl/icp/contraction_origins.h"
#include "theory/arith/nl/icp/intersection.h"
#include "theory/arith/nl/icp/variable_bounds.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace icp {

struct IAWrapper {
    const poly::IntervalAssignment& ia;
    const VariableMapper& vm;
};
inline std::ostream& operator<<(std::ostream& os, const IAWrapper& iaw) {
    os << "{ ";
    bool first = true;
    for (const auto& v: iaw.vm.mVarpolyCVC) {
        if (iaw.ia.has(v.first)) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << v.first << " -> " << iaw.ia.get(v.first);
        }
    }
    return os << " }";
}

struct ICPState {
    VariableBounds mBounds;
    std::vector<Candidate> mCandidates;
    ContractionOriginManager mOrigins;
    Node mLastConflict;

    ICPState(VariableMapper& vm): mBounds(vm) {}
};

class ICPSolver {
    InferenceManager& d_im;
    VariableMapper mMapper;
    std::map<Node, std::vector<Candidate>> mCandidateCache;
    std::unique_ptr<ICPState> mState;

    std::int64_t mBudget = 0;
    static constexpr std::int64_t mBudgetIncrement = 10;

    std::vector<Node> collectVariables(const Node& n) const;
    std::vector<Candidate> constructCandidates(const Node& n);
    void addCandidate(const Node& n);

public:
    ICPSolver(InferenceManager& im): d_im(im) {}
    void reset(std::vector<Node> assertions);
    void add(const Node& n);
    void init();

    poly::IntervalAssignment getInitial() const {
        return mState->mBounds.get();
    }

    Node getConflict() const {
        return mState->mLastConflict;
    }

    PropagationResult doIt(poly::IntervalAssignment& ia);
    std::vector<Node> asLemmas(const poly::IntervalAssignment& ia) const;

    /** Returns true if a conflict has been found. */
    bool execute() {
        init();
        auto ia = getInitial();
        bool did_progress = false;
        bool progress = false;
        do
        {
            switch (doIt(ia)) {
                case icp::PropagationResult::NOT_CHANGED:
                    progress = false;
                    break;
                case icp::PropagationResult::CONTRACTED:
                case icp::PropagationResult::CONTRACTED_STRONGLY:
                case icp::PropagationResult::CONTRACTED_WITHOUT_CURRENT:
                case icp::PropagationResult::CONTRACTED_STRONGLY_WITHOUT_CURRENT:
                    did_progress = true;
                    progress = true;
                    break;
                case icp::PropagationResult::CONFLICT:
                    Trace("nl-icp") << "Found a conflict: " << getConflict()
                                    << std::endl;
                    
                    d_im.addPendingArithLemma(getConflict(), Inference::ICP_PROPAGATION);
                    did_progress = true;
                    progress = false;
                    break;
            }
        } while (progress);
        if (did_progress) {
            for (const auto& l : asLemmas(ia))
            {
                d_im.addPendingArithLemma(l, Inference::ICP_PROPAGATION);
            }
            return true;
        }
        return false;
    }

    void print() {
        std::cout << mState->mBounds << std::endl;
        std::cout << "Candidates:" << std::endl;
        for (const auto& c: mState->mCandidates) {
            std::cout << "\t" << c << std::endl;
        }
        std::cout << mState->mOrigins << std::endl;
    }
};

}
}
}
}
}

#endif