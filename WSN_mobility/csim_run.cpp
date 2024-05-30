#include "csim_run.h"
#include "cevent_space.h"
#include <QClipboard>
#include <QGuiApplication>
#include <QtConcurrent>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>

cSim_run::cSim_run(cEvent_space *event_space, QPushButton *play_button, QLabel *aoi_width_label, QLabel *aoi_height_label, QLabel *aoi_unit_len_label,
                   QLabel *aoi_unit_width_label, QLabel *aoi_unit_height_label, QLabel *no_sinks_label, QLabel *no_nodes_label, QLineEdit *lineedit_of_time,
                   cFp_slider *simul_speed_slider, cFp_slider *comm_range_slider, cSlider *aoi_unit_len_slider, cSlider *aoi_unit_width_slider,
                   cSlider *aoi_unit_height_slider, cSlider *sink_size_slider, cSlider *no_sinks_slider, cSlider *node_size_slider, cSlider *no_nodes_slider,
                   QComboBox *sink_plac_combo, QComboBox *node_plac_combo, QComboBox *sink_mobi_combo, QComboBox *node_mobi_combo, QLabel *loaded_filename_label)
    : QObject()
{
    is_running = false;
    is_first_play = true;
    is_mobi_initialized = false;

    this->event_space = event_space;
    this->play_button = play_button;

    aoi_unit_length = 10;
    aoi_unit_width = 16;
    aoi_unit_height = 9;
    aoi_m_width = aoi_unit_length * aoi_unit_width;
    aoi_m_height = aoi_unit_length * aoi_unit_height;

    no_sinks = 5;
    sink_placement_algorithm_index = 0;
    sink_mobility_algorithm_index = -1;

    no_nodes = 30;
    node_placement_algorithm_index = 0;
    node_mobility_algorithm_index = -1;

//    labels
    this->aoi_width_label = aoi_width_label;
    this->aoi_height_label = aoi_height_label;
    this->aoi_unit_len_label = aoi_unit_len_label;
    this->aoi_unit_width_label = aoi_unit_width_label;
    this->aoi_unit_height_label = aoi_unit_height_label;
    this->no_sinks_label = no_sinks_label;
    this->no_nodes_label = no_nodes_label;
    this->loaded_filename_label = loaded_filename_label;

//    sliders
    this->simul_speed_slider = simul_speed_slider;
    this->comm_range_slider = comm_range_slider;
    this->aoi_unit_len_slider = aoi_unit_len_slider;
    this->aoi_unit_width_slider = aoi_unit_width_slider;
    this->aoi_unit_height_slider = aoi_unit_height_slider;
    this->sink_size_slider = sink_size_slider;
    this->no_sinks_slider = no_sinks_slider;
    this->node_size_slider = node_size_slider;
    this->no_nodes_slider = no_nodes_slider;

//    comboboxes
    this->sink_plac_combo = sink_plac_combo;
    this->node_plac_combo = node_plac_combo;
    this->sink_mobi_combo = sink_mobi_combo;
    this->node_mobi_combo = node_mobi_combo;

//    icons
    play_icon.addFile(QString::fromUtf8(":/images/icons8-play-50.png"), QSize(), QIcon::Normal, QIcon::Off);
    pause_icon.addFile(QString::fromUtf8(":/images/icons8-pause-50.png"), QSize(), QIcon::Normal, QIcon::Off);

//    sizes
    this->sink_meter_size = 0.2;
    this->node_meter_size = 0.1;

//    simulation speed
    simul_speed = 1;

//    communication range squared
    comm_range_squared = 255.852 * 255.852;

//    times
    global_time_ms = 0;
    step_time_ms = 33;
    this->lineedit_of_time = lineedit_of_time;
}

void cSim_run::Do_placement(bool is_sink, bool is_node)
{
    if ( sink_placement_algorithms.size() > 0 && node_placement_algorithms.size() > 0 ) {
        if ( is_sink ) {
            sink_placement_algorithms.at(sink_placement_algorithm_index)->Compute_initial_position(&sink_positions, no_sinks, aoi_m_width, aoi_m_height);
        }
        if ( is_node ) {
            node_placement_algorithms.at(node_placement_algorithm_index)->Compute_initial_position(&node_positions, no_nodes, aoi_m_width, aoi_m_height);
        }
    }
    event_space->update();
    global_time_ms = 0;
    lineedit_of_time->setText("0");
}

void cSim_run::Play_pause()
{
    if ( is_running ) {
        future.suspend();
        is_running = false;
        play_button->setIcon(play_icon);
    }else {
        if ( is_first_play ) {
            is_first_play = false;
            if ( ! is_mobi_initialized ) {
                is_mobi_initialized = true;
                if ( sink_mobility_algorithm_index > -1 ) {
                    sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Init_run();
                }
                if ( node_mobility_algorithm_index > -1 ) {
                    node_mobility_algorithms.at(node_mobility_algorithm_index)->Init_run();
                }
            }
            future = QtConcurrent::run(&cSim_run::Start_mobility_thread, this);
        }else {
            future.resume();
        }
        is_running = true;
        play_button->setIcon(pause_icon);
    }
}

void cSim_run::Stop(bool clicked)
{
    if ( ! is_first_play ) {
        future.cancel();
    }
    if ( is_running ) {
        is_running = false;
        play_button->setIcon(play_icon);
    }
    is_first_play = true;
    is_mobi_initialized = false;
    if ( clicked ) {
        Do_placement();
    }
}

void cSim_run::Start_mobility_thread(QPromise<void> &promise)
{
    const qint64 round_time = 33;
    auto target_msec = QDateTime::currentMSecsSinceEpoch() + round_time;
    while ( true ) {
        promise.suspendIfRequested();
        if (promise.isCanceled()){
            return;
        }
        if ( auto cur = QDateTime::currentMSecsSinceEpoch(); cur > target_msec ) {
            target_msec = cur + round_time;
            auto tmp_sipo = sink_positions;
            auto move_step_time = std::round(round_time * simul_speed);
            if ( sink_mobility_algorithm_index > -1 ) {
                sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Compute_next_position(move_step_time);
            }
            if ( node_mobility_algorithm_index > -1 ) {
                node_mobility_algorithms.at(node_mobility_algorithm_index)->Compute_next_position(move_step_time, &tmp_sipo);
            }
            event_space->update();
            global_time_ms += move_step_time;
            lineedit_of_time->setText(std::to_string(global_time_ms).c_str());
        }
    }
}

void cSim_run::AoI_unit_length(int value)
{
    aoi_unit_length = value;
    aoi_unit_len_label->setText(("AoI one unit length [m]: " + std::to_string(value)).c_str());
    aoi_m_width = aoi_unit_length * aoi_unit_width;
    aoi_width_label->setText(("Width [m]: " + std::to_string(aoi_m_width)).c_str());
    aoi_m_height = aoi_unit_length * aoi_unit_height;
    aoi_height_label->setText(("Height [m]: " + std::to_string(aoi_m_height)).c_str());
    event_space->sink_draw_real_radius = std::round((sink_meter_size / aoi_m_width) * event_space->draw_width);
    event_space->node_draw_real_radius = std::round((node_meter_size / aoi_m_width) * event_space->draw_width);
    if ( sink_mobility_algorithm_index > -1 ) {
        sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Unit_len_changed();
    }
    if ( node_mobility_algorithm_index > -1 ) {
        node_mobility_algorithms.at(node_mobility_algorithm_index)->Unit_len_changed();
    }
    if ( ! is_first_play ) {
        Stop();
    }
    Do_placement();
}

void cSim_run::AoI_unit_width(int value)
{
    aoi_unit_width = value;
    aoi_unit_width_label->setText(("Width of AoI in units: " + std::to_string(value)).c_str());
    aoi_m_width = aoi_unit_length * aoi_unit_width;
    aoi_width_label->setText(("Width [m]: " + std::to_string(aoi_m_width)).c_str());
    event_space->Set_draw_size((double)aoi_unit_width / aoi_unit_height);
    if ( ! is_first_play ) {
        Stop();
    }
    Do_placement();
}

void cSim_run::AoI_unit_height(int value)
{
    aoi_unit_height = value;
    aoi_unit_height_label->setText(("Height of AoI in units: " + std::to_string(value)).c_str());
    aoi_m_height = aoi_unit_length * aoi_unit_height;
    aoi_height_label->setText(("Height [m]: " + std::to_string(aoi_m_height)).c_str());
    event_space->Set_draw_size((double)aoi_unit_width / aoi_unit_height);
    if ( ! is_first_play ) {
        Stop();
    }
    Do_placement();
}

void cSim_run::No_sinks(int value)
{
    no_sinks = value;
    no_sinks_label->setText(("Number of sinks: " + std::to_string(value)).c_str());
    if ( ! is_first_play ) {
        Stop();
    }
    Do_placement(true, false);
}

void cSim_run::Sink_placement_algorithm(int index)
{
    sink_placement_algorithm_index = index;
    if ( ! is_first_play ) {
        Stop();
    }
    Do_placement(true, false);
}

void cSim_run::Sink_mobility_algorithm(int index)
{
    bool ir = false;
    if ( is_running ) {
        Play_pause();
        ir = true;
    }
    sink_mobility_algorithm_index = index - 1;
    if ( ! is_first_play && sink_mobility_algorithm_index > -1 ) {
        sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Init_run();
    }
    if ( ir ) {
        Play_pause();
    }
}

void cSim_run::No_nodes(int value)
{
    no_nodes = value;
    no_nodes_label->setText(("Number of nodes: " + std::to_string(value)).c_str());
    if ( ! is_first_play ) {
        Stop();
    }
    event_space->Set_circle_draw_radius_additional();
    event_space->Change_circle_draw_radius();
    Do_placement(false, true);
}

void cSim_run::Node_placement_algorithm(int index)
{
    node_placement_algorithm_index = index;
    if ( ! is_first_play ) {
        Stop();
    }
    Do_placement(false, true);
}

void cSim_run::Node_mobility_algorithm(int index)
{
    bool ir = false;
    if ( is_running ) {
        Play_pause();
        ir = true;
    }
    node_mobility_algorithm_index = index - 1;
    if ( ! is_first_play && node_mobility_algorithm_index > -1 ) {
        node_mobility_algorithms.at(node_mobility_algorithm_index)->Init_run();
    }
    if ( ir ) {
        Play_pause();
    }
}

void cSim_run::Sink_meter_size(int value)
{
    sink_meter_size = (double)value / 200;
    event_space->sink_draw_real_radius = std::round((sink_meter_size / aoi_m_width) * event_space->draw_width);
    event_space->update();
}

void cSim_run::Node_meter_size(int value)
{
    node_meter_size = (double)value / 200;
    event_space->node_draw_real_radius = std::round((node_meter_size / aoi_m_width) * event_space->draw_width);
    event_space->update();
}

void cSim_run::Simul_speed(double value)
{
    simul_speed = value;
}

void cSim_run::Comm_range(double value) {
    comm_range_squared = value * value;
    event_space->update();
}

void cSim_run::Copy_params_to_clipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString param_text("");

    param_text += "aoi_meter_width,";
    param_text += std::to_string(aoi_m_width);
    param_text += "\n";

    param_text += "aoi_meter_height,";
    param_text += std::to_string(aoi_m_height);
    param_text += "\n";

    param_text += "number_of_sinks,";
    param_text += std::to_string(no_sinks);
    param_text += "\n";

    param_text += "sink_meter_diam,";
    param_text += std::to_string(sink_meter_size);
    param_text += "\n";

    param_text += "positions_of_sinks,";
    for ( int i = 0; i < no_sinks; ++i ) {
        param_text += std::to_string(sink_positions.at(i * 2));
        param_text += ",";
        param_text += std::to_string(sink_positions.at(i * 2 + 1));
        if ( i < no_sinks - 1 ) {
            param_text += ",";
        }
    }
    param_text += "\n";

    param_text += "number_of_nodes,";
    param_text += std::to_string(no_nodes);
    param_text += "\n";

    param_text += "node_meter_diam,";
    param_text += std::to_string(node_meter_size);
    param_text += "\n";

    param_text += "positions_of_nodes,";
    for ( int i = 0; i < no_nodes; ++i ) {
        param_text += std::to_string(node_positions.at(i * 2));
        param_text += ",";
        param_text += std::to_string(node_positions.at(i * 2 + 1));
        if ( i < no_nodes - 1 ) {
            param_text += ",";
        }
    }

    if ( sink_mobility_algorithm_index != -1 ) {
        param_text += "\nsink_mobility_alg,";
        param_text += sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Alg_name();
        param_text += "\n";
        param_text += sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Get_params_string();
    }

    if ( node_mobility_algorithm_index != -1 ) {
        param_text += "\nnode_mobility_alg,";
        param_text += node_mobility_algorithms.at(node_mobility_algorithm_index)->Alg_name();
        param_text += "\n";
        param_text += node_mobility_algorithms.at(node_mobility_algorithm_index)->Get_params_string();
    }

    clipboard->setText(param_text);
}

void cSim_run::Import_config_file() {
    QString fileName = QFileDialog::getOpenFileName(event_space, tr("Import Configuration File"),
                                                    ".",
                                                    tr("Configuration files (*.conf *.txt)"));
    std::string input_file_name = fileName.toStdString();
    if ( input_file_name.size() > 0 ) {
        bool successful_read = false;
        char *conf_str = new char[20000];
        std::iostream::pos_type conf_size;
        if (std::ifstream is{input_file_name, std::ios::in | std::ios::ate}) {
            conf_size = is.tellg();
            conf_str[conf_size] = '\0';
            is.seekg(0);
            if (! is.read(&conf_str[0], conf_size)) {
                QMessageBox msgBox;
                msgBox.setText("Error while reading the given configuration file.");
                msgBox.exec();
            }else {
                successful_read = true;
            }
        }else {
            QMessageBox msgBox;
            msgBox.setText("Error while opening the given configuration file.");
            msgBox.exec();
        }
        if ( successful_read ) {
            {
                auto position_of_last_separator = input_file_name.rfind('/');
                if ( position_of_last_separator == std::string::npos ) {
                    position_of_last_separator = input_file_name.rfind('\\');
                }
                loaded_filename_label->setText(("<b>Loaded file: </b>" + input_file_name.substr(position_of_last_separator + 1)).c_str());
            }
            if ( ! is_first_play ) {
                Stop();
            }
//            bool is_there_sink_mobility = false;
//            bool is_there_node_mobility = false;
            std::vector<std::string> sink_mobi_para_name_str_vec;
            std::vector<std::string> sink_mobi_para_value_str_vec;
            std::vector<std::string> node_mobi_para_name_str_vec;
            std::vector<std::string> node_mobi_para_value_str_vec;
//            bool is_there_sink_placement = false;
//            bool is_there_node_placement = false;
            std::vector<std::string> sink_plac_para_name_str_vec;
            std::vector<std::string> sink_plac_para_value_str_vec;
            std::vector<std::string> node_plac_para_name_str_vec;
            std::vector<std::string> node_plac_para_value_str_vec;
            std::string sink_plac_alg_name{ "" };
            std::string node_plac_alg_name{ "" };
            std::string sink_mobi_alg_name{ "" };
            std::string node_mobi_alg_name{ "" };
            int algo_id = 0;
            bool position_given = false;

            char buffer[60], para_name[60];
            bool was_comma = false;
            int start_pos = 0;
            std::vector<std::string> vv;
            for ( int i = 0; i < conf_size; ++i ) {
                if ( conf_str[i] == '\n' ) {
                    if ( was_comma ) {
                        strncpy(buffer, conf_str + start_pos, i - start_pos);
                        buffer[i - start_pos] = '\0';
                        vv.push_back(std::string{ buffer });
                        // std::cout << vv[0] << '\n';
                        if ( strcmp(para_name, "simulation_speed [x]") == 0 ) {
                            simul_speed_slider->Set_fp_value(stod(vv[0]));
                        }else if ( strcmp(para_name, "communication_range [m]") == 0 ) {
                            comm_range_slider->Set_fp_value(stod(vv[0]));
                        }else if ( strcmp(para_name, "aoi_meter_width") == 0 ) {
                            int tdv = stod(vv[0]);
                            if ( tdv <= 100 ) {
                                aoi_unit_width_slider->setValue(tdv);
                                double mod_h = aoi_unit_len_slider->value() * aoi_unit_height_slider->value();
                                if ( mod_h > 100. ) {
                                    aoi_unit_height_slider->setValue(100);
                                }else {
                                    aoi_unit_height_slider->setValue(std::round(mod_h));
                                }
                                aoi_unit_len_slider->setValue(1);
                            }else {
                                int tiso = 1;
                                while ( tdv > 100 ) {
                                    tdv /= 10;
                                    tiso *= 10;
                                }
                                aoi_unit_width_slider->setValue(tdv);
                                double mod_h = ((double)(aoi_unit_len_slider->value()) / tiso) * aoi_unit_height_slider->value();
                                if ( mod_h < 1. ) {
                                    aoi_unit_height_slider->setValue(1);
                                }else if ( mod_h > 100. ) {
                                    aoi_unit_height_slider->setValue(100);
                                }else {
                                    aoi_unit_height_slider->setValue(std::round(mod_h));
                                }
                                aoi_unit_len_slider->setValue(tiso);
                            }
                        }else if ( strcmp(para_name, "aoi_meter_height") == 0 ) {
                            int tdv = stod(vv[0]);
                            if ( tdv <= 100 ) {
                                aoi_unit_height_slider->setValue(tdv);
                                double mod_h = aoi_unit_len_slider->value() * aoi_unit_width_slider->value();
                                if ( mod_h > 100. ) {
                                    aoi_unit_width_slider->setValue(100);
                                }else {
                                    aoi_unit_width_slider->setValue(std::round(mod_h));
                                }
                                aoi_unit_len_slider->setValue(1);
                            }else {
                                int tiso = 1;
                                while ( tdv > 100 ) {
                                    tdv /= 10;
                                    tiso *= 10;
                                }
                                aoi_unit_height_slider->setValue(tdv);
                                double mod_h = ((double)(aoi_unit_len_slider->value()) / tiso) * aoi_unit_width_slider->value();
                                if ( mod_h < 1. ) {
                                    aoi_unit_width_slider->setValue(1);
                                }else if ( mod_h > 100. ) {
                                    aoi_unit_width_slider->setValue(100);
                                }else {
                                    aoi_unit_width_slider->setValue(std::round(mod_h));
                                }
                                aoi_unit_len_slider->setValue(tiso);
                            }
                        }else if ( strcmp(para_name, "number_of_sinks") == 0 ) {
                            no_sinks_slider->setValue(stod(vv[0]));
                        }else if ( strcmp(para_name, "sink_meter_diam") == 0 ) {
                            sink_size_slider->setValue(stod(vv[0]));
                        }else if ( strcmp(para_name, "positions_of_sinks") == 0 ) {
                            position_given = true;
                            if ( vv.size() / 2 != no_sinks ) {
                                no_sinks_slider->setValue(vv.size() / 2);
                            }
                            sink_positions.clear();
                            for ( unsigned long j = 0; j < vv.size(); ++j ) {
                                sink_positions.push_back(stod(vv[j]));
                            }
                            global_time_ms = 0;
                            lineedit_of_time->setText("0");
                        }else if ( strcmp(para_name, "number_of_nodes") == 0 ) {
                            no_nodes_slider->setValue(stod(vv[0]));
                        }else if ( strcmp(para_name, "node_meter_diam") == 0 ) {
                            node_size_slider->setValue(stod(vv[0]));
                        }else if ( strcmp(para_name, "positions_of_nodes") == 0 ) {
                            position_given = true;
                            if ( vv.size() / 2 != no_nodes ) {
                                no_nodes_slider->setValue(vv.size() / 2);
                            }
                            node_positions.clear();
                            for ( unsigned long j = 0; j < vv.size(); ++j ) {
                                node_positions.push_back(stod(vv[j]));
                            }
                            global_time_ms = 0;
                            lineedit_of_time->setText("0");
                        }else if ( strcmp(para_name, "sink_placement_alg") == 0 ) {
                            algo_id = 0;
                            sink_plac_alg_name = vv[0];
                        }else if ( strcmp(para_name, "node_placement_alg") == 0 ) {
                            algo_id = 1;
                            node_plac_alg_name = vv[0];
                        }else if ( strcmp(para_name, "sink_mobility_alg") == 0 ) {
                            algo_id = 2;
                            sink_mobi_alg_name = vv[0];
                        }else if ( strcmp(para_name, "node_mobility_alg") == 0 ) {
                            algo_id = 3;
                            node_mobi_alg_name = vv[0];
                        }else {
                            if ( algo_id == 0 ) {
                                sink_plac_para_name_str_vec.push_back(para_name);
                                sink_plac_para_value_str_vec.push_back(vv[0]);
                            }else if ( algo_id == 1 ) {
                                node_plac_para_name_str_vec.push_back(para_name);
                                node_plac_para_value_str_vec.push_back(vv[0]);
                            }else if ( algo_id == 2 ) {
                                sink_mobi_para_name_str_vec.push_back(para_name);
                                sink_mobi_para_value_str_vec.push_back(vv[0]);
                            }else {
                                node_mobi_para_name_str_vec.push_back(para_name);
                                node_mobi_para_value_str_vec.push_back(vv[0]);
                            }
                        }
                        vv.clear();
                        was_comma = false;
                    }
                    start_pos = i + 1;
                }else if ( conf_str[i] == ',' ) {
                    if ( was_comma ) {
                        strncpy(buffer, conf_str + start_pos, i - start_pos);
                        buffer[i - start_pos] = '\0';
                        vv.push_back(std::string{ buffer });
                    }else {
                        strncpy(para_name, conf_str + start_pos, i - start_pos);
                        para_name[i - start_pos] = '\0';
                    }
                    was_comma = true;
                    start_pos = i + 1;
                }
            }
            if ( sink_plac_alg_name != "" ) {
                for ( unsigned long i = 0; i < sink_placement_algorithms.size(); ++i ) {
                    if ( sink_plac_alg_name == sink_placement_algorithms[i]->Alg_name().toStdString() ) {
                        for ( unsigned long j = 0; j < sink_plac_para_name_str_vec.size(); ++j ) {
                            sink_placement_algorithms[i]->Set_parameter(sink_plac_para_name_str_vec[j], sink_plac_para_value_str_vec[j]);
                        }
                        sink_plac_combo->setCurrentIndex(i);
                        break;
                    }
                }
            }
            if ( node_plac_alg_name != "" ) {
                for ( unsigned long i = 0; i < node_placement_algorithms.size(); ++i ) {
                    if ( node_plac_alg_name == node_placement_algorithms[i]->Alg_name().toStdString() ) {
                        for ( unsigned long j = 0; j < node_plac_para_name_str_vec.size(); ++j ) {
                            node_placement_algorithms[i]->Set_parameter(node_plac_para_name_str_vec[j], node_plac_para_value_str_vec[j]);
                        }
                        node_plac_combo->setCurrentIndex(i);
                        break;
                    }
                }
            }
            if ( sink_mobi_alg_name != "" ) {
                bool mobi_is_none = true;
                for ( unsigned long i = 0; i < sink_mobility_algorithms.size(); ++i ) {
                    if ( sink_mobi_alg_name == sink_mobility_algorithms[i]->Alg_name().toStdString() ) {
                        mobi_is_none = false;
                        for ( unsigned long j = 0; j < sink_mobi_para_name_str_vec.size(); ++j ) {
                            sink_mobility_algorithms[i]->Set_parameter(sink_mobi_para_name_str_vec[j], sink_mobi_para_value_str_vec[j]);
                        }
                        sink_mobi_combo->setCurrentIndex(i + 1);
                        break;
                    }
                }
                if ( mobi_is_none ) {
                    sink_mobi_combo->setCurrentIndex(0);
                }
                is_mobi_initialized = false;
            }
            if ( node_mobi_alg_name != "" ) {
                bool mobi_is_none = true;
                for ( unsigned long i = 0; i < node_mobility_algorithms.size(); ++i ) {
                    if ( node_mobi_alg_name == node_mobility_algorithms[i]->Alg_name().toStdString() ) {
                        mobi_is_none = false;
                        for ( unsigned long j = 0; j < node_mobi_para_name_str_vec.size(); ++j ) {
                            node_mobility_algorithms[i]->Set_parameter(node_mobi_para_name_str_vec[j], node_mobi_para_value_str_vec[j]);
                        }
                        node_mobi_combo->setCurrentIndex(i + 1);
                        break;
                    }
                }
                if ( mobi_is_none ) {
                    node_mobi_combo->setCurrentIndex(0);
                }
                is_mobi_initialized = false;
            }
            if ( position_given ) {
                event_space->update();
            }
        }
        delete[] conf_str;
    }
}

void cSim_run::Time_edit() {
    unsigned long long prev_time = global_time_ms;
    try
    {
        global_time_ms = std::stoull(lineedit_of_time->text().toStdString());
    }
    catch (std::invalid_argument const& ex)
    {

    }
    long long time_delta = global_time_ms - prev_time;
    if ( time_delta != 0 ) {
        Set_movements_to_the_global_time(time_delta);
    }
}

void cSim_run::Time_step_edit(const QString &text) {
    try
    {
        step_time_ms = std::stoull(text.toStdString());
    }
    catch (std::invalid_argument const& ex)
    {

    }
}

void cSim_run::Add_time_step() {
    unsigned long long prev_time = global_time_ms;
    global_time_ms += step_time_ms;
    lineedit_of_time->setText(std::to_string(global_time_ms).c_str());
    long long time_delta = global_time_ms - prev_time;
    if ( time_delta != 0 ) {
        Set_movements_to_the_global_time(time_delta);
    }
}

void cSim_run::Subtract_time_step() {
    unsigned long long prev_time = global_time_ms;
    global_time_ms -= step_time_ms;
    lineedit_of_time->setText(std::to_string(global_time_ms).c_str());
    long long time_delta = global_time_ms - prev_time;
    if ( time_delta != 0 ) {
        Set_movements_to_the_global_time(time_delta);
    }
}

void cSim_run::Set_movements_to_the_global_time(long long time_delta) {
    bool volt_ini = false;
    if ( ! is_mobi_initialized ) {
        is_mobi_initialized = true;
        volt_ini = true;
        if ( sink_mobility_algorithm_index > -1 ) {
            sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Init_run();
        }
        if ( node_mobility_algorithm_index > -1 ) {
            node_mobility_algorithms.at(node_mobility_algorithm_index)->Init_run();
        }
    }
    if ( time_delta > 0 ) {
        while ( time_delta > 0 ) {
            int mt = 0;
            if ( time_delta >= 100 ) {
                mt = 100;
            }else {
                mt = time_delta;
            }
            auto tmp_sipo = sink_positions;
            if ( sink_mobility_algorithm_index > -1 ) {
                sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Compute_next_position(mt);
            }
            if ( node_mobility_algorithm_index > -1 ) {
                node_mobility_algorithms.at(node_mobility_algorithm_index)->Compute_next_position(mt, &tmp_sipo);
            }
            time_delta -= mt;
        }
    }else {
        if ( sink_placement_algorithms.size() > 0 && node_placement_algorithms.size() > 0 ) {
            sink_placement_algorithms.at(sink_placement_algorithm_index)->Compute_initial_position(&sink_positions, no_sinks, aoi_m_width, aoi_m_height);
            node_placement_algorithms.at(node_placement_algorithm_index)->Compute_initial_position(&node_positions, no_nodes, aoi_m_width, aoi_m_height);
        }
        if ( ! volt_ini ) {
            if ( sink_mobility_algorithm_index > -1 ) {
                sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Init_run();
            }
            if ( node_mobility_algorithm_index > -1 ) {
                node_mobility_algorithms.at(node_mobility_algorithm_index)->Init_run();
            }
        }
        long long xt = global_time_ms;
        while ( xt > 0 ) {
            int mt = 0;
            if ( xt >= 100 ) {
                mt = 100;
            }else {
                mt = xt;
            }
            auto tmp_sipo = sink_positions;
            if ( sink_mobility_algorithm_index > -1 ) {
                sink_mobility_algorithms.at(sink_mobility_algorithm_index)->Compute_next_position(mt);
            }
            if ( node_mobility_algorithm_index > -1 ) {
                node_mobility_algorithms.at(node_mobility_algorithm_index)->Compute_next_position(mt, &tmp_sipo);
            }
            xt -= mt;
        }
    }
    event_space->update();
}

cSim_run::~cSim_run()
{
    if ( ! is_first_play ) {
        future.cancel();
    }
}
