#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <limits>

int main(int argc, char** argv) {
  // Read network.graph
  const char* graph_file = (argc > 1 ? argv[1] : "BasicExample/src/network.graph");
  std::ifstream infile(graph_file);
  if (!infile.is_open()) {
    std::cerr << "Cannot open " << graph_file << std::endl;
    return 1;
  }

  int V, S, L;
  infile >> V >> S >> L;

  // For each ground station: track best satellite (max rate)
  std::vector<double> best_rate(V, 0.0);
  std::vector<int> best_sat(V, -1);

  for (int i = 0; i < L; ++i) {
    int v, s;
    double rate;
    infile >> v >> s >> rate;
    if (rate > best_rate[v]) {
      best_rate[v] = rate;
      best_sat[v]  = s;
    }
  }
  infile.close();

  // Compute per-satellite collection times and global max
  std::vector<double> sat_time(S, 0.0);
  double T = 0.0;
  for (int v = 0; v < V; ++v) {
    if (best_sat[v] < 0) {
      std::cerr << "No valid link for station " << v << std::endl;
      return 1;
    }
    double t = 1000.0 / best_rate[v]; // time for one data unit
    sat_time[best_sat[v]] += t;
  }
  for (int s = 0; s < S; ++s) {
    if (sat_time[s] > T) T = sat_time[s];
  }

  // Write network.greedy.out
  std::ofstream outfile("BasicExample/src/network.greedy.out");
  outfile << T << std::endl;
  // ground_station_id satellite_id
  for (int v = 0; v < V; ++v) {
    outfile << v << " " << best_sat[v] << std::endl;
  }
  // satellite_id data_collection_time
  for (int s = 0; s < S; ++s) {
    outfile << s << " " << sat_time[s] << std::endl;
  }
  outfile.close();

  std::cout << "network.greedy.out generated with T=" << T << std::endl;
  return 0;
}
