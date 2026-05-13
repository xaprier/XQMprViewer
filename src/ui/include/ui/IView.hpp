#ifndef IVIEW_HPP
#define IVIEW_HPP

#include <vtkSmartPointer.h>

#include <QWidget>
#include <vector>

class vtkGenericOpenGLRenderWindow;
class vtkRenderWindowInteractor;
class QVTKOpenGLNativeWidget;

#include "overlays/CornerAnnotationOverlay.hpp"
#include "overlays/FPSOverlay.hpp"
#include "overlays/OrientationMarkerOverlay.hpp"

namespace ui {

/**
 * @brief Abstract base for all render-view widgets.
 *
 * Subclasses own one or more QVTKOpenGLNativeWidget instances and implement
 * _setupUI() to lay them out. StatusChanged is forwarded from the
 * controller so the main window can show it in a status bar.
 *
 * GetOverlays<T>() provides a uniform accessor for any registered overlay type.
 * To add a new overlay type: add a member vector and a GetOverlays<NewType>()
 * specialisation here — no changes needed in MainWindow.
 */
class IView : public QWidget {
    Q_OBJECT
  public:
    explicit IView(QWidget* parent = nullptr) : QWidget(parent) {}
    ~IView() = default;

    template <typename T>
    std::vector<T*>& GetOverlays();

  Q_SIGNALS:
    void StatusChanged(const QString& message);

  protected:
    virtual void _setupUI() = 0;

    std::vector<QVTKOpenGLNativeWidget*> m_vtkWidgets{};
    std::vector<overlays::FPSOverlay*> m_fpsOverlays{};
    std::vector<overlays::OrientationMarkerOverlay*> m_orientationMarkerOverlays{};
    std::vector<overlays::CornerAnnotationOverlay*> m_cornerAnnotationOverlays{};
};

template <>
inline std::vector<overlays::FPSOverlay*>& IView::GetOverlays<overlays::FPSOverlay>() {
    return m_fpsOverlays;
}

template <>
inline std::vector<overlays::OrientationMarkerOverlay*>&
IView::GetOverlays<overlays::OrientationMarkerOverlay>() {
    return m_orientationMarkerOverlays;
}

template <>
inline std::vector<overlays::CornerAnnotationOverlay*>&
IView::GetOverlays<overlays::CornerAnnotationOverlay>() {
    return m_cornerAnnotationOverlays;
}

}  // namespace ui

#endif  // IVIEW_HPP
