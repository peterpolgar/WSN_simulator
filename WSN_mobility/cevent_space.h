#ifndef CEVENT_SPACE_H
#define CEVENT_SPACE_H

#include <QWidget>
#include <QtWidgets/QLineEdit>
#include <QGridLayout>

class cSim_run;

class cEvent_space : public QWidget
{
    Q_OBJECT
public:
    explicit cEvent_space(QWidget *parent = nullptr);
    QSize sizeHint() const;
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

    void Set_draw_size(double width_per_height);
    void Change_circle_draw_radius();
    void Set_circle_draw_radius_additional();
    void Set_sim_run(cSim_run *sim_run);

    int draw_width;
    int draw_height;
    int start_x;
    int start_y;
    int frame_width;
    double width_per_height;
    int node_draw_radius;
    int node_draw_real_radius;
    int sink_draw_real_radius;
    double circle_draw_radius_additional;
    bool is_first_run;

    int dragged_item_index;
    bool is_dragged_sink;
    bool is_left_clicked;
    int mouse_x;
    int mouse_y;

    cSim_run *sim_run;
    QWidget* popup_widget;
    QLineEdit* x_lineedit;
    QLineEdit* y_lineedit;
};

#endif // CEVENT_SPACE_H
