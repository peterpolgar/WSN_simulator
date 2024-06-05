#include "widget.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.showMaximized();
    QFile ss("style.qss");
    if (!ss.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 2;
    }
    a.setStyleSheet(ss.readAll());
    return a.exec();
}
