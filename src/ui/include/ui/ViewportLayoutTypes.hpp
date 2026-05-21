#ifndef VIEWPORTLAYOUTTYPES_HPP
#define VIEWPORTLAYOUTTYPES_HPP

#include <array>
#include <string>
#include <vector>

#include <XQVtkViewport/ViewportConfig.hpp>

namespace ui {

/**
 * @brief Predefined layout types for the viewport layout selector.
 */
enum class ViewportLayoutType {
    SingleAxial = 0,
    SingleCoronal,
    SingleSagittal,
    HorizontalSplit,
    VerticalSplit,
    TwoPlusOne,
    TwoPlusOneReversed,  // top row: 2 squares, bottom row: 1 wide rectangle
};

/**
 * @brief Describes the viewport region and plane index for one pane in a layout.
 *
 * planeIndex maps to SliceController::Orientation: 0=Axial, 1=Coronal, 2=Sagittal.
 * -1 means the pane is unused / not rendered.
 * slotIndex is the stable positional slot (0=first, 1=second, ...) used for
 * per-slot plane assignment overrides.
 */
struct ViewportPaneConfig {
    qvv::ViewportConfig viewport;
    int planeIndex{0};
    int slotIndex{0};
};

/**
 * @brief Complete description of a layout: all active panes and their normalised rects.
 *
 * slotPlanes maps slot index → plane index (0=Axial,1=Coronal,2=Sagittal).
 * When empty the factory default assignment is used.
 */
struct ViewportLayoutDefinition {
    ViewportLayoutType type;
    std::string label;
    std::vector<ViewportPaneConfig> panes;
    std::vector<int> slotPlanes;  // per-slot plane assignment (may be empty = default)
};

/**
 * @brief Factory: returns a fully populated ViewportLayoutDefinition for each type.
 * If slotPlanes is non-empty it overrides the default plane→slot assignment.
 */
ViewportLayoutDefinition MakeLayoutDefinition(ViewportLayoutType type,
                                               const std::vector<int>& slotPlanes = {});

}  // namespace ui

#endif  // VIEWPORTLAYOUTTYPES_HPP
