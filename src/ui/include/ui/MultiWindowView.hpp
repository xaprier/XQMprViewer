#ifndef MULTIWINDOWVIEW_HPP
#define MULTIWINDOWVIEW_HPP

#include <memory>

#include "ui/IView.hpp"

namespace controllers {
class MultiWindowController;
}

namespace ui {

/**
 * @brief View widget for multi-window DICOM display.
 *
 * Creates three QVTKOpenGLNativeWidget instances arranged side by side, each
 * backed by its own render window (Axial / Coronal / Sagittal).
 */
class MultiWindowView : public IView {
    Q_OBJECT
  public:
    explicit MultiWindowView(QWidget* parent = nullptr);
    ~MultiWindowView() override;

    /** @brief Returns the underlying controller (e.g. to connect signals). */
    controllers::MultiWindowController* GetController() {
        return m_multiWindowController.get();
    }

  private:
    void _setupUI() override;

    std::unique_ptr<controllers::MultiWindowController> m_multiWindowController{};
};
}  // namespace ui

#endif  // MULTIWINDOWVIEW_HPP
