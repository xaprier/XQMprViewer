#include <QVTKOpenGLNativeWidget.h>

#include <QSurfaceFormat>

#include "app/Application.hpp"
#include "ui/MainWindow.hpp"

int main(int argc, char* argv[]) {
    // QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    app::Application application(argc, argv);

    ui::MainWindow window;
    window.show();
    app::Application::processEvents();

    return app::Application::exec();
}
