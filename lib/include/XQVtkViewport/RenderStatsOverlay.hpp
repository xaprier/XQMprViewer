#ifndef XQVtkViewport_RENDERSTATSOVERLAY_HPP
#define XQVtkViewport_RENDERSTATSOVERLAY_HPP

#include <vtkSmartPointer.h>

#include "RenderStats.hpp"
#include "ViewportConfig.hpp"
#include "XQVtkViewport_export.hpp"

class vtkRenderer;
class vtkTextActor;

namespace qvv {

/**
 * @brief Draws render statistics in the top-right corner of each viewport.
 *
 * Uses a vtkTextActor managed internally by ViewportManager.
 * Activated via ViewportManager::EnableStatsOverlay(true).
 */
class QVV_EXPORT RenderStatsOverlay {
  public:
    explicit RenderStatsOverlay(vtkRenderer* renderer);
    ~RenderStatsOverlay();

    RenderStatsOverlay(const RenderStatsOverlay&) = delete;
    RenderStatsOverlay& operator=(const RenderStatsOverlay&) = delete;

    /**
     * @brief Controls overlay visibility.
     * @param visible true to show, false to hide.
     */
    void SetVisible(bool visible);

    [[nodiscard]] bool IsVisible() const;

    /**
     * @brief Refreshes the displayed statistic values and label.
     * @param label Viewport label (e.g. "Axial").
     * @param stats Current render statistics.
     */
    void Update(const std::string& label, const RenderStats& stats);

  private:
    vtkSmartPointer<vtkTextActor> m_textActor;
    bool m_visible{true};
};

}  // namespace qvv

#endif  // XQVtkViewport_RENDERSTATSOVERLAY_HPP
