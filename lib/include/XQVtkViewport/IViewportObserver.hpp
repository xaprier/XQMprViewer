#ifndef XQVtkViewport_IVIEWPORTOBSERVER_HPP
#define XQVtkViewport_IVIEWPORTOBSERVER_HPP

#include "RenderStats.hpp"
#include "XQVtkViewport_export.hpp"

namespace qvv {

/**
 * @brief Pure-virtual interface for listening to ViewportManager events.
 *
 * Implement this interface and register via ViewportManager::AddObserver() to
 * receive callbacks. All callbacks are invoked on the main thread.
 */
class QVV_EXPORT IViewportObserver {
  public:
    virtual ~IViewportObserver() = default;

    /**
     * @brief Called after a new viewport has been added.
     * @param index Index of the newly added viewport.
     */
    virtual void OnViewportAdded(int index) = 0;

    /**
     * @brief Called when a viewport is resized.
     * @param index Index of the resized viewport.
     * @param w     New width in pixels.
     * @param h     New height in pixels.
     */
    virtual void OnViewportResized(int index, int w, int h) = 0;

    /**
     * @brief Called at the end of every rendered frame.
     * @param index Index of the viewport that was rendered.
     * @param stats Render statistics for this frame.
     */
    virtual void OnFrameRendered(int index, const RenderStats& stats) = 0;
};

}  // namespace qvv

#endif  // XQVtkViewport_IVIEWPORTOBSERVER_HPP
