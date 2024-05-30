#ifndef PARAMS_ENUM_H
#define PARAMS_ENUM_H

enum class eParams {
    number_of_measurements,
    number_of_extreme_measurements,
    number_of_received_frames_on_sinks,
    average_hopcount_of_received_frames,
    average_sending_time_of_received_frames,
    relative_delivery_error,
    average_energy_of_nodes,
    average_comm_energy,
    sink_positions,
    node_positions,
    percentage_of_alive_nodes
};

#endif // PARAMS_ENUM_H
