#ifndef XQVtkViewport_VIEWPORTLAYOUT_HPP
#define XQVtkViewport_VIEWPORTLAYOUT_HPP

#include <string>
#include <vector>

#include "ViewportConfig.hpp"
#include "XQVtkViewport_export.hpp"

namespace qvv {

/**
 * @brief Factory utility for common viewport arrangements.
 *
 * Produces ready-to-use ViewportConfig vectors for typical multi-viewport
 * layouts. The generated configs can be fed directly to
 * ViewportManager::AddViewport() or applied to a vtkRenderer via
 * vtkRenderer::SetViewport().
 */
class QVV_EXPORT ViewportLayout {
  public:
    ViewportLayout() = delete;

    /**
     * @brief Produces @p count equal-width horizontal strip viewport configs.
     *
     * @param count      Number of viewports (clamped to >= 1).
     * @param background Background colour [R,G,B] in [0,1] applied to each viewport.
     * @param labels     Optional label list; missing entries default to empty.
     * @return           @p count ViewportConfig objects ordered left to right.
     */
    [[nodiscard]] static std::vector<ViewportConfig> HorizontalSplit(
        int count,
        std::array<double, 3> background = {0.1, 0.1, 0.1},
        const std::vector<std::string>& labels = {});

    /**
     * @brief Produces @p count equal-height vertical strip viewport configs.
     *
     * @param count      Number of viewports (clamped to >= 1).
     * @param background Background colour [R,G,B] in [0,1] applied to each viewport.
     * @param labels     Optional label list; missing entries default to empty.
     * @return           @p count ViewportConfig objects ordered bottom to top.
     */
    [[nodiscard]] static std::vector<ViewportConfig> VerticalSplit(
        int count,
        std::array<double, 3> background = {0.1, 0.1, 0.1},
        const std::vector<std::string>& labels = {});

    /**
     * @brief Produces a rows × cols grid of viewport configs.
     *
     * Ordering is row-major starting from the top-left. VTK uses a bottom-left
     * origin, so Y coordinates are automatically inverted.
     *
     * @param rows       Number of rows (clamped to >= 1).
     * @param cols       Number of columns (clamped to >= 1).
     * @param background Background colour [R,G,B] in [0,1] applied to each viewport.
     * @param labels     Optional label list; missing entries default to empty.
     * @return           rows * cols ViewportConfig objects in row-major order.
     */
    [[nodiscard]] static std::vector<ViewportConfig> Grid(
        int rows,
        int cols,
        std::array<double, 3> background = {0.1, 0.1, 0.1},
        const std::vector<std::string>& labels = {});
};

}  // namespace qvv

#endif  // XQVtkViewport_VIEWPORTLAYOUT_HPP
