#ifndef XQVtkViewport_VIEWPORTCONFIG_HPP
#define XQVtkViewport_VIEWPORTCONFIG_HPP

#include <array>
#include <string>

#include "XQVtkViewport_export.hpp"

namespace qvv {

/**
 * @brief Normalised coordinate and display settings for a single viewport.
 *
 * All coordinates follow VTK convention: normalised [0.0, 1.0] range with the
 * origin at the bottom-left corner.
 *
 * @code
 * qvv::ViewportConfig axial;
 * axial.xMin = 0.0; axial.xMax = 0.33;
 * axial.label = "Axial";
 * @endcode
 */
struct QVV_EXPORT ViewportConfig {
    double xMin{0.0};
    double yMin{0.0};
    double xMax{1.0};
    double yMax{1.0};
    std::array<double, 3> background{0.1, 0.1, 0.1};
    int layer{0};
    std::string label{};
};

}  // namespace qvv

#endif  // XQVtkViewport_VIEWPORTCONFIG_HPP
