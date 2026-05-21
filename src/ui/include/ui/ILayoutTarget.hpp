#ifndef ILAYOUTTARGET_HPP
#define ILAYOUTTARGET_HPP

#include "ui/ViewportLayoutTypes.hpp"

namespace ui {

/**
 * @brief Interface implemented by any view that can accept a runtime layout change.
 *
 * ViewportLayoutManager calls ApplyLayout() on every registered ILayoutTarget
 * when the user selects a new layout. Each concrete view translates the abstract
 * ViewportLayoutDefinition into its own mode-specific behavior:
 *
 *  - ViewportView  → updates renderer viewport rects on the shared render window
 *  - MultiWindowView → shows/hides QVTKOpenGLNativeWidget instances
 */
class ILayoutTarget {
  public:
    virtual ~ILayoutTarget() = default;

    /**
     * @brief Apply the given layout to this view.
     *
     * Implementations must:
     *  1. Update the visual arrangement of their render surface(s).
     *  2. Request a render so changes appear immediately.
     *
     * Called by ViewportLayoutManager whenever the active layout changes.
     * Also called once after the pipeline is initialised so the initial
     * layout is applied correctly.
     */
    virtual void ApplyLayout(const ViewportLayoutDefinition& def) = 0;
};

}  // namespace ui

#endif  // ILAYOUTTARGET_HPP
