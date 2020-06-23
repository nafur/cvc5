#include "theory_call_exporter.h"

#include "expr/node_algorithm.h"
#include "smt/command.h"
#include "smt/smt_engine.h"
#include "smt/smt_engine_scope.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

void export_theory_call(std::size_t n,
                        const std::vector<Node>& assertions,
                        const NRAStatistics& stats)
{
  std::string filename = CVC4::smt::currentSmtEngine()->getFilename() + "-tc-"
                         + std::to_string(n) + ".smt2";
  auto last_slash = filename.find_last_of("/");
  if (last_slash != std::string::npos) {
      filename.erase(0, last_slash + 1);
  }
  std::ofstream out(filename);
  out << language::SetLanguage(CVC4::OutputLanguage::LANG_SMTLIB_V2_6);

  // Add statistics (in json)
  stats.as_json(out, "; ");

  // Set logic
  out << CVC4::SetBenchmarkLogicCommand("QF_NRA") << std::endl;

  // Declare variables
  std::unordered_set<TNode, TNodeHashFunction> vars;
  for (const auto& a : assertions)
  {
    expr::getVariables(a, vars);
  }
  for (const auto& v : vars)
  {
    out << CVC4::DeclareFunctionCommand(
        v.toString(), v.toExpr(), v.getType().toType())
        << std::endl;
  }

  // Add assertions
  for (const auto& a : assertions)
  {
    out << CVC4::AssertCommand(a.toExpr()) << std::endl;
  }

  // Add check-sat
  out << CVC4::CheckSatCommand() << std::endl;
  out.close();
}


}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
