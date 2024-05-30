#include "cslider.h"

cSlider::cSlider(int min, int max, int def_value)
    : QSlider()
{
    is_first_run = true;
    is_released = false;
    this->setPageStep(1);
    this->setOrientation(Qt::Horizontal);
    this->setMinimum(min);
    this->setMaximum(max);
    this->setValue(def_value);
    this->setTracking(false);

    groove_start_label = new QLabel(this);
    groove_start_label->setText(std::to_string(min).c_str());
    groove_end_label = new QLabel(this);
    groove_end_label->setText(std::to_string(max).c_str());
    handle_label = new QLabel(this);

    QObject::connect(this, &QSlider::sliderMoved, this, &cSlider::Slider_moved);
    QObject::connect(this, &QAbstractSlider::actionTriggered, this, &cSlider::Page_changed);
    QObject::connect(this, &QSlider::sliderReleased, this, &cSlider::Slider_released);
    QObject::connect(this, &QSlider::valueChanged, this, &cSlider::Slider_moved);
}

void cSlider::Slider_released()
{
    is_released = true;
}

void cSlider::Page_changed(int action)
{
    if ( is_released == false && action == QAbstractSlider::SliderMove ) {
        Slider_moved();
    }
    is_released = false;
}

void cSlider::Slider_moved()
{
    handle_label->setText(std::to_string(this->sliderPosition()).c_str());
    handle_label->adjustSize();

    QStyleOptionSlider opt;
    this->initStyleOption(&opt);
    auto pp = this->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle);
    auto tl = pp.topLeft();

    auto wq = (handle_label->width() - pp.width()) * 0.2;
    if ( wq < 0 ) {
        wq = 0;
    }
    handle_label->move(tl.x() + 2 - ((double)this->sliderPosition() / this->maximum()) * (handle_label->width() / 2 + wq), tl.y());
}

void cSlider::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);
    if ( is_first_run ) {
        is_first_run = false;
        Set_groove_labels();
        Slider_moved();
    }
}

void cSlider::resizeEvent(QResizeEvent *event)
{
    QSlider::resizeEvent(event);
//    set the positions of the labels
    if ( ! is_first_run ) {
        Slider_moved();
        Set_groove_labels();
    }
}

void cSlider::Set_groove_labels()
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
