#ifndef CVC4__THEORY__NLARITH__CAD__CDCAC_H
#define CVC4__THEORY__NLARITH__CAD__CDCAC_H

#include "constraints.h"

#include "../libpoly/assignment.h"
#include "../libpoly/interval.h"
#include "../libpoly/polynomial.h"

#include <vector>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

using namespace libpoly;

/** An interval as specified in section 4.1
 */
struct CACInterval {
    Interval mInterval;
    std::vector<Polynomial> mLowerPolys;
    std::vector<Polynomial> mUpperPolys;
    std::vector<Polynomial> mMainPolys;
    std::vector<Polynomial> mDownPolys;
};
inline bool operator==(const CACInterval& lhs, const CACInterval& rhs) {
    return lhs.mInterval == rhs.mInterval;
}
inline bool operator<(const CACInterval& lhs, const CACInterval& rhs) {
    return lhs.mInterval < rhs.mInterval;
}

/** Sort intervals according to section 4.4.1.
 * Also removes fully redundant intervals as in 4.5. 1.
 */
void clean_intervals(std::vector<CACInterval>& intervals);

class CDCAC {
public:
    Assignment mAssignment;
    Constraints mConstraints;

    std::vector<Variable> mVariableOrdering;

    /** Collect all unsatisfiable intervals.
     * Combines unsatisfiable regions from mConstraints evaluated over mAssignment.
     * Implements Algorithm 2.
     */
    std::vector<CACInterval> get_unsat_intervals(std::size_t cur_variable) const;

    /** Sample a point outside of the infeasible intervals.
     * Stores the sample in sample, returns whether such a sample exists.
     * If false is returned, the infeasible intervals cover the real line.
     * Implements sample_outside() from section 4.3
     */
    bool sample_outside(const std::vector<CACInterval>& infeasible, Value& sample) const;

    std::vector<Polynomial> required_coefficients(const Polynomial& p) const;

    std::vector<Polynomial> construct_characterization(const std::vector<CACInterval>& intervals);

    CACInterval interval_from_characterization(const std::vector<Polynomial>& characterization, std::size_t cur_variable, const Value& sample);

    std::vector<CACInterval> get_unsat_cover(std::size_t cur_variable = 0);
};

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif