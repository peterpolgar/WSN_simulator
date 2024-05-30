#include "widget.h"
#include "placement_interface.h"
#include "mobility_interface.h"

#include <iostream>
#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include <cstdlib>
#include <functional>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    this->resize(1920, 1080);
    this->setWindowTitle("WSN mobility");

    main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    event_space = new cEvent_space();
    main_layout->addWidget(event_space, 5);
    auto main_right_layout = new QVBoxLayout();
    main_right_layout->setContentsMargins(0, 0, 0, 0);

    class cTrwidget : public QWidget {
    public:
        cTrwidget(cMy_scroll_area *sa, QWidget *parent = nullptr) : QWidget(parent) {
            this->sa = sa;
        }
        void wheelEvent(QWheelEvent *event) override {
            sa->wheelEvent(event);
        }
    private:
        cMy_scroll_area *sa;
    };

    par_list_scroll = new cMy_scroll_area();
    auto atrwidget = new cTrwidget(par_list_scroll);
    atrwidget->setObjectName("atrwidget");
    auto atrlayout = new QVBoxLayout(atrwidget);
    atrlayout->setContentsMargins(0, 0, 0, 0);
    par_list_scroll->setWidgetResizable(true);
    auto par_widget = new QWidget();
    par_list_layout = new QVBoxLayout(par_widget);
    par_list_layout->setContentsMargins(0, 0, 0, 0);

    auto copy_params_to_clipboard = new QPushButton("Copy parameters to clipboard");
    atrlayout->addWidget(copy_params_to_clipboard);

    auto import_conf_file_button = new QPushButton("Import configuration");
    atrlayout->addWidget(import_conf_file_button);

    auto imported_file_label = new QLabel("<b>Loaded file:</b>");
    atrlayout->addWidget(imported_file_label);

    QIcon play_icon;
    play_icon.addFile(QString::fromUtf8(":/images/icons8-play-50.png"), QSize(), QIcon::Normal, QIcon::Off);
    play_button = new QPushButton();
    play_button->setIcon(play_icon);

    QIcon stop_icon;
    stop_icon.addFile(QString::fromUtf8(":/images/icons8-stop-50.png"), QSize(), QIcon::Normal, QIcon::Off);
    stop_button = new QPushButton();
    stop_button->setIcon(stop_icon);

    play_layout = new QHBoxLayout();
    play_layout->addWidget(play_button);
    play_layout->addWidget(stop_button);
    play_layout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    play_layout->setStretch(0, 1);
    play_layout->setStretch(1, 1);
    play_layout->setStretch(2, 1);

    atrlayout->addLayout(play_layout);

    auto stepping_layout = new QHBoxLayout();
    stepping_layout->setSpacing(0);
    auto time_lineedit = new QLineEdit("0");
    stepping_layout->addWidget(time_lineedit);
    stepping_layout->addWidget(new QLabel("ms"));
    auto add_time_button = new QPushButton("+");
    add_time_button->setMinimumWidth(20);
    stepping_layout->addWidget(add_time_button);
    auto subtract_time_button = new QPushButton("-");
    subtract_time_button->setMinimumWidth(20);
    stepping_layout->addWidget(subtract_time_button);
    auto time_step_lineedit = new QLineEdit("33");
    stepping_layout->addWidget(time_step_lineedit);
    stepping_layout->addWidget(new QLabel("ms"));
    stepping_layout->setStretch(0, 6);
    stepping_layout->setStretch(2, 1);
    stepping_layout->setStretch(3, 1);
    stepping_layout->setStretch(4, 6);

    atrlayout->addLayout(stepping_layout);

    auto simul_speed_label = new QLabel("Simulation speed [x]");

    atrlayout->addWidget(simul_speed_label);

    simul_speed_slider = new cFp_slider(0.25, 30, 1);
    simul_speed_slider->setObjectName("simul_speed_slider");

    atrlayout->addWidget(simul_speed_slider);

    atrlayout->addSpacerItem(new QSpacerItem(40, 5, QSizePolicy::Expanding, QSizePolicy::Fixed));

    main_right_layout->addWidget(atrwidget);

    par_list_layout->addWidget(new QLabel("Communication range [m]"));

    auto comm_range_slider = new cFp_slider(1, 1000, 255.852);
    par_list_layout->addWidget(comm_range_slider);

    aoi_head_label = new QLabel("<b>AoI</b>");
    aoi_head_label->setAlignment(Qt::AlignCenter);
    aoi_head_label->setObjectName("aoi_head_name");

    par_list_layout->addWidget(aoi_head_label);

    aoi_width_label = new QLabel("Width [m]: 160");
    aoi_height_label = new QLabel("Height [m]: 90");
    aoi_wh_layout = new QHBoxLayout();
    aoi_wh_layout->addWidget(aoi_width_label);
    aoi_wh_layout->addWidget(aoi_height_label);
    aoi_wh_layout->setStretch(0, 1);
    aoi_wh_layout->setStretch(1, 1);

    par_list_layout->addLayout(aoi_wh_layout);

    aoi_unit_len_label = new QLabel("AoI one unit length [m]: 10");

    par_list_layout->addWidget(aoi_unit_len_label);

    aoi_unit_len_slider = new cSlider(1, 1000, 10);

    par_list_layout->addWidget(aoi_unit_len_slider);

    aoi_unit_width_label = new QLabel("Width of AoI in units: 16");

    par_list_layout->addWidget(aoi_unit_width_label);

    aoi_unit_width_slider = new cSlider(1, 100, 16);

    par_list_layout->addWidget(aoi_unit_width_slider);

    aoi_unit_height_label = new QLabel("Height of AoI in units: 9");

    par_list_layout->addWidget(aoi_unit_height_label);

    aoi_unit_height_slider = new cSlider(1, 100, 9);

    par_list_layout->addWidget(aoi_unit_height_slider);

    sinks_head_label = new QLabel("<b>Sink(s)</b>");
    sinks_head_label->setAlignment(Qt::AlignCenter);
    sinks_head_label->setObjectName("sinks_head_label");

    par_list_layout->addWidget(sinks_head_label);

    auto sink_size_label = new QLabel("Diameter size of a sink [cm]");

    par_list_layout->addWidget(sink_size_label);

    auto sink_size_slider = new cSlider(1, 200, 40);

    par_list_layout->addWidget(sink_size_slider);

    no_sinks_label = new QLabel("Number of sinks: 5");

    par_list_layout->addWidget(no_sinks_label);

    no_sinks_slider = new cSlider(1, 20, 5);

    par_list_layout->addWidget(no_sinks_slider);

    sink_placement_label = new QLabel("<b>Sink Placement algorithm:</b>");

    par_list_layout->addWidget(sink_placement_label);

    sink_placement_combo = new QComboBox();

    par_list_layout->addWidget(sink_placement_combo);

    sink_placement_param_stack = new cStacked_widget();
    sink_placement_param_stack->setContentsMargins(0, 0, 0, 0);
    par_list_layout->addWidget(sink_placement_param_stack);

    sink_mobility_label = new QLabel("<b>Sink Mobility algorithm:</b>");

    par_list_layout->addWidget(sink_mobility_label);

    sink_mobility_combo = new QComboBox();
    sink_mobility_combo->addItem("Mobility NaN");

    par_list_layout->addWidget(sink_mobility_combo);

    sink_mobility_param_stack = new cStacked_widget();
    sink_mobility_param_stack->setContentsMargins(0, 0, 0, 0);
    auto zero_sink_widget = new cZero_widget();
    sink_mobility_param_stack->addWidget(zero_sink_widget);
    par_list_layout->addWidget(sink_mobility_param_stack);

    node_head_label = new QLabel("<b>Nodes</b>");
    node_head_label->setAlignment(Qt::AlignCenter);
    node_head_label->setObjectName("node_head_label");

    par_list_layout->addWidget(node_head_label);

    auto node_size_label = new QLabel("Diameter size of a node [cm]");

    par_list_layout->addWidget(node_size_label);

    auto node_size_slider = new cSlider(1, 200, 20);

    par_list_layout->addWidget(node_size_slider);

    no_nodes_label = new QLabel("Number of nodes: 30");

    par_list_layout->addWidget(no_nodes_label);

    no_nodes_slider = new cSlider(1, 200, 30);

    par_list_layout->addWidget(no_nodes_slider);

    node_placement_label = new QLabel("<b>Node Placement algorithm:</b>");

    par_list_layout->addWidget(node_placement_label);

    node_placement_combo = new QComboBox();

    par_list_layout->addWidget(node_placement_combo);

    node_placement_param_stack = new cStacked_widget();
    node_placement_param_stack->setContentsMargins(0, 0, 0, 0);
    par_list_layout->addWidget(node_placement_param_stack);

    node_mobility_label = new QLabel("<b>Node Mobility algorithm:</b>");

    par_list_layout->addWidget(node_mobility_label);

    node_mobility_combo = new QComboBox();
    node_mobility_combo->addItem("Mobility NaN");

    par_list_layout->addWidget(node_mobility_combo);

    node_mobility_param_stack = new cStacked_widget();
    node_mobility_param_stack->setContentsMargins(0, 0, 0, 0);
    auto zero_node_widget = new cZero_widget();
    node_mobility_param_stack->addWidget(zero_node_widget);
    par_list_layout->addWidget(node_mobility_param_stack);

    par_list_layout->addStretch();

    par_list_scroll->setWidget(par_widget);
    main_right_layout->addWidget(par_list_scroll);
    main_layout->addLayout(main_right_layout, 1);

//    lllllllllllllllllllllllllllllllllllllllllllllllllll

    simul = new cSim_run(event_space, play_button, aoi_width_label, aoi_height_label, aoi_unit_len_label, aoi_unit_width_label, aoi_unit_height_label, no_sinks_label,
                         no_nodes_label, time_lineedit, simul_speed_slider, comm_range_slider, aoi_unit_len_slider, aoi_unit_width_slider, aoi_unit_height_slider,
                         sink_size_slider, no_sinks_slider, node_size_slider, no_nodes_slider, sink_placement_combo, node_placement_combo, sink_mobility_combo,
                         node_mobility_combo, imported_file_label);
    event_space->Set_sim_run(simul);
    event_space->Set_circle_draw_radius_additional();
    QObject::connect(play_button, &QPushButton::clicked, simul, &cSim_run::Play_pause);
    QObject::connect(stop_button, &QPushButton::clicked, simul, std::bind(&cSim_run::Stop, simul, true));
    QObject::connect(simul_speed_slider, &cFp_slider::Fp_value_changed, simul, &cSim_run::Simul_speed);
    QObject::connect(comm_range_slider, &cFp_slider::Fp_value_changed, simul, &cSim_run::Comm_range);
    QObject::connect(aoi_unit_len_slider, &QSlider::valueChanged, simul, &cSim_run::AoI_unit_length);
    QObject::connect(aoi_unit_height_slider, &QSlider::valueChanged, simul, &cSim_run::AoI_unit_height);
    QObject::connect(aoi_unit_width_slider, &QSlider::valueChanged, simul, &cSim_run::AoI_unit_width);
    QObject::connect(no_sinks_slider, &QSlider::valueChanged, simul, &cSim_run::No_sinks);
    QObject::connect(sink_placement_combo, &QComboBox::currentIndexChanged, sink_placement_param_stack, &QStackedWidget::setCurrentIndex);
    QObject::connect(sink_placement_combo, &QComboBox::currentIndexChanged, simul, &cSim_run::Sink_placement_algorithm);
    QObject::connect(sink_mobility_combo, &QComboBox::currentIndexChanged, sink_mobility_param_stack, &QStackedWidget::setCurrentIndex);
    QObject::connect(sink_mobility_combo, &QComboBox::currentIndexChanged, simul, &cSim_run::Sink_mobility_algorithm);
    QObject::connect(no_nodes_slider, &QSlider::valueChanged, simul, &cSim_run::No_nodes);
    QObject::connect(node_placement_combo, &QComboBox::currentIndexChanged, node_placement_param_stack, &QStackedWidget::setCurrentIndex);
    QObject::connect(node_placement_combo, &QComboBox::currentIndexChanged, simul, &cSim_run::Node_placement_algorithm);
    QObject::connect(node_mobility_combo, &QComboBox::currentIndexChanged, node_mobility_param_stack, &QStackedWidget::setCurrentIndex);
    QObject::connect(node_mobility_combo, &QComboBox::currentIndexChanged, simul, &cSim_run::Node_mobility_algorithm);
    QObject::connect(sink_size_slider, &QSlider::valueChanged, simul, &cSim_run::Sink_meter_size);
    QObject::connect(node_size_slider, &QSlider::valueChanged, simul, &cSim_run::Node_meter_size);
    QObject::connect(copy_params_to_clipboard, &QPushButton::clicked, simul, &cSim_run::Copy_params_to_clipboard);
    QObject::connect(import_conf_file_button, &QPushButton::clicked, simul, &cSim_run::Import_config_file);
    QObject::connect(time_lineedit, &QLineEdit::editingFinished, simul, &cSim_run::Time_edit);
    QObject::connect(time_step_lineedit, &QLineEdit::textEdited, simul, &cSim_run::Time_step_edit);
    QObject::connect(add_time_button, &QPushButton::clicked, simul, &cSim_run::Add_time_step);
    QObject::connect(subtract_time_button, &QPushButton::clicked, simul, &cSim_run::Subtract_time_step);

//    lllllllllllllllllllllllllllllllllllllllllllllllllll

    QDir pluginsDir(QCoreApplication::applicationDirPath());
    pluginsDir.cd("plugins/placement");
    const QStringList entries = pluginsDir.entryList(QDir::Files);
    for (const QString &fileName : entries) {
//        for sinks
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            iPlacement_interface *tmp = qobject_cast<iPlacement_interface *>(plugin);
            if (tmp) {
                if ( tmp->Is_for_sinks() ) {
                    sink_placement_combo->addItem(tmp->Alg_name());
                    simul->sink_placement_algorithms.push_back(tmp);
                    if ( auto gp = tmp->Get_parameters(); gp != nullptr ) {
                        int alg_index = simul->sink_placement_algorithms.size() - 1;
                        auto spps_widget = new QWidget();
                        std::vector<QWidget*> pass_widgets;
                        auto spps_layout = new QVBoxLayout();
                        spps_layout->setContentsMargins(11, 0, 0, 0);
                        spps_layout->addWidget(new QLabel("Its parameters:"));
                        for ( int i = 0; i < gp->size(); ++i ) {
                            auto& info = gp->at(i);
                            auto par_label = new QLabel(info.name);
                            par_label->setAlignment(Qt::AlignCenter);
                            spps_layout->addWidget(par_label);
                            pass_widgets.push_back(par_label);
                            if ( info.is_categorical ) {
                                auto coc = new QComboBox();
                                for ( int j = 0; j < info.categorical_values->size(); ++j ) {
                                    coc->addItem(info.categorical_values->at(j));
                                }
                                coc->setCurrentIndex(info.def_categorical_value_index);
                                spps_layout->addWidget(coc);
                                pass_widgets.push_back(coc);
                                QObject::connect(coc, &QComboBox::currentIndexChanged, this, [this, info, alg_index](int idx){
                                    simul->sink_placement_algorithms.at(alg_index)->Set_parameter(info.id, idx);
                                    if ( ! (simul->is_first_play) ) {
                                        simul->Stop();
                                    }
                                    simul->Do_placement(true, false);
                                });
                            }else if( info.is_integer ) {
                                auto slc = new cSlider(info.range_start, info.range_end, info.def_number_value);
                                spps_layout->addWidget(slc);
                                pass_widgets.push_back(slc);
                                QObject::connect(slc, &QSlider::valueChanged, this, [this, info, alg_index](int val){
                                    simul->sink_placement_algorithms.at(alg_index)->Set_parameter(info.id, val);
                                    if ( ! (simul->is_first_play) ) {
                                        simul->Stop();
                                    }
                                    simul->Do_placement(true, false);
                                });
                            }else {
                                auto slc = new cFp_slider(info.range_start, info.range_end, info.def_number_value);
                                spps_layout->addWidget(slc);
                                pass_widgets.push_back(slc);
                                QObject::connect(slc, &cFp_slider::Fp_value_changed, this, [this, info, alg_index](double val){
                                    simul->sink_placement_algorithms.at(alg_index)->Set_parameter(info.id, val);
                                    if ( ! (simul->is_first_play) ) {
                                        simul->Stop();
                                    }
                                    simul->Do_placement(true, false);
                                });
                            }
                        }
                        tmp->Get_widgets(pass_widgets);
                        spps_widget->setLayout(spps_layout);
                        sink_placement_param_stack->addWidget(spps_widget);
                    }else {
                        auto null_widget = new cZero_widget();
                        sink_placement_param_stack->addWidget(null_widget);
                    }
                }

//        for nodes
                tmp = tmp->Create_new_instance();
                if ( tmp->Is_for_nodes() ) {
                    node_placement_combo->addItem(tmp->Alg_name());
                    simul->node_placement_algorithms.push_back(tmp);
                    if ( auto gp = tmp->Get_parameters(); gp != nullptr ) {
                        int alg_index = simul->node_placement_algorithms.size() - 1;
                        auto spps_widget = new QWidget();
                        std::vector<QWidget*> pass_widgets;
                        auto spps_layout = new QVBoxLayout();
                        spps_layout->setContentsMargins(11, 0, 0, 0);
                        spps_layout->addWidget(new QLabel("Its parameters:"));
                        for ( int i = 0; i < gp->size(); ++i ) {
                            auto& info = gp->at(i);
                            auto par_label = new QLabel(info.name);
                            par_label->setAlignment(Qt::AlignCenter);
                            spps_layout->addWidget(par_label);
                            pass_widgets.push_back(par_label);
                            if ( info.is_categorical ) {
                                auto coc = new QComboBox();
                                for ( int j = 0; j < info.categorical_values->size(); ++j ) {
                                    coc->addItem(info.categorical_values->at(j));
                                }
                                coc->setCurrentIndex(info.def_categorical_value_index);
                                spps_layout->addWidget(coc);
                                pass_widgets.push_back(coc);
                                QObject::connect(coc, &QComboBox::currentIndexChanged, this, [this, info, alg_index](int idx){
                                    simul->node_placement_algorithms.at(alg_index)->Set_parameter(info.id, idx);
                                    if ( ! (simul->is_first_play) ) {
                                        simul->Stop();
                                    }
                                    simul->Do_placement(false, true);
                                });
                            }else if( info.is_integer ) {
                                auto slc = new cSlider(info.range_start, info.range_end, info.def_number_value);
                                spps_layout->addWidget(slc);
                                pass_widgets.push_back(slc);
                                QObject::connect(slc, &QSlider::valueChanged, this, [this, info, alg_index](int val){
                                    simul->node_placement_algorithms.at(alg_index)->Set_parameter(info.id, val);
                                    if ( ! (simul->is_first_play) ) {
                                        simul->Stop();
                                    }
                                    simul->Do_placement(false, true);
                                });
                            }else {
                                auto slc = new cFp_slider(info.range_start, info.range_end, info.def_number_value);
                                spps_layout->addWidget(slc);
                                pass_widgets.push_back(slc);
                                QObject::connect(slc, &cFp_slider::Fp_value_changed, this, [this, info, alg_index](double val){
                                    simul->node_placement_algorithms.at(alg_index)->Set_parameter(info.id, val);
                                    if ( ! (simul->is_first_play) ) {
                                        simul->Stop();
                                    }
                                    simul->Do_placement(false, true);
                                });
                            }
                        }
                        tmp->Get_widgets(pass_widgets);
                        spps_widget->setLayout(spps_layout);
                        node_placement_param_stack->addWidget(spps_widget);
                    }else {
                        auto null_widget = new cZero_widget();
                        node_placement_param_stack->addWidget(null_widget);
                    }
                }
            }
        }
    }
    if ( !(simul->sink_placement_algorithms.size() > 0 && simul->node_placement_algorithms.size() > 0) ) {
        std::cerr << "There is no loadable placement plugin. Exit.\n";
        std::exit(1);
    }

    pluginsDir.cd("../mobility");
    const QStringList entries2 = pluginsDir.entryList(QDir::Files);
    for (const QString &fileName : entries2) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            iMobility_interface *tmp = qobject_cast<iMobility_interface *>(plugin);
            if (tmp) {
//                for sinks
                if ( tmp->Is_for_sinks() ) {
                    tmp->Get_client_params(&simul->sink_positions, &simul->no_sinks, &simul->node_positions, &simul->no_nodes,
                                           &event_space->sink_draw_real_radius,
                                           &event_space->node_draw_real_radius, &simul->aoi_m_width, &simul->aoi_m_height, true);
                    sink_mobility_combo->addItem(tmp->Alg_name());
                    simul->sink_mobility_algorithms.push_back(tmp);
                    if ( auto gp = tmp->Get_parameters(); gp != nullptr ) {
                        int alg_index = simul->sink_mobility_algorithms.size() - 1;
                        auto spps_widget = new QWidget();
                        std::vector<QWidget*> pass_widgets;
                        auto spps_layout = new QVBoxLayout();
                        spps_layout->setContentsMargins(11, 0, 0, 0);
                        spps_layout->addWidget(new QLabel("Its parameters:"));
                        for ( int i = 0; i < gp->size(); ++i ) {
                            auto& info = gp->at(i);
                            auto par_label = new QLabel(info.name);
                            par_label->setAlignment(Qt::AlignCenter);
                            spps_layout->addWidget(par_label);
                            pass_widgets.push_back(par_label);
                            if ( info.is_categorical ) {
                                auto coc = new QComboBox();
                                for ( int j = 0; j < info.categorical_values->size(); ++j ) {
                                    coc->addItem(info.categorical_values->at(j));
                                }
                                coc->setCurrentIndex(info.def_categorical_value_index);
                                spps_layout->addWidget(coc);
                                pass_widgets.push_back(coc);
                                QObject::connect(coc, &QComboBox::currentIndexChanged, this, [this, info, alg_index](int idx){
                                    bool ir = false;
                                    if ( simul->is_running ) {
                                        simul->Play_pause();
                                        ir = true;
                                    }
                                    simul->sink_mobility_algorithms.at(alg_index)->Set_parameter(info.id, idx);
                                    if ( ir ) {
                                        simul->Play_pause();
                                    }
                                });
                            }else if( info.is_integer ) {
                                auto slc = new cSlider(info.range_start, info.range_end, info.def_number_value);
                                spps_layout->addWidget(slc);
                                pass_widgets.push_back(slc);
                                QObject::connect(slc, &QSlider::valueChanged, this, [this, info, alg_index](int val){
                                    bool ir = false;
                                    if ( simul->is_running ) {
                                        simul->Play_pause();
                                        ir = true;
                                    }
                                    simul->sink_mobility_algorithms.at(alg_index)->Set_parameter(info.id, val);
                                    if ( ir ) {
                                        simul->Play_pause();
                                    }
                                });
                            }else {
                                auto slc = new cFp_slider(info.range_start, info.range_end, info.def_number_value);
                                spps_layout->addWidget(slc);
                                pass_widgets.push_back(slc);
                                QObject::connect(slc, &cFp_slider::Fp_value_changed, this, [this, info, alg_index](double val){
                                    bool ir = false;
                                    if ( simul->is_running ) {
                                        simul->Play_pause();
                                        ir = true;
                                    }
                                    simul->sink_mobility_algorithms.at(alg_index)->Set_parameter(info.id, val);
                                    if ( ir ) {
                                        simul->Play_pause();
                                    }
                                });
                            }
                        }
                        tmp->Get_widgets(pass_widgets);
                        spps_widget->setLayout(spps_layout);
                        sink_mobility_param_stack->addWidget(spps_widget);
                    }else {
                        auto null_widget = new cZero_widget();
                        sink_mobility_param_stack->addWidget(null_widget);
                    }
                }

      //          for nodes
                tmp = tmp->Create_new_instance();
                if ( tmp->Is_for_nodes() ) {
                    tmp->Get_client_params(&simul->node_positions, &simul->no_nodes, &simul->sink_positions, &simul->no_sinks,
                                           &event_space->sink_draw_real_radius,
                                           &event_space->node_draw_real_radius, &simul->aoi_m_width, &simul->aoi_m_height, false);
                    node_mobility_combo->addItem(tmp->Alg_name());
                    simul->node_mobility_algorithms.push_back(tmp);
                    if ( auto gp = tmp->Get_parameters(); gp != nullptr ) {
                        int alg_index = simul->node_mobility_algorithms.size() - 1;
                        auto spps_widget = new QWidget();
                        std::vector<QWidget*> pass_widgets;
                        auto spps_layout = new QVBoxLayout();
                        spps_layout->setContentsMargins(11, 0, 0, 0);
                        spps_layout->addWidget(new QLabel("Its parameters:"));
                        for ( int i = 0; i < gp->size(); ++i ) {
                            auto& info = gp->at(i);
                            auto par_label = new QLabel(info.name);
                            par_label->setAlignment(Qt::AlignCenter);
                            spps_layout->addWidget(par_label);
                            pass_widgets.push_back(par_label);
                            if ( info.is_categorical ) {
                                auto coc = new QComboBox();
                                for ( int j = 0; j < info.categorical_values->size(); ++j ) {
                                    coc->addItem(info.categorical_values->at(j));
                                }
                                coc->setCurrentIndex(info.def_categorical_value_index);
                                spps_layout->addWidget(coc);
                                pass_widgets.push_back(coc);
                                QObject::connect(coc, &QComboBox::currentIndexChanged, this, [this, info, alg_index](int idx){
                                    bool ir = false;
                                    if ( simul->is_running ) {
                                        simul->Play_pause();
                                        ir = true;
                                    }
                                    simul->node_mobility_algorithms.at(alg_index)->Set_parameter(info.id, idx);
                                    if ( ir ) {
                                        simul->Play_pause();
                                    }
                                });
                            }else if( info.is_integer ) {
                                auto slc = new cSlider(info.range_start, info.range_end, info.def_number_value);
                                spps_layout->addWidget(slc);
                                pass_widgets.push_back(slc);
                                QObject::connect(slc, &QSlider::valueChanged, this, [this, info, alg_index](int val){
                                    bool ir = false;
                                    if ( simul->is_running ) {
                                        simul->Play_pause();
                                        ir = true;
                                    }
                                    simul->node_mobility_algorithms.at(alg_index)->Set_parameter(info.id, val);
                                    if ( ir ) {
                                        simul->Play_pause();
                                    }
                                });
                            }else {
                                auto slc = new cFp_slider(info.range_start, info.range_end, info.def_number_value);
                                spps_layout->addWidget(slc);
                                pass_widgets.push_back(slc);
                                QObject::connect(slc, &cFp_slider::Fp_value_changed, this, [this, info, alg_index](double val){
                                    bool ir = false;
                                    if ( simul->is_running ) {
                                        simul->Play_pause();
                                        ir = true;
                                    }
                                    simul->node_mobility_algorithms.at(alg_index)->Set_parameter(info.id, val);
                                    if ( ir ) {
                                        simul->Play_pause();
                                    }
                                });
                            }
                        }
                        tmp->Get_widgets(pass_widgets);
                        spps_widget->setLayout(spps_layout);
                        node_mobility_param_stack->addWidget(spps_widget);
                    }else {
                        auto null_widget = new cZero_widget();
                        node_mobility_param_stack->addWidget(null_widget);
                    }
                }
            }
        }
    }
}

Widget::~Widget()
{
    delete simul;
}

