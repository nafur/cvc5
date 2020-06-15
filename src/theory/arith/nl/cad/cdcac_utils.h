#ifndef CVC4__THEORY__NLARITH__CAD__CDCAC_UTILS_H
#define CVC4__THEORY__NLARITH__CAD__CDCAC_UTILS_H

#include <poly/polyxx.h>

#include <vector>

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
  poly::Interval mInterval;
  std::vector<poly::Polynomial> mLowerPolys;
  std::vector<poly::Polynomial> mUpperPolys;
  std::vector<poly::Polynomial> mMainPolys;
  std::vector<poly::Polynomial> mDownPolys;
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

bool interval_covers(const poly::Interval& lhs, const poly::Interval& rhs);
bool interval_connect(const poly::Interval& lhs, const poly::Interval& rhs);

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
                    poly::Value& sample);

class CDCACDebugger
{
  std::size_t mCheckCounter = 0;
  const std::vector<poly::Variable>& mVariables;

 public:
  CDCACDebugger(const std::vector<poly::Variable>& vars) : mVariables(vars) {}
  void check_interval(const poly::Assignment& a,
                      const poly::Variable& variable,
                      const CACInterval& i);
};

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif