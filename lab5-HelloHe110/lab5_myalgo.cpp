#include <bits/stdc++.h>
using namespace std;

using pii = pair<int, int>;
using pdi = pair<double, int>;

struct Edge {
    int from, to;
    double capacity, used = 0;
};

struct Instance {
    int n, m, f;
    vector<Edge> edges;
    vector<vector<int>> adj;  // adj[i] = edge ids
    vector<pii> sd_pairs;
};

vector<Instance> read_input() {
    int t;
    cin >> t;
    vector<Instance> instances(t);

    for (int ti = 0; ti < t; ++ti) {
        auto& ins = instances[ti];
        cin >> ins.n >> ins.m >> ins.f;
        ins.adj.assign(ins.n, {});
        for (int i = 0; i < ins.m; ++i) {
            int u, v;
            double cap;
            cin >> u >> v >> cap;
            int id = ins.edges.size();
            ins.edges.push_back({u, v, cap});
            ins.edges.push_back({v, u, cap});
            ins.adj[u].push_back(id);
            ins.adj[v].push_back(id + 1);
        }
        for (int i = 0; i < ins.f; ++i) {
            int s, d;
            cin >> s >> d;
            ins.sd_pairs.emplace_back(s, d);
        }
    }
    return instances;
}

bool find_path(const Instance& ins, vector<Edge>& edges,
               const vector<bool>& tx_used, const vector<bool>& rx_used,
               int s, int d, vector<int>& path, double& bottleneck) {
    int n = ins.n;
    vector<double> dist(n, 1e18);
    vector<int> prev_edge(n, -1);
    priority_queue<pdi, vector<pdi>, greater<>> pq;

    dist[s] = 0;
    pq.emplace(0.0, s);

    while (!pq.empty()) {
        auto [cost, u] = pq.top(); pq.pop();
        if (u == d) break;
        for (int eid : ins.adj[u]) {
            auto& e = edges[eid];
            if (e.used >= e.capacity) continue;
            if (tx_used[e.from] || rx_used[e.to]) continue;

            double weight = 1.0 + e.used * 1e4 / max(e.capacity, 1.0);
            if (dist[e.to] > dist[u] + weight) {
                dist[e.to] = dist[u] + weight;
                prev_edge[e.to] = eid;
                pq.emplace(dist[e.to], e.to);
            }
        }
    }

    if (prev_edge[d] == -1) return false;

    // Reconstruct path
    bottleneck = 1e18;
    int cur = d;
    while (cur != s) {
        int eid = prev_edge[cur];
        path.push_back(eid);
        bottleneck = min(bottleneck, edges[eid].capacity - edges[eid].used);
        cur = edges[eid].from;
    }
    reverse(path.begin(), path.end());
    return true;
}

int main() {
    const string student_id = "111550022";
    cout << student_id << '\n';

    auto instances = read_input();
    double total_all = 0;

    for (const auto& ins : instances) {
        vector<Edge> edges = ins.edges;
        vector<bool> tx_used(ins.n, false), rx_used(ins.n, false);
        set<pii> used_links;
        vector<tuple<double, vector<int>>> flow_paths;
        double total = 0;

        for (auto [s, d] : ins.sd_pairs) {
            vector<int> path;
            double rate = 0;
            if (find_path(ins, edges, tx_used, rx_used, s, d, path, rate)) {
                for (int eid : path) {
                    edges[eid].used += rate;
                    tx_used[edges[eid].from] = true;
                    rx_used[edges[eid].to] = true;
                    used_links.insert({edges[eid].from, edges[eid].to});
                }
                total += rate;
                vector<int> path_nodes = {s};
                for (int eid : path) path_nodes.push_back(edges[eid].to);
                flow_paths.push_back({rate, path_nodes});
            } else {
                flow_paths.push_back({0.0, {}});
            }
        }

        // Output
        cout << used_links.size() << '\n';
        for (auto [u, v] : used_links) cout << u << ' ' << v << '\n';
        for (auto [rate, path] : flow_paths) {
            if (rate == 0) {
                cout << "0 0\n";
            } else {
                cout << fixed << setprecision(6) << rate << ' ' << path.size();
                for (int v : path) cout << ' ' << v;
                cout << '\n';
            }
        }
        cout << fixed << setprecision(6) << total << '\n';
        total_all += total;
    }

    cout << fixed << setprecision(6) << total_all / instances.size() << '\n';
    return 0;
}