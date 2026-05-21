#include "app/Application.hpp"

#include "app/Version.hpp"

namespace app {

Application::Application(int& argc, char** argv) : QApplication(argc, argv) {
    setApplicationName(Version::kAppName);
    setApplicationVersion(Version::kVersion);
    setOrganizationName(Version::kOrganization);
    setApplicationDisplayName(Version::kAppName);
}

Application* Application::instance() {
    return static_cast<Application*>(QCoreApplication::instance());
}

QString Application::documentation() const {
    return QString::fromUtf8(Version::kDocumentation);
}

}  // namespace app
