#ifndef WIDGET_H
#define WIDGET_H

#include "cevent_space.h"
#include "cslider.h"
#include "cfp_slider.h"
#include "csim_run.h"
#include "cstacked_widget.h"
#include <QWidget>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
//#include <QtWidgets/QSpacerItem>

class Widget : public QWidget
{
    Q_OBJECT

    class cZero_widget : public QWidget
    {
    public:
        cZero_widget() : QWidget()
        {
            resize(0, 0);
        }

        QSize sizeHint() const
        {
            return QSize(0, 0);
        }

        QSize minimumSizeHint() const
        {
            return QSize(0, 0);
        }
    };

    class cMy_scroll_area : public QScrollArea {
    public:
        void wheelEvent(QWheelEvent *event) override {
            QScrollArea::wheelEvent(event);
        }
    };

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    QHBoxLayout* main_layout;
    cEvent_space* event_space;
    cMy_scroll_area* par_list_scroll;
    QVBoxLayout* par_list_layout;
    QHBoxLayout* play_layout;
    QPushButton* play_button;
    QIcon* play_icon;
    QPushButton* stop_button;
    QIcon* pause_icon;
    cFp_slider* simul_speed_slider;
    QLabel* aoi_head_label;
    QHBoxLayout* aoi_wh_layout;
    QLabel* aoi_width_label;
    QLabel* aoi_height_label;
    QLabel* aoi_unit_len_label;
    cSlider* aoi_unit_len_slider;
    QLabel* aoi_unit_width_label;
    cSlider* aoi_unit_width_slider;
    QLabel* aoi_unit_height_label;
    cSlider* aoi_unit_height_slider;
    QLabel* sinks_head_label;
    QLabel* no_sinks_label;
    cSlider* no_sinks_slider;
    QLabel* sink_placement_label;
    QComboBox* sink_placement_combo;
    cStacked_widget* sink_placement_param_stack;
    QLabel* sink_mobility_label;
    QComboBox* sink_mobility_combo;
    cStacked_widget* sink_mobility_param_stack;
    QLabel* node_head_label;
    QLabel* no_nodes_label;
    cSlider* no_nodes_slider;
    QLabel* node_placement_label;
    QComboBox* node_placement_combo;
    cStacked_widget* node_placement_param_stack;
    QLabel* node_mobility_label;
    QComboBox* node_mobility_combo;
    cStacked_widget* node_mobility_param_stack;
    cSim_run* simul;
};
#endif // WIDGET_H
