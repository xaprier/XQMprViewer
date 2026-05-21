#ifndef XQVtkViewport_APP_APPLICATION_HPP
#define XQVtkViewport_APP_APPLICATION_HPP

#include <QApplication>
#include <QString>

namespace app {

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);

    static Application* instance();

    QString documentation() const;
};

inline Application* qAppInstance() {
    return Application::instance();
}

}  // namespace app

#endif  // XQVtkViewport_APP_APPLICATION_HPP
