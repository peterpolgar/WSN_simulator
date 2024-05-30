#ifndef MOBILITY_INTERFACE_H
#define MOBILITY_INTERFACE_H

#include <QObject>
#include <QString>
#include <QWidget>

class iMobility_interface
{
public:
    struct sParam
    {
        int id;
        QString name;
        bool is_categorical;
        bool is_integer;
        double range_start;
        double range_end;
        double def_number_value;
        std::vector<QString> *categorical_values;
        int def_categorical_value_index;
    };

    virtual ~iMobility_interface() = default;
    virtual QString Alg_name() = 0;
    virtual bool Is_for_sinks() = 0;
    virtual bool Is_for_nodes() = 0;
    virtual void Get_client_params(std::vector<double> *set_pos_vec, int *number_of_set_items, const std::vector<double> *get_pos_vec,
                                   int *number_of_get_items, int *sink_radius, int *node_radius, int *meter_width, int *meter_height, bool is_for_sinks_not_nodes) = 0;
    virtual void Compute_next_position(int time_in_msec, const std::vector<double> *get_tmp_pos_vec = {}) = 0;
    virtual void Set_parameter(std::string param_str, std::string ertek_str) = 0;
    virtual std::vector<sParam>* Get_parameters() = 0;
    virtual void Set_parameter(int param_id, double value_or_index) = 0;
    virtual iMobility_interface* Create_new_instance() = 0;
    virtual QString Get_params_string() = 0;
    virtual void Init_run() = 0;
    virtual void Unit_len_changed() = 0;
    virtual void Resize_happened(double new_old_rate) = 0;
    virtual void Get_widgets(std::vector<QWidget*> widgets) = 0;

    std::vector<double> *set_pos_vec;
    int *number_of_set_items;
    const std::vector<double> *get_pos_vec;
    int *number_of_get_items;
    int *area_m_width;
    int *area_m_height;
    int *sink_radius;
    int *node_radius;
    bool is_for_sinks_not_nodes;
    double col_avoid_radius_basic;
    enum eCollision_avoidance { none, opposite_dir };
    eCollision_avoidance collision_avoidance;
    std::vector<QWidget*> widgets;
    double distance_from_aoi_edges_m = 0.;
};


QT_BEGIN_NAMESPACE

#define iMobility_interface_iid "mobility.interface"

Q_DECLARE_INTERFACE(iMobility_interface, iMobility_interface_iid)
QT_END_NAMESPACE

#endif // MOBILITY_INTERFACE_H
