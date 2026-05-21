#ifndef MULTIWINDOWVIEW_HPP
#define MULTIWINDOWVIEW_HPP

#include <memory>

#include "ui/ILayoutTarget.hpp"
#include "ui/IView.hpp"

class QGridLayout;

namespace controllers {
class MultiWindowController;
}

namespace ui {

/**
 * @brief View widget for multi-window DICOM display.
 *
 * Creates three QVTKOpenGLNativeWidget instances, each backed by its own
 * render window. ApplyLayout() rebuilds the QGridLayout geometry to match
 * the selected layout type (horizontal, vertical, 2+1, 1+2, singles).
 */
class MultiWindowView : public IView, public ILayoutTarget {
    Q_OBJECT
  public:
    explicit MultiWindowView(QWidget* parent = nullptr);
    ~MultiWindowView() override;

    /** @brief Returns the underlying controller (e.g. to connect signals or forward commands). */
    [[nodiscard]] controllers::MultiWindowController* GetController() const {
        return m_multiWindowController.get();
    }

    // ILayoutTarget
    void ApplyLayout(const ViewportLayoutDefinition& def) override;

  protected:
    void showEvent(QShowEvent* event) override;

  private:
    void _setupUI() override;
    void _RebuildGrid(const ViewportLayoutDefinition& def);

    std::unique_ptr<controllers::MultiWindowController> m_multiWindowController{};
    QGridLayout* m_grid{nullptr};
};

}  // namespace ui

#endif  // MULTIWINDOWVIEW_HPP
