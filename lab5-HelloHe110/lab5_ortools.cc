// MaxSumThroughput.ortools.cc
#include <bits/stdc++.h>
#include "ortools/linear_solver/linear_solver.h"
using namespace std;

namespace operations_research {

// 記錄真實邊（無向圖）
struct Edge {
    int from, to;
    double capacity;
};

// 記錄每個流的來源與目的
struct SDPair {
    int src, dst;
};

// 學號（作為輸出之一部份）
int student_id = 111550022;

// 單一測資案例的求解函式
double SolveSingleTestCase(int U, int E, int F, istream& in, ostream& out) {
    // 1. 讀取真實邊資料，建立容量矩陣（無向圖：雙向皆記錄）
    vector<vector<double>> capacity(U, vector<double>(U, 0));
    vector<Edge> real_edges;
    for (int i = 0; i < E; ++i) {
        int u, v;
        double cap;
        in >> u >> v >> cap;
        capacity[u][v] = cap;
        capacity[v][u] = cap;
        real_edges.push_back({u, v, cap});
        real_edges.push_back({v, u, cap});
    }
  
    // 2. 讀取 F 組來源與目的對
    vector<SDPair> flows(F);
    vector<int> sources, destinations;
    for (int i = 0; i < F; ++i) {
        int s, d;
        in >> s >> d;
        flows[i] = {s, d};
        sources.push_back(s);
        destinations.push_back(d);
    }
  
    // 3. 計算每個真實節點的流出與流入上界（總出／入容量）
    vector<double> out_bound(U, 0.0), in_bound(U, 0.0);
    for (int u = 0; u < U; ++u) {
        for (int v = 0; v < U; ++v) {
            out_bound[u] += capacity[u][v];
            in_bound[u]  += capacity[v][u];
        }
    }
  
    // 4. 虛擬節點設定：總數 V = U + 2*F
    // 為每流建立一個虛擬來源節點及一個虛擬目的節點
    int V = U + 2 * F;
    vector<int> virSrcs(F), virDsts(F);
    for (int i = 0; i < F; ++i) {
        virSrcs[i] = U + i;
        virDsts[i] = U + F + i;
    }
  
    // 5. 構造允許的邊集合：
    //    ① 真實邊：所有 (u,v) (u,v ∈ [0, U))
    //    ② 虛擬邊：對每個流 i，增加 (virSrcs[i], sources[i]) 與 (destinations[i], virDsts[i])
    set<pair<int, int>> allowed_edges;
    for (auto& e : real_edges)
        allowed_edges.insert({e.from, e.to});
    for (int i = 0; i < F; ++i) {
        allowed_edges.insert({virSrcs[i], sources[i]});
        allowed_edges.insert({destinations[i], virDsts[i]});
    }
    vector<pair<int, int>> all_edges(allowed_edges.begin(), allowed_edges.end());
    sort(all_edges.begin(), all_edges.end());
  
    // 6. 建立混合整數求解器（使用 CBC 作為 MILP 求解器）
    MPSolver solver("ILP", MPSolver::CBC_MIXED_INTEGER_PROGRAMMING);
    cerr << "[DEBUG] Solver(ILP-CBC_MIXED_INTEGER_PROGRAMMING) created\n";

    // 7. 建立變數
    // X[f,u,v]：連續變數，代表流 f 在邊 (u,v) 上的流量，且其上界：
    //      真實邊：capacity[u][v]
    //      虛擬來源邊：out_bound[ sources[f] ]
    //      虛擬目的邊：in_bound[ destinations[f] ]
    // Y[f,u,v]：二元變數，表示流 f 是否選擇使用邊 (u,v)
    // Z[u,v]：對真實邊 (u,v)（u,v ∈ [0,U)），只在 f==0 時建立，用以後續限制節點單一發射／接收
    map<tuple<int, int, int>, const MPVariable*> X;
    map<tuple<int, int, int>, const MPVariable*> Y;
    map<pair<int, int>, const MPVariable*> Z;
    for (int f = 0; f < F; ++f) {
        for (auto& edge : all_edges) {
            int u = edge.first, v = edge.second;
            double ub = 0;
            if (u < U && v < U) { ub = capacity[u][v]; } // 真實邊
            else if (u >= U) {   // 虛擬來源邊，要求 u == virSrcs[f]且 v == sources[f]
                if (u == virSrcs[f] && v == sources[f]) ub = out_bound[sources[f]];
                else continue; // 非本流的虛擬邊略過
            } 
            else if (v >= U) {   // 虛擬目的邊，要求 v == virDsts[f]且 u == destinations[f]
                if (v == virDsts[f] && u == destinations[f]) ub = in_bound[destinations[f]];
                else continue;
            }

            string varName = "X_f" + to_string(f) + "_" + to_string(u) + "_" + to_string(v);
            X[{f, u, v}] = solver.MakeNumVar(0.0, ub, varName);
            string varY = "Y_f" + to_string(f) + "_" + to_string(u) + "_" + to_string(v);
            Y[{f, u, v}] = solver.MakeBoolVar(varY);
            
            // 只對真實邊建立全局 Z 變數（只在 f==0 時建立一次即可）
            if (f == 0 && u < U && v < U) {
                string varZ = "Z_" + to_string(u) + "_" + to_string(v);
                Z[{u, v}] = solver.MakeBoolVar(varZ);
            }
        }
    }
  
    // 8. 目標函數：最大化各流從虛擬來源至真實來源的流量總和
    MPObjective* const objective = solver.MutableObjective();
    for (int f = 0; f < F; ++f) {
        auto it = X.find({f, virSrcs[f], sources[f]});
        if (it != X.end())  objective->SetCoefficient(it->second, 1.0);
    }
    objective->SetMaximization();

    // 9. 加入約束

    // (1) 單一路徑約束：對每個流 f 及每個節點 u (u ∈ [0,V))，
    //  「從 u 發出」以及「進入 u」的 Y 變數總和均 ≤ 1
    for (int f = 0; f < F; ++f) {
        for (int u = 0; u < V; ++u) {
            MPConstraint* out_cons = solver.MakeRowConstraint(0.0, 1.0);
            MPConstraint* in_cons  = solver.MakeRowConstraint(0.0, 1.0);
            for (int v = 0; v < V; ++v) {
                auto key = make_tuple(f, u, v);
                if (X.find(key) != X.end()) out_cons->SetCoefficient(Y[key], 1);

                auto key_in = make_tuple(f, v, u);
                if (X.find(key_in) != X.end()) in_cons->SetCoefficient(Y[key_in], 1);
            }
        }
    }
  
    // (2) 流量守恆約束
    // (a) 在流的進入與離開網路處：虛擬來源到真實來源的流量與真實目的到虛擬目的流量必相同
    for (int f = 0; f < F; ++f) {
        MPConstraint* flow_se = solver.MakeRowConstraint(0.0, 0.0);
        auto key_in = make_tuple(f, virSrcs[f], sources[f]);
        if (X.find(key_in) != X.end()) flow_se->SetCoefficient(X[key_in], 1.0);
        auto key_out = make_tuple(f, destinations[f], virDsts[f]);
        if (X.find(key_out) != X.end()) flow_se->SetCoefficient(X[key_out], -1.0);
    }
  
    // (b) 真實節點（u ∈ [0, U)）上流量平衡：進流 - 出流 = 0
    for (int f = 0; f < F; ++f) {
        for (int u = 0; u < U; ++u) {
            MPConstraint* flow_cons = solver.MakeRowConstraint(0.0, 0.0);
            for (int v = 0; v < V; ++v) {
                auto key = make_tuple(f, u, v);
                if (X.find(key) != X.end()) flow_cons->SetCoefficient(X[key], 1.0);
                auto key2 = make_tuple(f, v, u);
                if (X.find(key2) != X.end()) flow_cons->SetCoefficient(X[key2], -1.0);
            }
        }
    }
  
    // (3) 邊容量約束：對所有真實邊 (u,v) (u,v ∈ [0,U))，
    //  所有流在此邊上的流量總和 ≤ capacity[u][v]
    for (auto& edge : all_edges) {
        int u = edge.first, v = edge.second;
        if (u < U && v < U) {
            MPConstraint* cap_cons = solver.MakeRowConstraint(0.0, capacity[u][v]);
            for (int f = 0; f < F; ++f) {
                auto key = make_tuple(f, u, v);
                if (X.find(key) != X.end()) cap_cons->SetCoefficient(X[key], 1.0);
            }
        }
    }
  
    // (4) X 與 Y 綁定：對每組 (f,u,v)，必須滿足  
    //  X[f,u,v] ≤ Y[f,u,v] × (該邊上界)
    for (auto& entry : X) {
        int f, u, v;
        tie(f, u, v) = entry.first;
        double bound = 0;
        if (u < U && v < U) bound = capacity[u][v];
        else if (u >= U) bound = out_bound[sources[f]]; // 虛擬來源邊
        else if (v >= U) bound = in_bound[destinations[f]]; // 虛擬目的邊
        // MPConstraint* bind_cons = solver.MakeRowConstraint(-solver.infinity(), 0.0);
        MPConstraint* bind_cons = solver.MakeRowConstraint(0.0, solver.infinity());
        bind_cons->SetCoefficient(entry.second, -1.0);
        bind_cons->SetCoefficient(Y[entry.first], bound);
    }
  
    // (5) Y 與 Z 綁定：對每個真實邊 (u,v) (u,v ∈ [0,U))，對所有流 f 均須滿足：  
    //  Z[u,v] ≥ Y[f,u,v]
    for (auto& edge : all_edges) {
        int u = edge.first, v = edge.second;
        if (u < U && v < U) {
            for (int f = 0; f < F; ++f) {
                auto key = make_tuple(f, u, v);
                if (Y.find(key) != Y.end()) {
                    MPConstraint* yz_cons = solver.MakeRowConstraint(0.0, solver.infinity());
                    yz_cons->SetCoefficient(Y[key], -1.0);
                    yz_cons->SetCoefficient(Z[{u, v}], 1.0);
                }
            }
        }
    }
    // [新增排他性約束]
    // 對每條真實邊 (u,v) (u,v ∈ [0,U))，所有流 f 的 Y[f,u,v] 之和 ≤ 1，
    // 即每條物理邊同時只能被最多一條 SD 路使用
    // for (int u = 0; u < U; u++) {
    //     for (int v = 0; v < U; v++) {
    //         if (capacity[u][v] > 0) {  // 此物理邊存在
    //             MPConstraint* exclusivity = solver.MakeRowConstraint(0.0, 1.0);
    //             for (int f = 0; f < F; f++) {
    //                 auto key = make_tuple(f, u, v);
    //                 if (Y.find(key) != Y.end())
    //                     exclusivity->SetCoefficient(Y[key], 1.0);
    //             }
    //         }
    //     }
    // }
  
    // (6) 單一發射限制：對真實節點，每個節點 u (u ∈ [0,U)) 的所有從 u 出發的真實邊，
    //  其全局變數 Z 的總和 ≤ 1
    for (int u = 0; u < V; ++u) {
        MPConstraint* tx_cons = solver.MakeRowConstraint(0.0, 1.0);
        for (int v = 0; v < V; ++v) {
            if (u < U && v < U && Z.find({u, v}) != Z.end())
                tx_cons->SetCoefficient(Z[{u, v}], 1.0);
        }
    }
  
    // (7) 單一接收限制：對真實節點，每個節點 u (u ∈ [0,U)) 的所有進入 u 的真實邊，
    //  其全局變數 Z 的總和 ≤ 1
    for (int u = 0; u < V; ++u) {
        MPConstraint* rx_cons = solver.MakeRowConstraint(0.0, 1.0);
        for (int v = 0; v < V; ++v) {
            if (v < U && u < U && Z.find({v, u}) != Z.end())
                rx_cons->SetCoefficient(Z[{v, u}], 1.0);
        }
    }
  
    // 10. 求解模型
    MPSolver::ResultStatus result_status = solver.Solve();
    cerr << "[DEBUG] Solver status: " << result_status << "\n";
    cerr << "[DEBUG] Objective value: " << solver.Objective().Value() << "\n";
    if (result_status != MPSolver::OPTIMAL) {
        cerr << "[DEBUG] Solver did not find an optimal solution.\n";
        out << "0\n";
        return 0.0;
    }
  
    // 11. 記錄所有被使用的真實邊
    map<pair<int, int>, bool> edge_used_map;
    for (auto& [key, xvar] : X) {
        int f, u, v;
        tie(f, u, v) = key;
        if (u < U && v < U && xvar->solution_value() > 1e-6) {
            edge_used_map[{u, v}] = true;
        }
    }
    vector<pair<int, int>> used_edges;
    for (auto& [uv, flag] : edge_used_map) {
        if (flag) {
            used_edges.push_back(uv);
        }
    }
    // out << "--------------------------------\n";
    out << used_edges.size() << "\n";
    for (const auto& [u, v] : used_edges) {
        out << u << " " << v << "\n";
    }
    // out << "\n";

    // 12. 根據 X 變數的解，找出所有被使用的真實邊（u,v 且 u,v ∈ [0,U)，且流量 > 1e-6）
    double total_throughput = 0;
    for (int f = 0; f < F; ++f) {
        int s = flows[f].src, d = flows[f].dst;
        int vs = virSrcs[f], vd = virDsts[f];
        
        double rate = X[{f, vs, s}]->solution_value();
        
        cerr << "[DEBUG] Flow " << f << " rate: " << rate << " (src=" << s << ", dst=" << d << ", vs=" << vs << ", vd=" << vd << ")\n";
        
        if (rate < 1e-6) {
            out << "0 0\n";
        } else {
            // BFS to reconstruct path from source to destination
            unordered_map<int, int> parent;
            queue<int> q;
            q.push(s);  // 從實際源點開始
            parent[s] = -1;

            bool found_path = false;
            while (!q.empty()) {
                int u = q.front(); q.pop();
                if (u == d) {  // 找到實際終點
                    found_path = true;
                    break;
                }

                for (int v = 0; v < V; ++v) {
                    auto it = X.find({f, u, v});
                    if (it != X.end() && it->second->solution_value() > 1e-6 && !parent.count(v)) {
                        parent[v] = u;
                        q.push(v);
                    }
                }
            }

            if (!found_path) {
                cerr << "[DEBUG] Path not found for flow " << f << " from " << s << " to " << d << "\n";
                out << rate << " 0\n";
            } else {
                vector<int> path;
                int curr = d;
                while (curr != -1) {
                    if (curr < U) {  // 只包含實際節點
                        path.push_back(curr);
                    }
                    curr = parent[curr];
                }
                reverse(path.begin(), path.end());

                // out << fixed << setprecision(6) << rate << " " << path.size() << "    ";
                out << fixed << setprecision(6) << rate << " " << path.size();
                // out << vs;
                for (int u : path) out << " " << u;
                out << "\n";
                // out << " " << vd << "\n";
                total_throughput += rate;
            }
        }
    }
    
    // out << "total throughput: " << fixed << setprecision(6) << total_throughput << "\n";
    out << fixed << setprecision(6) << total_throughput << "\n";
    cerr << "[DEBUG] Total throughput: " << total_throughput << "\n";
    
    return total_throughput;
}

// 將所有測資案例依序求解，並輸出學號、各測資結果及平均 throughput
void SolveAllCases(istream& in, ostream& out) {
    int T;
    in >> T;
    out << student_id << "\n";
    
    double total_throughput = 0.0;
    for (int t = 0; t < T; ++t) {
        int U, E, F;
        in >> U >> E >> F;
        cerr << "[DEBUG] Test case " << t << " U: " << U << " E: " << E << " F: " << F << "\n";
        double thr = SolveSingleTestCase(U, E, F, in, out);
        total_throughput += thr;
        cerr << "[DEBUG] Test case " << t << " throughput: " << thr << "\n\n";
        out.flush();
    }
    
    double avg_throughput = total_throughput / T;
    out << fixed << setprecision(6) << avg_throughput << "\n";
    cerr << "[DEBUG] Average throughput across all cases: " << avg_throughput << "\n";
}

}  // namespace operations_research

int main() {
    operations_research::SolveAllCases(cin, cout);
    return 0;
}
