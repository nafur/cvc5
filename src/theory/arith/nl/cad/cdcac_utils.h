#ifndef CVC4__THEORY__NLARITH__CAD__CDCAC_UTILS_H
#define CVC4__THEORY__NLARITH__CAD__CDCAC_UTILS_H

#include <vector>

#include "../libpoly/interval.h"
#include "../libpoly/polynomial.h"
#include "../libpoly/value.h"
#include "expr/node.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

/** An interval as specified in section 4.1
 */
struct CACInterval
{
  libpoly::Interval mInterval;
  std::vector<libpoly::Polynomial> mLowerPolys;
  std::vector<libpoly::Polynomial> mUpperPolys;
  std::vector<libpoly::Polynomial> mMainPolys;
  std::vector<libpoly::Polynomial> mDownPolys;
  std::vector<Node> mOrigins;
};
inline bool operator==(const CACInterval& lhs, const CACInterval& rhs)
{
  return lhs.mInterval == rhs.mInterval;
}
inline bool operator<(const CACInterval& lhs, const CACInterval& rhs)
{
  return lhs.mInterval < rhs.mInterval;
}

/** Sort intervals according to section 4.4.1.
 * Also removes fully redundant intervals as in 4.5. 1.
 */
void clean_intervals(std::vector<CACInterval>& intervals);

/** Collect all origins from the list of intervals.
 */
std::vector<Node> collect_constraints(
    const std::vector<CACInterval>& intervals);


/** Sample a point outside of the infeasible intervals.
 * Stores the sample in sample, returns whether such a sample exists.
 * If false is returned, the infeasible intervals cover the real line.
 * Implements sample_outside() from section 4.3
 */
bool sample_outside(const std::vector<CACInterval>& infeasible,
                    libpoly::Value& sample);
}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif