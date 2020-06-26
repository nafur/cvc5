#ifndef CVC4__THEORY__NLARITH__CAD__CDCAC_STATS_H
#define CVC4__THEORY__NLARITH__CAD__CDCAC_STATS_H

#include "smt/smt_statistics_registry.h"

#include "constraints.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

struct NRAFeatures {
    std::size_t num_variables = 0;
    std::size_t num_polynomials = 0;
    std::size_t max_tdegree = 0;
    // Maximum degree of variable ...
    std::size_t max_degree_a = 0;
    std::size_t max_degree_b = 0;
    std::size_t max_degree_c = 0;
    std::size_t max_degree_x = 0;
    std::size_t max_degree_y = 0;
    std::size_t max_degree_z = 0;
    // Percentage of polynomials that contain variable ...
    double perc_poly_contain_a = 0;
    double perc_poly_contain_b = 0;
    double perc_poly_contain_c = 0;
    double perc_poly_contain_x = 0;
    double perc_poly_contain_y = 0;
    double perc_poly_contain_z = 0;
    // Percentage of polynomials that contain variable ...
    double perc_monomial_contain_a = 0;
    double perc_monomial_contain_b = 0;
    double perc_monomial_contain_c = 0;
    double perc_monomial_contain_x = 0;
    double perc_monomial_contain_y = 0;
    double perc_monomial_contain_z = 0;

    NRAFeatures(const Constraints::ConstraintVector& constraints);
    std::vector<double> to_feature_vector() const;
    void to_json_vector(std::ostream& os, const std::string& prefix) const;
    void to_json_dict(std::ostream& os, const std::string& prefix) const;
};

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
    void set(const NRAFeatures& features);
};

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif