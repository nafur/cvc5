#ifndef CVC4__THEORY__NLARITH__CAD__THEORY_CALL_EXPORTER_H
#define CVC4__THEORY__NLARITH__CAD__THEORY_CALL_EXPORTER_H

#include "cdcac_stats.h"
#include "expr/node.h"

#include <vector>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

void export_theory_call(std::size_t n,
                        const std::vector<Node>& assertions,
                        const NRAStatistics& stats);

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif