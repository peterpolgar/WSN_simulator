#ifndef CFP_SLIDER_H
#define CFP_SLIDER_H

#include <QSlider>
#include <QLabel>
#include <QStyle>
#include <QStyleOption>

class cFp_slider : public QSlider
{
    Q_OBJECT
public:
    cFp_slider(double min, double max, double def_value);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

    void Set_groove_labels();
    void Set_fp_value(double value);

signals:
    void Fp_value_changed(double v);

public slots:
    void Slider_moved();
    void Page_changed(int action);
    void Slider_released();
    void Floating_value_changed(int v);

private:
    const int no_units = 10000;
    double own_min;
    double own_range;

    QLabel *groove_start_label;
    QLabel *groove_end_label;
    QLabel *handle_label;
    bool is_first_run;
    bool is_released;
};

#endif // CFP_SLIDER_H
