#ifndef CSLIDER_H
#define CSLIDER_H

#include <QStyle>
#include <QStyleOption>
#include <QLabel>
#include <QSlider>

class cSlider : public QSlider
{
    Q_OBJECT
public:
    cSlider(int min, int max, int def_value);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

    void Set_groove_labels();
public slots:
    void Slider_moved();
    void Page_changed(int action);
    void Slider_released();

private:
    QLabel *groove_start_label;
    QLabel *groove_end_label;
    QLabel *handle_label;
    bool is_first_run;
    bool is_released;
};

#endif // CSLIDER_H
