#include <QApplication>
#include "app/application_controller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app::ApplicationController controller;
    controller.start();
    return app.exec();
}
