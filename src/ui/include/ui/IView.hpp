#ifndef IVIEW_HPP
#define IVIEW_HPP

#include <vtkSmartPointer.h>

#include <QWidget>
#include <vector>

class vtkGenericOpenGLRenderWindow;
class vtkRenderWindowInteractor;
class QVTKOpenGLNativeWidget;

namespace ui {

/**
 * @brief Abstract base for all render-view widgets.
 *
 * Subclasses own one or more QVTKOpenGLNativeWidget instances and implement
 * _setupUI() to lay them out. StatusChanged is forwarded from the
 * controller so the main window can show it in a status bar.
 */
class IView : public QWidget {
    Q_OBJECT
  public:
    explicit IView(QWidget* parent = nullptr) : QWidget(parent) {}
    ~IView() = default;

  Q_SIGNALS:
    /** @brief Forwarded from the underlying controller for status bar display. */
    void StatusChanged(const QString& message);

  protected:
    virtual void _setupUI() = 0;

    std::vector<QVTKOpenGLNativeWidget*> m_vtkWidgets{};
};
}  // namespace ui

#endif  // IVIEW_HPP
