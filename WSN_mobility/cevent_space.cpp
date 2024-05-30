#include "cevent_space.h"
#include "csim_run.h"
#include <QPainter>
#include <QMouseEvent>
#include <cstring>
#include <cstdio>
#include <cmath>

cEvent_space::cEvent_space(QWidget *parent)
    : QWidget{parent}
{
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->is_first_run = true;
    popup_widget = new QWidget(this);
    popup_widget->resize(180, 120);
    popup_widget->setObjectName("popup_widget");
    auto popup_layout = new QGridLayout(popup_widget);
    popup_layout->setContentsMargins(0, 0, 0, 0);
    popup_layout->setSpacing(0);
    popup_layout->addWidget(new QLabel(" X:"), 0, 0, 1, 1);
    x_lineedit = new QLineEdit();
    popup_layout->addWidget(x_lineedit, 0, 1, 1, 1);
    popup_layout->addWidget(new QLabel("m"), 0, 2, 1, 1);
    popup_layout->addWidget(new QLabel(" Y:"), 1, 0, 1, 1);
    y_lineedit = new QLineEdit();
    popup_layout->addWidget(y_lineedit, 1, 1, 1, 1);
    popup_layout->addWidget(new QLabel("m"), 1, 2, 1, 1);
    popup_widget->hide();

    QObject::connect(x_lineedit, &QLineEdit::editingFinished, this, [this]{
        if ( is_dragged_sink ){
            sim_run->sink_positions.at(dragged_item_index * 2) = (double)std::stod(x_lineedit->text().toStdString());
        }else{
            sim_run->node_positions.at(dragged_item_index * 2) = (double)std::stod(x_lineedit->text().toStdString());
        }
        update();
    });

    QObject::connect(y_lineedit, &QLineEdit::editingFinished, this, [this]{
        if ( is_dragged_sink ){
            sim_run->sink_positions.at(dragged_item_index * 2 + 1) = (double)std::stod(y_lineedit->text().toStdString());
        }else{
            sim_run->node_positions.at(dragged_item_index * 2 + 1) = (double)std::stod(y_lineedit->text().toStdString());
        }
        update();
    });

    dragged_item_index = -1;
    is_dragged_sink = false;
    is_left_clicked = false;
}

QSize cEvent_space::sizeHint() const
{
    return QSize(2000, 2000);
}

void cEvent_space::Set_draw_size(double width_per_height)
{
    this->width_per_height = width_per_height;
    int real_wi = this->width();
    int real_he = this->height();
    double real_wph = (double)real_wi / real_he;
    this->frame_width = (real_wi > real_he ? real_he / 200 : real_wi / 200);
    if ( this->frame_width == 0 ) {
        this->frame_width = 5;
    }
    if ( width_per_height > real_wph ) {
        this->draw_width = real_wi - 2 * frame_width;
        this->draw_height = std::round(draw_width / width_per_height);
        this->start_x = frame_width;
        this->start_y = (real_he - this->draw_height) / 2;
    }else {
        this->draw_height = real_he - 2 * frame_width;
        this->draw_width = std::round(draw_height * width_per_height);
        this->start_x = (real_wi - this->draw_width) / 2;
        this->start_y = frame_width;
    }

    sink_draw_real_radius = std::round((sim_run->sink_meter_size / sim_run->aoi_m_width) * draw_width);
    node_draw_real_radius = std::round((sim_run->node_meter_size / sim_run->aoi_m_width) * draw_width);

    Change_circle_draw_radius();
}

void cEvent_space::Change_circle_draw_radius()
{
    int real_wi = this->width();
    int real_he = this->height();
    if ( real_wi > real_he ) {
        node_draw_radius = real_he * (0.01 + circle_draw_radius_additional);
//        sink_draw_radius = 1.3 * node_draw_radius;
    }else {
        node_draw_radius = real_wi * (0.01 + circle_draw_radius_additional);
//        sink_draw_radius = 1.3 * node_draw_radius;
    }
}

void cEvent_space::Set_circle_draw_radius_additional()
{
    circle_draw_radius_additional = (double)(200 - sim_run->no_nodes) * 0.00017;
}

void cEvent_space::Set_sim_run(cSim_run* sim_run)
{
    this->sim_run = sim_run;
}

void cEvent_space::resizeEvent(QResizeEvent *event)
{
    if ( is_first_run ) {
        Set_draw_size((double)sim_run->aoi_unit_width / sim_run->aoi_unit_height);
        sim_run->Do_placement();
        is_first_run = false;
    }else {
        bool ir = false;
        if ( sim_run->is_running ) {
            sim_run->Play_pause();
            ir = true;
        }
//        int old_draw_width = draw_width, old_draw_height = draw_height;
        Set_draw_size(width_per_height);
//        double new_per_old_width = (double)draw_width / old_draw_width;
//        double new_per_old_height = (double)draw_height / old_draw_height;
//        for ( int i = 0; i < sim_run->no_sinks; ++i ) {
//            sim_run->sink_positions.at(i * 2) = std::round(new_per_old_width * sim_run->sink_positions.at(i * 2));
//            sim_run->sink_positions.at(i * 2 + 1) = std::round(new_per_old_height * sim_run->sink_positions.at(i * 2 + 1));
//        }
//        for ( int i = 0; i < sim_run->no_nodes; ++i ) {
//            sim_run->node_positions.at(i * 2) = std::round(new_per_old_width * sim_run->node_positions.at(i * 2));
//            sim_run->node_positions.at(i * 2 + 1) = std::round(new_per_old_height * sim_run->node_positions.at(i * 2 + 1));
//        }
        popup_widget->resize(this->width() * 0.052, this->height() * 0.06);
        if ( ir ) {
//            if ( sim_run->sink_mobility_algorithm_index > -1 ) {
//                sim_run->sink_mobility_algorithms.at(sim_run->sink_mobility_algorithm_index)->Resize_happened(new_per_old_width);
//            }
//            if ( sim_run->node_mobility_algorithm_index > -1 ) {
//                sim_run->node_mobility_algorithms.at(sim_run->node_mobility_algorithm_index)->Resize_happened(new_per_old_width);
//            }
            sim_run->Play_pause();
        }
    }
    QWidget::resizeEvent(event);
}

void cEvent_space::paintEvent(QPaintEvent *event)
{
    double pixel_per_meter = (double)draw_width / sim_run->aoi_m_width;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen the_pen(QColor(0, 14, 210));

//    draw event space
    painter.setPen(Qt::NoPen);
    painter.fillRect(start_x, start_y, draw_width, draw_height, QColor(210, 210, 210));

//    if left click on a sink or on a node
    double lc_item_x = -1;
    double lc_item_y = -1;
    if ( is_left_clicked ) {
        if ( is_dragged_sink ) {
            lc_item_x = sim_run->sink_positions[dragged_item_index * 2];
            lc_item_y = sim_run->sink_positions[dragged_item_index * 2 + 1];
        }else {
            lc_item_x = sim_run->node_positions[dragged_item_index * 2];
            lc_item_y = sim_run->node_positions[dragged_item_index * 2 + 1];
        }
    }

//    paint nodes
    the_pen.setColor(QColor(0, 0, 0));
    the_pen.setWidth(frame_width / 2);
    painter.setPen(the_pen);
    for ( int i = 0; i < sim_run->no_nodes; ++i ) {
//        qDebug() << node_positions->at(i * 2) << " " << node_positions->at(i * 2 + 1);
        double tav_sq = 123456789;
        if ( is_left_clicked ) {
            double diff_x = sim_run->node_positions[i * 2] - lc_item_x;
            double diff_y = sim_run->node_positions[i * 2 + 1] - lc_item_y;
            tav_sq = diff_x * diff_x + diff_y * diff_y;
        }
        if ( tav_sq <= sim_run->comm_range_squared ) {
            painter.setBrush(QColor(159, 0, 211));
        }else {
            painter.setBrush(QColor(40, 40, 40));
        }
        painter.drawEllipse(QPointF(start_x + (sim_run->node_positions.at(i * 2) * pixel_per_meter), start_y + (sim_run->node_positions.at(i * 2 + 1) * pixel_per_meter)), node_draw_radius, node_draw_radius);
    }

//    paint sinks
//    the_pen.setColor(QColor(137, 104, 27));
//    painter.setPen(the_pen);
    for ( int i = 0; i < sim_run->no_sinks; ++i ) {
        //        qDebug() << sink_positions->at(i * 2) << " " << sink_positions->at(i * 2 + 1);
        double tav_sq = 123456789;
        if ( is_left_clicked ) {
            double diff_x = sim_run->sink_positions[i * 2] - lc_item_x;
            double diff_y = sim_run->sink_positions[i * 2 + 1] - lc_item_y;
            tav_sq = diff_x * diff_x + diff_y * diff_y;
        }
        if ( tav_sq <= sim_run->comm_range_squared ) {
            painter.setBrush(QColor(178, 0, 70));
        }else {
            painter.setBrush(QColor(255, 174, 67));
        }
        painter.drawEllipse(QPointF(start_x + (sim_run->sink_positions.at(i * 2) * pixel_per_meter), start_y + (sim_run->sink_positions.at(i * 2 + 1) * pixel_per_meter)), node_draw_radius, node_draw_radius);
    }

//    draw comm. range circle if necessary
    if ( is_left_clicked ) {
        if ( is_dragged_sink ) {
            painter.setBrush(QColor(0, 8, 121));
            painter.drawEllipse(QPointF(start_x + (sim_run->sink_positions.at(dragged_item_index * 2) * pixel_per_meter), start_y + (sim_run->sink_positions.at(dragged_item_index * 2 + 1) * pixel_per_meter)), node_draw_radius, node_draw_radius);
        }else {
            painter.setBrush(QColor(0, 162, 181));
            painter.drawEllipse(QPointF(start_x + (sim_run->node_positions.at(dragged_item_index * 2) * pixel_per_meter), start_y + (sim_run->node_positions.at(dragged_item_index * 2 + 1) * pixel_per_meter)), node_draw_radius, node_draw_radius);
        }
        the_pen.setWidth(frame_width);
        the_pen.setColor(QColor(69, 0, 71));
        painter.setPen(the_pen);
        painter.setBrush(QColor(0, 0, 0, 0));
        double crs = sqrt(sim_run->comm_range_squared) * pixel_per_meter;
        painter.drawEllipse(QPointF(start_x + (lc_item_x * pixel_per_meter), start_y + (lc_item_y * pixel_per_meter)), crs, crs);
        the_pen.setWidth(frame_width / 2);
    }

//    paint an inner rectangle for some mobility model, e.g. Gauss-Markov, for nodes
    if ( sim_run->node_mobility_algorithm_index > -1 && sim_run->node_mobility_algorithms.at(sim_run->node_mobility_algorithm_index)->distance_from_aoi_edges_m > 0. ) {
        the_pen.setColor(QColor(35, 203, 0));
        painter.setPen(the_pen);
        painter.setBrush(QColor(0, 0, 0, 0));
        double dfae = (sim_run->node_mobility_algorithms.at(sim_run->node_mobility_algorithm_index)->distance_from_aoi_edges_m * pixel_per_meter);
        painter.drawRect(start_x + std::round(dfae), start_y + std::round(dfae), std::round(draw_width - 2 * dfae), std::round(draw_height - 2 * dfae));
    }

//    paint an inner rectangle for some mobility model, e.g. Gauss-Markov, for sinks
    if ( sim_run->sink_mobility_algorithm_index > -1 && sim_run->sink_mobility_algorithms.at(sim_run->sink_mobility_algorithm_index)->distance_from_aoi_edges_m > 0. ) {
        the_pen.setColor(QColor(234, 143, 0));
        painter.setPen(the_pen);
        painter.setBrush(QColor(0, 0, 0, 0));
        double dfae = (sim_run->sink_mobility_algorithms.at(sim_run->sink_mobility_algorithm_index)->distance_from_aoi_edges_m * pixel_per_meter);
        painter.drawRect(start_x + std::round(dfae), start_y + std::round(dfae), std::round(draw_width - 2 * dfae), std::round(draw_height - 2 * dfae));
    }

//    paint nodes collision circles
    if ( sim_run->node_mobility_algorithm_index > -1 && sim_run->node_mobility_algorithms.at(sim_run->node_mobility_algorithm_index)->collision_avoidance != iMobility_interface::none
        && sim_run->node_mobility_algorithms.at(sim_run->node_mobility_algorithm_index)->col_avoid_radius_basic > 0. ) {
        the_pen.setColor(QColor(201, 144, 0));
        painter.setPen(the_pen);
        painter.setBrush(QColor(0, 0, 0, 0));
        int carad = (sim_run->node_mobility_algorithms.at(sim_run->node_mobility_algorithm_index)->col_avoid_radius_basic * pixel_per_meter);
        for ( int i = 0; i < sim_run->no_nodes; ++i ) {
            painter.drawEllipse(QPointF(start_x + (sim_run->node_positions.at(i * 2) * pixel_per_meter), start_y + (sim_run->node_positions.at(i * 2 + 1) * pixel_per_meter)), carad, carad);
        }
    }

//    paint sinks collision circles
    if ( sim_run->sink_mobility_algorithm_index > -1 && sim_run->sink_mobility_algorithms.at(sim_run->sink_mobility_algorithm_index)->collision_avoidance != iMobility_interface::none
        && sim_run->sink_mobility_algorithms.at(sim_run->sink_mobility_algorithm_index)->col_avoid_radius_basic > 0. ) {
        the_pen.setColor(QColor(201, 144, 0));
        painter.setPen(the_pen);
        painter.setBrush(QColor(0, 0, 0, 0));
        int carad = (sim_run->sink_mobility_algorithms.at(sim_run->sink_mobility_algorithm_index)->col_avoid_radius_basic * pixel_per_meter);
        for ( int i = 0; i < sim_run->no_sinks; ++i ) {
            painter.drawEllipse(QPointF(start_x + (sim_run->sink_positions.at(i * 2) * pixel_per_meter), start_y + (sim_run->sink_positions.at(i * 2 + 1) * pixel_per_meter)), carad, carad);
        }
    }

//    paint meter sized nodes
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 255, 0));
    for ( int i = 0; i < sim_run->no_nodes; ++i ) {
        painter.drawEllipse(QPointF(start_x + (sim_run->node_positions.at(i * 2) * pixel_per_meter), start_y + (sim_run->node_positions.at(i * 2 + 1) * pixel_per_meter)), node_draw_real_radius, node_draw_real_radius);
    }

//    paint meter sized sinks
    painter.setBrush(QColor(255, 255, 255));
    for ( int i = 0; i < sim_run->no_sinks; ++i ) {
        painter.drawEllipse(QPointF(start_x + (sim_run->sink_positions.at(i * 2) * pixel_per_meter), start_y + (sim_run->sink_positions.at(i * 2 + 1) * pixel_per_meter)), sink_draw_real_radius, sink_draw_real_radius);
    }

//    draw background and frame
    painter.setPen(Qt::NoPen);
    if ( start_x > start_y ) {
        painter.fillRect(0, 0, start_x - 1, this->height(), QColor(189, 217, 230));
        painter.fillRect(start_x + draw_width + 1, 0, start_x, this->height(), QColor(189, 217, 230));
    }else {
        painter.fillRect(0, 0, this->width(), start_y - 1, QColor(189, 217, 230));
        painter.fillRect(0, start_y + draw_height + 1, this->width(), start_y, QColor(189, 217, 230));
    }

    the_pen.setWidth(frame_width);
    the_pen.setColor(QColor(0, 14, 210));
    painter.setPen(the_pen);
    painter.setBrush(QColor(0, 0, 0, 0));
    int tkiv = std::round(frame_width / 2);
    painter.drawRect(start_x - tkiv, start_y - tkiv, draw_width + frame_width, draw_height + frame_width);
}

void cEvent_space::mouseMoveEvent(QMouseEvent *event)
{
    double meter_per_pixel = (double)sim_run->aoi_m_width / draw_width;
    QWidget::mouseMoveEvent(event);
    auto mx = event->pos().x();
    auto my = event->pos().y();
    auto diff_x = (mx - mouse_x) * meter_per_pixel;
    auto diff_y = (my - mouse_y) * meter_per_pixel;
    mouse_x = mx;
    mouse_y = my;
    if ( dragged_item_index > -1 ) {
        if ( is_dragged_sink ) {
            sim_run->sink_positions.at(dragged_item_index * 2) += diff_x;
            sim_run->sink_positions.at(dragged_item_index * 2 + 1) += diff_y;
        }else {
            sim_run->node_positions.at(dragged_item_index * 2) += diff_x;
            sim_run->node_positions.at(dragged_item_index * 2 + 1) += diff_y;
        }
        update();
    }
}

void cEvent_space::mousePressEvent(QMouseEvent *event)
{
    double pixel_per_meter = (double)draw_width / sim_run->aoi_m_width;
    QWidget::mousePressEvent(event);
    mouse_x = event->pos().x();
    mouse_y = event->pos().y();
    dragged_item_index = -1;
    for ( int i = 0; i < sim_run->no_sinks; ++i ) {
        int diff_x = start_x + sim_run->sink_positions.at(i * 2) * pixel_per_meter - mouse_x;
        int diff_y = start_y + sim_run->sink_positions.at(i * 2 + 1) * pixel_per_meter - mouse_y;
        if ( (diff_x * diff_x + diff_y * diff_y) <= node_draw_radius * node_draw_radius ) {
            dragged_item_index = i;
            is_dragged_sink = true;
            if ( event->button() == Qt::RightButton ) {
                char buffer[10];
                std::snprintf(buffer, 10, "%.2f", sim_run->sink_positions.at(i * 2));
                x_lineedit->setText(buffer);
                std::snprintf(buffer, 10, "%.2f", sim_run->sink_positions.at(i * 2 + 1));
                y_lineedit->setText(buffer);
                popup_widget->move(mouse_x - popup_widget->width() / 2, mouse_y - popup_widget->height() / 2);
                popup_widget->show();
            }else if ( event->button() == Qt::LeftButton ) {
                is_left_clicked = true;
                update();
            }
            break;
        }
    }
    if ( dragged_item_index > -1 ) {
        return;
    }
    for ( int i = 0; i < sim_run->no_nodes; ++i ) {
        int diff_x = start_x + sim_run->node_positions.at(i * 2) * pixel_per_meter - mouse_x;
        int diff_y = start_y + sim_run->node_positions.at(i * 2 + 1) * pixel_per_meter - mouse_y;
        if ( (diff_x * diff_x + diff_y * diff_y) <= node_draw_radius * node_draw_radius ) {
            dragged_item_index = i;
            is_dragged_sink = false;
            if ( event->button() == Qt::RightButton ) {
                char buffer[10];
                std::snprintf(buffer, 10, "%.2f", sim_run->node_positions.at(i * 2));
                x_lineedit->setText(buffer);
                std::snprintf(buffer, 10, "%.2f", sim_run->node_positions.at(i * 2 + 1));
                y_lineedit->setText(buffer);
                popup_widget->move(mouse_x - popup_widget->width() / 2, mouse_y - popup_widget->height() / 2);
                popup_widget->show();
            }else if ( event->button() == Qt::LeftButton ) {
                is_left_clicked = true;
                update();
            }
            break;
        }
    }
    if ( dragged_item_index == -1 ) {
        popup_widget->hide();
        is_left_clicked = false;
        update();
    }
}
