#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "ortools/linear_solver/linear_solver.h"

using namespace operations_research;

int main(int argc, char** argv) {
  const char* graph_path = (argc > 1 ? argv[1] : "BasicExample/src/network.graph");
  std::ifstream infile(graph_path);
  if (!infile.is_open()) {
    std::cerr << "ERROR: Cannot open graph file: " << graph_path << std::endl;
    return 1;
  }

  int V, S, L;
  infile >> V >> S >> L;
  // Adjacency: for each v, list of (s, time)
  std::vector<std::vector<std::pair<int, double>>> adj(V);
  for (int i = 0; i < L; ++i) {
    int v, s; double rate;
    infile >> v >> s >> rate;
    double t = 1000.0 / rate;
    adj[v].emplace_back(s, t);
  }
  infile.close();

  // Check every station has >=1 link
  for (int v = 0; v < V; ++v) {
    if (adj[v].empty()) {
      std::cerr << "ERROR: Ground station " << v
                << " has no valid links. Infeasible.\n";
      return 1;
    }
  }

  // Create CBC solver
  std::unique_ptr<MPSolver> solver(
      MPSolver::CreateSolver("CBC_MIXED_INTEGER_PROGRAMMING"));
  if (!solver) {
    std::cerr << "ERROR: Solver unavailable." << std::endl;
    return 1;
  }

  // Decision vars x[v][k] for each link k of station v
  std::vector<std::vector<const MPVariable*>> x(V);
  std::vector<std::vector<int>> sat_of(V);
  for (int v = 0; v < V; ++v) {
    for (int k = 0; k < (int)adj[v].size(); ++k) {
      int s = adj[v][k].first;
      x[v].push_back(
        solver->MakeBoolVar("x_" + std::to_string(v) + "_" + std::to_string(s))
      );
      sat_of[v].push_back(s);
    }
  }

  // Max collection time
  const MPVariable* T = solver->MakeNumVar(0.0, MPSolver::infinity(), "T");

  // 1) Each ground station selects exactly one satellite
  for (int v = 0; v < V; ++v) {
    MPConstraint* c = solver->MakeRowConstraint(1.0, 1.0, "assign_" + std::to_string(v));
    for (auto* var : x[v]) c->SetCoefficient(var, 1.0);
  }

  // 2) Satellite loads: T >= sum of times for assigned stations
  // For each satellite s, create constraint: T - sum_{v assigned to s}(t_vk * x_vk) >= 0
  std::unordered_map<int, MPConstraint*> sat_time;
  for (int v = 0; v < V; ++v) {
    for (int k = 0; k < (int)x[v].size(); ++k) {
      int s = sat_of[v][k];
      double t = adj[v][k].second;
      if (!sat_time.count(s)) {
        sat_time[s] = solver->MakeRowConstraint(0.0,
                                                MPSolver::infinity(),
                                                "time_s_" + std::to_string(s));
        sat_time[s]->SetCoefficient(T, 1.0);
      }
      sat_time[s]->SetCoefficient(x[v][k], -t);
    }
  }

  // Objective: minimize T
  MPObjective* obj = solver->MutableObjective();
  obj->SetCoefficient(T, 1.0);
  obj->SetMinimization();

  // Solve
  const MPSolver::ResultStatus status = solver->Solve();
  if (status != MPSolver::OPTIMAL) {
    std::cerr << "ERROR: Infeasible or no optimal solution (status=" << status << ")." << std::endl;
    return 1;
  }

  // Write output
  std::ofstream outfile("BasicExample/src/network.ortools.out");
  outfile << T->solution_value() << "\n";
  // assignments
  for (int v = 0; v < V; ++v) {
    for (int k = 0; k < (int)x[v].size(); ++k) {
      if (x[v][k]->solution_value() > 0.5) {
        outfile << v << " " << sat_of[v][k] << "\n";
      }
    }
  }
  // per-satellite times
  std::vector<std::pair<int, double>> sorted_times;
  for (const auto& kv : sat_time) {
    int s = kv.first;
    double total = 0;
    // sum again or track separately
    for (int v = 0; v < V; ++v) {
      for (int k = 0; k < (int)x[v].size(); ++k) {
        if (sat_of[v][k] == s && x[v][k]->solution_value() > 0.5) {
          total += adj[v][k].second;
        }
      }
    }
    sorted_times.push_back({s, total});
  }
  
  // Sort by satellite number
  std::sort(sorted_times.begin(), sorted_times.end());
  
  // Output in sorted order
  for (const auto& [s, total] : sorted_times) {
    outfile << s << " " << total << "\n";
  }
  outfile.close();

  std::cout << "Success: network.ortools.out generated with T="
            << T->solution_value() << std::endl;
  return 0;
}
