#include "cstacked_widget.h"

cStacked_widget::cStacked_widget()
    : QStackedWidget()
{

}

QSize cStacked_widget::sizeHint() const
{
    if ( count() > 0 ) {
        return currentWidget()->sizeHint();
    }else {
        return QStackedWidget::sizeHint();
    }
}

QSize cStacked_widget::minimumSizeHint() const
{
    if ( count() > 0 ) {
        return currentWidget()->minimumSizeHint();
    }else {
        return QStackedWidget::minimumSizeHint();
    }
}
