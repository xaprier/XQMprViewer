#ifndef XQVtkViewport_VIEWPORTVIEW_HPP
#define XQVtkViewport_VIEWPORTVIEW_HPP

#include <memory>
#include <vector>

#include "controllers/ViewportController.hpp"
#include "ui/ILayoutTarget.hpp"
#include "ui/IView.hpp"

namespace ui {

/**
 * @brief View widget for single-window multi-viewport DICOM display.
 *
 * Implements ILayoutTarget: when ApplyLayout() is called, the renderer
 * viewport rects on the single shared render window are updated.
 *
 * If called before the VTK pipeline is ready (before first DICOM load),
 * the layout is stored and applied automatically when ViewersReady fires.
 */
class ViewportView : public IView, public ILayoutTarget {
    Q_OBJECT
  public:
    explicit ViewportView(QWidget* parent = nullptr);
    ~ViewportView() override;

    /** @brief Returns the underlying controller (e.g. to connect signals or forward commands). */
    [[nodiscard]] controllers::ViewportController* GetController() const {
        return m_viewportController.get();
    }

    // ILayoutTarget
    void ApplyLayout(const ViewportLayoutDefinition& def) override;

  private:
    void _setupUI() override;

    static std::vector<controllers::ViewportPaneDesc>
    _PanesToDesc(const std::vector<ViewportPaneConfig>& panes);

    std::unique_ptr<controllers::ViewportController> m_viewportController{};

    // Stored when ApplyLayout is called before ViewersReady.
    const ViewportLayoutDefinition* m_pendingLayout{nullptr};
    ViewportLayoutDefinition m_pendingLayoutStorage;
};

}  // namespace ui

#endif  // XQVtkViewport_VIEWPORTVIEW_HPP
