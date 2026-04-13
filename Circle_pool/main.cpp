#include "Circle_pool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Circle_pool window;
    window.show();
    return app.exec();
}
