#ifndef PLACEMENT_INTERFACE_H
#define PLACEMENT_INTERFACE_H

#include <QObject>
#include <QString>
#include <QWidget>

class iPlacement_interface
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

    virtual ~iPlacement_interface() = default;
    virtual QString Alg_name() = 0;
    virtual bool Is_for_sinks() = 0;
    virtual bool Is_for_nodes() = 0;
    virtual void Compute_initial_position(std::vector<double> *pos_vec, int number_of_items, int width, int height) = 0;
    virtual void Set_parameter(std::string param_str, std::string ertek_str) = 0;
    virtual std::vector<sParam>* Get_parameters() = 0;
    virtual void Set_parameter(int param_id, double value_or_index) = 0;
    virtual iPlacement_interface* Create_new_instance() = 0;
    virtual void Get_widgets(std::vector<QWidget*> widgets) = 0;

    std::vector<QWidget*> widgets;
};


QT_BEGIN_NAMESPACE

#define iPlacement_interface_iid "placement.interface"

Q_DECLARE_INTERFACE(iPlacement_interface, iPlacement_interface_iid)
QT_END_NAMESPACE

#endif // PLACEMENT_INTERFACE_H
