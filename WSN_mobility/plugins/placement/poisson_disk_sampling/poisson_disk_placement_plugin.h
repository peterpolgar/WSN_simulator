#ifndef POISSON_DISK_PLACEMENT_PLUGIN_H
#define POISSON_DISK_PLACEMENT_PLUGIN_H

#include <QObject>
#include <QtPlugin>
#include "placement_interface.h"

class cPoisson_disk_placement : public QObject, iPlacement_interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "placement.interface" FILE "poisson.json")
    Q_INTERFACES(iPlacement_interface)

public:
    cPoisson_disk_placement();
    QString Alg_name() override;
    bool Is_for_sinks() override;
    bool Is_for_nodes() override;
    void Compute_initial_position(std::vector<double> *pos_vec, int number_of_items, int width, int height) override;
    void Set_parameter(std::string param_str, std::string ertek_str) override;
    std::vector<sParam>* Get_parameters() override;
    void Set_parameter(int param_id, double value_or_index) override;
    iPlacement_interface* Create_new_instance() override;
    void Get_widgets(std::vector<QWidget*> widgets) override;

    int seed;
};

#endif // POISSON_DISK_PLACEMENT_PLUGIN_H
