#include <vector>
#include "dijkstra_routing_plugin.h"

#define EXPORT __attribute__((visibility("default")))

extern "C" EXPORT iRouting_interface* getObj(void) {
    return (iRouting_interface*)(new cDijkstra_routing);
}

cDijkstra_routing::cDijkstra_routing()
{
    
}

std::string cDijkstra_routing::Alg_name()
{
    return "Dijkstra";
}

void cDijkstra_routing::Get_client_params(std::vector<std::vector<double>>* metric_matrix, unsigned number_of_sinks)
{
    this->metric_matrix = metric_matrix;
    this->number_of_sinks = number_of_sinks;
}

void cDijkstra_routing::Init_run()
{
    
}

int cDijkstra_routing::Where_to_send(int source_idx)
{
    double best_koltseg = 123987654321.;
    int best_dest_id = -1;
    for ( unsigned i = 0; i < number_of_sinks; ++i ) {
        double local_koltseg = 123987654321.;
        int local_id = -1;
        
        unsigned N = metric_matrix->size();
        std::vector<unsigned> Q;
        std::vector<double> dist;
        std::vector<int> prev;
        for ( unsigned j = 0; j < N; ++j ) {
            Q.push_back(j);
            dist.push_back(123987654321.);
            prev.push_back(-1);
        }
        dist[source_idx] = 0;
        while ( Q.size() > 0 ) {
            double minv = 123987654321.;
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
                    local_koltseg = 123987654321.;
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
                double alt = 123987654321.;
                bool is_v_in_Q = false;
                for ( unsigned k = 0; k < Q.size(); ++k ) {
                    if ( Q[k] == v ) {
                        is_v_in_Q = true;
                        break;
                    }
                }
                if ( (*metric_matrix)[u][v] > 0. and is_v_in_Q ) {
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
        
        if ( local_koltseg < best_koltseg ) {
            best_koltseg = local_koltseg;
            best_dest_id = local_id;
        }
    }
    return best_dest_id;
}

void cDijkstra_routing::Set_parameter(std::string param_str, std::string ertek_str)
{
    
}

iRouting_interface* cDijkstra_routing::Create_new_instance()
{
    return new cDijkstra_routing();
}

std::string cDijkstra_routing::Get_params_string()
{
    std::string x("");
    if ( metric_matrix != nullptr ) {
        x += "metric_matrix is not nullptr\n";
    }else {
        x += "Error: metric_matrix is nullptr\n";
    }
    x += "number of sinks,";
    x += std::to_string(number_of_sinks);
    return x;
}