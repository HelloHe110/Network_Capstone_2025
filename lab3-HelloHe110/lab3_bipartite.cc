// Copyright 2010-2024 Google LLC
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// [START program]
// Bipartite matching example using the CBC solver.
// [START import]
#include <cstdlib>
#include <memory>

#include "absl/flags/flag.h"
#include "absl/log/flags.h"
#include "ortools/base/init_google.h"
#include "ortools/base/logging.h"
#include "ortools/init/init.h"
#include "ortools/linear_solver/linear_solver.h"
// [END import]

namespace operations_research {
void BipartiteMatching(){
  // [START solver]
  // Create the mixed integer linear solver with the CBC backend.
  std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("CBC_MIXED_INTEGER_PROGRAMMING"));
  if (!solver) {
    LOG(WARNING) << "Could not create solver CBC";
    return;
  }
  // [END solver]

  // Set the number of left and right
  int left_num = 3;
  int right_num = 3;

  // Weight matrix: left[i] to right[j], 0 <= i, j <= 2
  std::vector<std::vector<int>> weights = {
    {5, 8, 6},
    {4, 7, 2},
    {3, 9, 1}
  };

  // [START variables]
  // Create variable x[i][j]: whether the edge between A[i] and B[j] is selected
  std::vector<std::vector<MPVariable*> > x(left_num, std::vector<MPVariable*>(right_num));
  for(int i=0;i<left_num;i++){
    for(int j=0;j<right_num;j++){
      x[i][j] = solver -> MakeIntVar(0, 1, "x_"+std::to_string(i)+std::to_string(j));
    }
  }
  // [END variables]

  // [START constraints]
  // 1. Each node on the left can match at most one node on the right
  for(int i=0;i<left_num;i++){
    LinearExpr sum;
    for(int j=0;j<right_num;j++){
      sum += x[i][j];
    }
    solver -> MakeRowConstraint(sum <= 1);
  }

  // 2. Each node on the right can match at most one node on the left
  for(int j=0;j<right_num;j++){
    LinearExpr sum;
    for(int i=0;i<left_num;i++){
      sum += x[i][j];
    }
    solver -> MakeRowConstraint(sum <= 1);
  }
  // [END constraints]

  // [START objective]
  // Objective: maximize the total matching weight
  MPObjective* const objective = solver->MutableObjective();
  for(int i=0;i<left_num;i++){
    for(int j=0;j<right_num;j++){
      objective -> SetCoefficient(x[i][j], weights[i][j]);
    }
  }
  objective->SetMaximization();
  // [END objective]

  // [START solve]
  LOG(INFO) << "Solving with " << solver->SolverVersion();
  const MPSolver::ResultStatus result_status = solver->Solve();
  // [END solve]

  // [START print_solution]
  // Check that the problem has an optimal solution.
  LOG(INFO) << "Status: " << result_status;
  if(result_status != MPSolver::OPTIMAL){
    LOG(INFO) << "The problem does not have an optimal solution!";
    if(result_status == MPSolver::FEASIBLE){
      LOG(INFO) << "A potentially suboptimal solution was found";
    } 
    else{
      LOG(WARNING) << "The solver could not solve the problem.";
      return;
    }
  }

  LOG(INFO) << "Solution:";
  LOG(INFO) << "Objective value = " << objective->Value();
  for(int i=0;i<left_num;i++){
    for(int j=0;j<right_num;j++){
      if(x[i][j]->solution_value() == 1){
        LOG(INFO) << "Match: A" << i << " - B" << j << " (weight: " << weights[i][j] << ")";
      }
    }
  }
  // [END print_solution]

  // [START advanced]
  LOG(INFO) << "Advanced usage:";
  LOG(INFO) << "Problem solved in " << solver->wall_time() << " milliseconds";
  LOG(INFO) << "Problem solved in " << solver->iterations() << " iterations";
  // [END advanced]
}
}  // namespace operations_research

int main(int argc, char* argv[]) {
  InitGoogle(argv[0], &argc, &argv, true);
  absl::SetFlag(&FLAGS_stderrthreshold, 0);
  operations_research::BipartiteMatching();
  return EXIT_SUCCESS;
}
// [END program]
