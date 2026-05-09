#ifndef XQVtkViewport_RENDERSTATS_HPP
#define XQVtkViewport_RENDERSTATS_HPP

#include "XQVtkViewport_export.hpp"

namespace qvv {

/**
 * @brief Per-viewport render performance statistics.
 *
 * - @c frameTimeMs   — elapsed time between the last two frames (ms).
 * - @c renderTimeMs  — time spent inside vtkRenderWindow::Render() (ms).
 * - @c frameCount    — total number of frames rendered since creation.
 */
struct QVV_EXPORT RenderStats {
    double frameTimeMs{0.0};
    double renderTimeMs{0.0};
    int frameCount{0};

    /**
     * @brief Computes instantaneous FPS from @c frameTimeMs.
     * @return FPS value; returns 0.0 if @c frameTimeMs is zero.
     */
    [[nodiscard]] double AverageFps() const;

    /**
     * @brief Returns the last frame time in milliseconds.
     * @return @c frameTimeMs when @c frameCount > 0, otherwise 0.0.
     */
    [[nodiscard]] double AverageFrameTimeMs() const;

    /**
     * @brief Resets all statistics to zero.
     */
    void Reset();
};

}  // namespace qvv

#endif  // XQVtkViewport_RENDERSTATS_HPP
