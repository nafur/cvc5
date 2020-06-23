#ifndef CVC4__THEORY__NLARITH__CAD__CDCAC_STATS_H
#define CVC4__THEORY__NLARITH__CAD__CDCAC_STATS_H

#include "smt/smt_statistics_registry.h"

#include "constraints.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

/** An interval as specified in section 4.1
 */
struct NRAStatistics
{
    IntStat num_variables;
    IntStat num_polynomials;
    IntStat max_tdegree;
    // Maximum degree ov variable ...
    IntStat max_degree_a;
    IntStat max_degree_b;
    IntStat max_degree_c;
    IntStat max_degree_x;
    IntStat max_degree_y;
    IntStat max_degree_z;
    // Percentage of polynomials that contain variable ...
    AverageStat perc_poly_contain_a;
    AverageStat perc_poly_contain_b;
    AverageStat perc_poly_contain_c;
    AverageStat perc_poly_contain_x;
    AverageStat perc_poly_contain_y;
    AverageStat perc_poly_contain_z;
    // Percentage of polynomials that contain variable ...
    AverageStat perc_monomial_contain_a;
    AverageStat perc_monomial_contain_b;
    AverageStat perc_monomial_contain_c;
    AverageStat perc_monomial_contain_x;
    AverageStat perc_monomial_contain_y;
    AverageStat perc_monomial_contain_z;

    NRAStatistics(std::string name);
    ~NRAStatistics();

    void make_stats(const Constraints::ConstraintVector& constraints);
    void as_json(std::ostream& os, const std::string& prefix) const;
};

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif