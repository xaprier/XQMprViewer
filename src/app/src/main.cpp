#include <QVTKOpenGLNativeWidget.h>

#include "app/Application.hpp"
#include "ui/MainWindow.hpp"

int main(int argc, char* argv[]) {
    app::Application application(argc, argv);

    ui::MainWindow window;
    window.show();
    app::Application::processEvents();

    return app::Application::exec();
}
