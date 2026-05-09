#ifndef XQVtkViewport_VIEWPORTVIEW_HPP
#define XQVtkViewport_VIEWPORTVIEW_HPP

#include <memory>

#include "ui/IView.hpp"

namespace controllers {
class ViewportController;
}

namespace ui {

/**
 * @brief View widget for single-window multi-viewport DICOM display.
 *
 * Implements the same IView interface as MultiWindowView. A single
 * QVTKOpenGLNativeWidget's render window is split into three equal horizontal
 * regions (Axial / Coronal / Sagittal) by ViewportController.
 */
class ViewportView : public IView {
    Q_OBJECT
  public:
    explicit ViewportView(QWidget* parent = nullptr);
    ~ViewportView() override;

    [[nodiscard]] controllers::ViewportController* GetController() const {
        return m_viewportController.get();
    }

  private:
    void _setupUI() override;

    std::unique_ptr<controllers::ViewportController> m_viewportController{};
};

}  // namespace ui

#endif  // XQVtkViewport_VIEWPORTVIEW_HPP
