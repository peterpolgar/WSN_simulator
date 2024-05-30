#ifndef CSIM_RUN_H
#define CSIM_RUN_H

#include <QLabel>
#include <QObject>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QLineEdit>
#include <QFuture>
#include <QPromise>
#include "placement_interface.h"
#include "mobility_interface.h"
#include "cslider.h"
#include "cfp_slider.h"

class cEvent_space;

class cSim_run : public QObject
{
    Q_OBJECT
public:
    cSim_run(cEvent_space *event_space, QPushButton *play_button, QLabel *aoi_width_label, QLabel *aoi_height_label, QLabel *aoi_unit_len_label,
             QLabel *aoi_unit_width_label, QLabel *aoi_unit_height_label, QLabel *no_sinks_label, QLabel *no_nodes_label, QLineEdit *lineedit_of_time,
             cFp_slider *simul_speed_slider, cFp_slider *comm_range_slider, cSlider *aoi_unit_len_slider, cSlider *aoi_unit_width_slider,
             cSlider *aoi_unit_height_slider, cSlider *sink_size_slider, cSlider *no_sinks_slider, cSlider *node_size_slider, cSlider *no_nodes_slider,
             QComboBox *sink_plac_combo, QComboBox *node_plac_combo, QComboBox *sink_mobi_combo, QComboBox *node_mobi_combo, QLabel *loaded_filename_label);
    ~cSim_run();
    void Do_placement(bool is_sink = true, bool is_node = true);
    void Start_mobility_thread(QPromise<void> &promise);
public slots:
    void Play_pause();
    void Stop(bool clicked = false);
    void AoI_unit_length(int value);
    void AoI_unit_width(int value);
    void AoI_unit_height(int value);
    void No_sinks(int value);
    void Sink_placement_algorithm(int index);
    void Sink_mobility_algorithm(int index);
    void No_nodes(int value);
    void Node_placement_algorithm(int index);
    void Node_mobility_algorithm(int index);
    void Sink_meter_size(int value);
    void Node_meter_size(int value);
    void Copy_params_to_clipboard();
    void Simul_speed(double value);
    void Time_edit();
    void Time_step_edit(const QString &text);
    void Add_time_step();
    void Subtract_time_step();
    void Set_movements_to_the_global_time(long long time_delta);
    void Comm_range(double value);
    void Import_config_file();

private:
    cEvent_space *event_space;
    QPushButton *play_button;
    QIcon play_icon, pause_icon;
    int aoi_unit_length;

//  labels
    QLabel *aoi_width_label, *aoi_height_label, *aoi_unit_len_label, *aoi_unit_width_label, *aoi_unit_height_label, *no_sinks_label, *no_nodes_label, *loaded_filename_label;

//  sliders
    cFp_slider *simul_speed_slider, *comm_range_slider;
    cSlider *aoi_unit_len_slider, *aoi_unit_width_slider, *aoi_unit_height_slider, *sink_size_slider, *no_sinks_slider, *node_size_slider, *no_nodes_slider;

//  comboboxes
    QComboBox *sink_plac_combo, *node_plac_combo, *sink_mobi_combo, *node_mobi_combo;

public:
    int sink_placement_algorithm_index;
    int sink_mobility_algorithm_index;
    int node_placement_algorithm_index;
    int node_mobility_algorithm_index;

    int aoi_m_width;
    int aoi_m_height;

//  sizes
    double sink_meter_size;
    double node_meter_size;

    bool is_running;
    bool is_first_play;
    bool is_mobi_initialized;
    int aoi_unit_width;
    int aoi_unit_height;
    int no_sinks;
    int no_nodes;
    double simul_speed;
    double comm_range_squared;

//  times
    unsigned long long global_time_ms;
    unsigned long long step_time_ms;
    QLineEdit *lineedit_of_time;

//  positions
    std::vector<double> sink_positions;
    std::vector<double> node_positions;

//  algorithms
    std::vector<iPlacement_interface*> sink_placement_algorithms;
    std::vector<iPlacement_interface*> node_placement_algorithms;
    std::vector<iMobility_interface*> sink_mobility_algorithms;
    std::vector<iMobility_interface*> node_mobility_algorithms;

    QFuture<void> future;
};

#endif // CSIM_RUN_H
