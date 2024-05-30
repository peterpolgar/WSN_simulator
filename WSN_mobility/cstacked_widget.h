#ifndef CSTACKED_WIDGET_H
#define CSTACKED_WIDGET_H

#include <QStackedWidget>

class cStacked_widget : public QStackedWidget
{
    Q_OBJECT
public:
    cStacked_widget();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
};

#endif // CSTACKED_WIDGET_H
