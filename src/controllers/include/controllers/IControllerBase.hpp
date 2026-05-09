#ifndef CONTROLLERBASE_HPP
#define CONTROLLERBASE_HPP

#include <QObject>

namespace controllers {

/**
 * @brief Minimal QObject base shared by all controllers.
 *
 * Provides the StatusChanged signal so any controller subclass can push
 * human-readable status text to the UI without knowing about it directly.
 */
class IControllerBase : public QObject {
    Q_OBJECT
  public:
    explicit IControllerBase(QObject* parent = nullptr) : QObject(parent) {}
    ~IControllerBase() override = default;

  Q_SIGNALS:
    /** @brief Emitted whenever the controller has a status update for the user. */
    void StatusChanged(const QString& message);
};
}  // namespace controllers

#endif  // CONTROLLERBASE_HPP
