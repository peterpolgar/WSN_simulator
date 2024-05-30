#include <vector>
#include <iostream>

int Where_to_send(int source_idx, std::vector<std::vector<int>>* metric_matrix, unsigned number_of_sinks)
{
    long long best_koltseg = 123987654321;
    int best_dest_id = -1;
    for ( unsigned i = 0; i < number_of_sinks; ++i ) {
        long long local_koltseg = 123987654321;
        int local_id = -1;
        
        unsigned N = metric_matrix->size();
        std::vector<unsigned> Q;
        std::vector<long long> dist;
        std::vector<int> prev;
        for ( unsigned j = 0; j < N; ++j ) {
            Q.push_back(j);
            dist.push_back(123987654321);
            prev.push_back(-1);
        }
        dist[source_idx] = 0;
        while ( Q.size() > 0 ) {
            long long minv = 123987654321;
            unsigned u = Q[0];
            for ( unsigned j = 0; j < Q.size(); ++j ) {
                if ( dist[Q[j]] < minv ) {
                    minv = dist[Q[j]];
                    u = Q[j];
                }
            }
            if ( u == i ) {
                unsigned x = i;
                local_koltseg = 0;
                while ( prev[x] != source_idx && prev[x] != -1 ) {
                    local_koltseg += (*metric_matrix)[prev[x]][x];
                    x = prev[x];
                }
                if ( prev[x] == -1 ) {
                    local_id = -1;
                    local_koltseg = 123987654321;
                    break;
                }else {
                    local_id = x;
                    local_koltseg += (*metric_matrix)[source_idx][x];
                    break;
                }
            }
            for ( unsigned j = 0; j < Q.size(); ++j ) {
                if ( Q[j] == u ) {
                    Q.erase(Q.begin() + j);
                    break;
                }
            }
            for ( unsigned v = 0; v < (*metric_matrix)[u].size(); ++v ) {
                long long alt = 123987654321;
                bool is_v_in_Q = false;
                for ( unsigned k = 0; k < Q.size(); ++k ) {
                    if ( Q[k] == v ) {
                        is_v_in_Q = true;
                        break;
                    }
                }
                if ( (*metric_matrix)[u][v] > 0 and is_v_in_Q ) {
                    alt = dist[u] + (*metric_matrix)[u][v];
                }
                if ( alt < dist[v] ) {
                    dist[v] = alt;
                    prev[v] = u;
                }
            }
        }
        if ( Q.size() == 0 ) {
            local_id = -1;
        }
        std::cout << i << " koltsege: " << local_koltseg << "\n";
        
        if ( local_koltseg < best_koltseg ) {
            best_koltseg = local_koltseg;
            best_dest_id = local_id;
        }
    }
    return best_dest_id;
}

int main(int argc, char const *argv[])
{
    std::vector<std::vector<int>> metric_matrix = {
        {-1, 4, -1, -1, -1, -1, -1, 8, -1},
        {4, -1, 8, -1, -1, -1, -1, 11, -1},
        {-1, 8, -1, 7, -1, 4, -1, -1, 2},
        {-1, -1, 7, -1, 9, 14, -1, -1, -1},
        {-1, -1, -1, 9, -1, 10, -1, -1, -1},
        {-1, -1, 4, 14, 10, -1, 2, -1, -1},
        {-1, -1, -1, -1, -1, 2, -1, 1, 6},
        {8, 11, -1, -1, -1, -1, 1, -1, 7},
        {-1, -1, 2, -1, -1, -1, 6, 7, -1}
    };
    std::cout << "where to send: " << Where_to_send(4, &metric_matrix, 3) << '\n';
    return 0;
}