#!/usr/bin/env bash

alap_dir=$1

module load gcc

cd ${alap_dir}/plugins/routing
rm *.so

cd dijkstra
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -shared -fPIC -o ../dijkstra_routing_plugin.so dijkstra_routing_plugin.cpp

cd ${alap_dir}/plugins/mobility
rm *.so

cd random_walk
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -shared -fPIC -o ../random_walk_mobility_plugin.so random_walk_mobility_plugin.cpp
cd ../random_waypoint
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -shared -fPIC -o ../random_waypoint_mobility_plugin.so random_waypoint_mobility_plugin.cpp
cd ../random_direction
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -shared -fPIC -o ../random_direction_mobility_plugin.so random_direction_mobility_plugin.cpp
cd ../gauss_markov
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -shared -fPIC -o ../gauss_markov_mobility_plugin.so gauss_markov_mobility_plugin.cpp
cd ../directed_unique_direction
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -shared -fPIC -o ../directed_unique_direction_mobility_plugin.so directed_unique_direction_mobility_plugin.cpp
cd ../directed_multi_direction
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -shared -fPIC -o ../directed_multi_direction_mobility_plugin.so directed_multi_direction_mobility_plugin.cpp

cd ${alap_dir}
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -ldl -o the_simulation main.cpp
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -o csv_gen prod_csv_gen.cpp
