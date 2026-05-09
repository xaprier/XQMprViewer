#include <QVTKOpenGLNativeWidget.h>

#include <QApplication>
#include <QSurfaceFormat>

#include "ui/MainWindow.hpp"

int main(int argc, char* argv[]) {
    // QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);
    app.setApplicationName("XQVtkViewport_Widgets");
    app.setOrganizationName("xaprier");

    ui::MainWindow window;
    window.show();
    QApplication::processEvents();

    return app.exec();
}
