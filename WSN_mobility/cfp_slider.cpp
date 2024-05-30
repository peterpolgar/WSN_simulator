#include "cfp_slider.h"
#include <cmath>

cFp_slider::cFp_slider(double min, double max, double def_value)
    : QSlider()
{
    own_min = min;
    own_range = max - min;

    is_first_run = true;
    is_released = false;
    this->setPageStep(1);
    this->setOrientation(Qt::Horizontal);
    this->setMinimum(0);
    this->setMaximum(no_units);
    this->setValue(std::round((def_value - own_min) / (own_range / no_units)));
    this->setTracking(false);

    groove_start_label = new QLabel(this);
    auto strmin = std::to_string(min);
    groove_start_label->setText(strmin.substr(0, strmin.find(".")+3).c_str());
    groove_end_label = new QLabel(this);
    auto strmax = std::to_string(max);
    groove_end_label->setText(strmax.substr(0, strmax.find(".")+3).c_str());
    handle_label = new QLabel(this);

    QObject::connect(this, &QSlider::sliderMoved, this, &cFp_slider::Slider_moved);
    QObject::connect(this, &QAbstractSlider::actionTriggered, this, &cFp_slider::Page_changed);
    QObject::connect(this, &QSlider::sliderReleased, this, &cFp_slider::Slider_released);
    QObject::connect(this, &QSlider::valueChanged, this, &cFp_slider::Floating_value_changed);
    QObject::connect(this, &QSlider::valueChanged, this, &cFp_slider::Slider_moved);
}

void cFp_slider::Slider_released()
{
    is_released = true;
}

void cFp_slider::Page_changed(int action)
{
    if ( is_released == false && action == QAbstractSlider::SliderMove ) {
        Slider_moved();
    }
    is_released = false;
}

void cFp_slider::Slider_moved()
{
    auto tmpstr = std::to_string(((double)this->sliderPosition() / no_units) * own_range + own_min);
    handle_label->setText(tmpstr.substr(0, tmpstr.find(".")+3).c_str());
    handle_label->adjustSize();

    QStyleOptionSlider opt;
    this->initStyleOption(&opt);
    auto pp = this->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle);
    auto tl = pp.topLeft();

    auto wq = (handle_label->width() - pp.width()) * 0.4;
    if ( wq < 0 ) {
        wq = 0;
    }
    handle_label->move(tl.x() + 2 - ((double)this->sliderPosition() / this->maximum()) * (handle_label->width() / 2 + wq), tl.y());
}

void cFp_slider::Floating_value_changed(int v)
{
    emit Fp_value_changed(((double)this->value() / no_units) * own_range + own_min);
}

void cFp_slider::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);
    if ( is_first_run ) {
        is_first_run = false;
        Set_groove_labels();
        Slider_moved();
    }
}

void cFp_slider::resizeEvent(QResizeEvent *event)
{
    QSlider::resizeEvent(event);
    //    set the positions of the labels
    if ( ! is_first_run ) {
        Slider_moved();
        Set_groove_labels();
    }
}

void cFp_slider::Set_groove_labels()
{
    QStyleOptionSlider opt;
    this->initStyleOption(&opt);
    auto sr = this->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove);

    groove_start_label->adjustSize();
    auto a = sr.bottomLeft();
    groove_start_label->move(a.x(), a.y() - groove_start_label->height());

    groove_end_label->adjustSize();
    a = sr.bottomRight();
    groove_end_label->move(a.x() - groove_end_label->width(), a.y() - groove_end_label->height());
}

void cFp_slider::Set_fp_value(double value)
{
    setValue(std::round((value - own_min) / (own_range / no_units)));
}
