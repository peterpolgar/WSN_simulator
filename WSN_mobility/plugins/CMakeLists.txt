# placement plugins
# ----------------------------------------------------------------------------

qt_add_plugin(poisson_disk_placement_plugin
    CLASS_NAME cPoisson_disk_placement
    placement/poisson_disk_sampling/poisson_disk_placement_plugin.cpp placement/poisson_disk_sampling/poisson_disk_placement_plugin.h
    ../cslider.h ../cslider.cpp
    ../cfp_slider.h ../cfp_slider.cpp
)

set_target_properties(poisson_disk_placement_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/placement"
)

target_include_directories(poisson_disk_placement_plugin PRIVATE
    ..
)

target_link_libraries(poisson_disk_placement_plugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ----------------------------------------------------------------------------
# mobility plugins
# ----------------------------------------------------------------------------

qt_add_plugin(random_walk_mobility_plugin
    CLASS_NAME cRandom_walk_mobility
    mobility/random_walk/random_walk_mobility_plugin.cpp mobility/random_walk/random_walk_mobility_plugin.h
    ../cslider.h ../cslider.cpp
    ../cfp_slider.h ../cfp_slider.cpp
)

set_target_properties(random_walk_mobility_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/mobility"
)

target_include_directories(random_walk_mobility_plugin PRIVATE
    ..
)

target_link_libraries(random_walk_mobility_plugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ----------------------------------------------------------------------------

qt_add_plugin(random_waypoint_mobility_plugin
    CLASS_NAME cRandom_waypoint_mobility
    mobility/random_waypoint/random_waypoint_mobility_plugin.cpp mobility/random_waypoint/random_waypoint_mobility_plugin.h
    ../cslider.h ../cslider.cpp
    ../cfp_slider.h ../cfp_slider.cpp
)

set_target_properties(random_waypoint_mobility_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/mobility"
)

target_include_directories(random_waypoint_mobility_plugin PRIVATE
    ..
)

target_link_libraries(random_waypoint_mobility_plugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ----------------------------------------------------------------------------

qt_add_plugin(directed_multi_direction_mobility_plugin
    CLASS_NAME cDirected_multi_direction_mobility
    mobility/directed_multi_direction/directed_multi_direction_mobility_plugin.cpp mobility/directed_multi_direction/directed_multi_direction_mobility_plugin.h
    ../cslider.h ../cslider.cpp
    ../cfp_slider.h ../cfp_slider.cpp
)

set_target_properties(directed_multi_direction_mobility_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/mobility"
)

target_include_directories(directed_multi_direction_mobility_plugin PRIVATE
    ..
)

target_link_libraries(directed_multi_direction_mobility_plugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ----------------------------------------------------------------------------

qt_add_plugin(directed_unique_direction_mobility_plugin
    CLASS_NAME cDirected_unique_direction_mobility
    mobility/directed_unique_direction/directed_unique_direction_mobility_plugin.cpp mobility/directed_unique_direction/directed_unique_direction_mobility_plugin.h
    ../cslider.h ../cslider.cpp
    ../cfp_slider.h ../cfp_slider.cpp
)

set_target_properties(directed_unique_direction_mobility_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/mobility"
)

target_include_directories(directed_unique_direction_mobility_plugin PRIVATE
    ..
)

target_link_libraries(directed_unique_direction_mobility_plugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ----------------------------------------------------------------------------

qt_add_plugin(random_direction_mobility_plugin
    CLASS_NAME cRandom_direction_mobility
    mobility/random_direction/random_direction_mobility_plugin.cpp mobility/random_direction/random_direction_mobility_plugin.h
    ../cslider.h ../cslider.cpp
    ../cfp_slider.h ../cfp_slider.cpp
)

set_target_properties(random_direction_mobility_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/mobility"
)

target_include_directories(random_direction_mobility_plugin PRIVATE
    ..
)

target_link_libraries(random_direction_mobility_plugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ----------------------------------------------------------------------------

qt_add_plugin(gauss_markov_mobility_plugin
    CLASS_NAME cGauss_markov_mobility
    mobility/gauss_markov/gauss_markov_mobility_plugin.cpp mobility/gauss_markov/gauss_markov_mobility_plugin.h
    ../cslider.h ../cslider.cpp
    ../cfp_slider.h ../cfp_slider.cpp
)

set_target_properties(gauss_markov_mobility_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/mobility"
)

target_include_directories(gauss_markov_mobility_plugin PRIVATE
    ..
)

target_link_libraries(gauss_markov_mobility_plugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
