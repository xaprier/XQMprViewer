#ifndef OVERLAYLAYOUTADAPTER_HPP
#define OVERLAYLAYOUTADAPTER_HPP

#include <array>
#include <vector>

#include "ui/ViewportLayoutTypes.hpp"

namespace overlays {
class OrientationMarkerOverlay;
class CornerAnnotationOverlay;
class FPSOverlay;
}  // namespace overlays

namespace adapters {

/**
 * @brief Bridges the layout system and the overlay system for one IView.
 *
 * Manages the interplay between two independent visibility axes:
 *
 *   Axis A — User toggle (enabled/disabled via ControllerPanel).
 *   Axis B — Layout visibility (is this plane's pane present in the active layout?).
 *
 * An overlay is shown only when BOTH axes are true.
 *
 * Usage:
 *   1. Construct with sharedViewport=true for ViewportView (one widget),
 *      false for MultiWindowView (one widget per plane).
 *   2. Register overlays with AddXxxOverlay(). Order = planeIndex (0/1/2).
 *   3. Call Sync() whenever the layout changes.
 *   4. Call NotifyUserEnabled<T>(i, bool) when the user toggles an overlay type
 *      so the adapter can recompute visibility correctly.
 */
class OverlayLayoutAdapter {
  public:
    explicit OverlayLayoutAdapter(bool sharedViewport);

    /**
     * @brief Register overlays in planeIndex order (0=Axial, 1=Coronal, 2=Sagittal).
     *
     * Ownership is NOT transferred; the caller (IView) owns the overlays.
     * Each Add call appends one entry; call exactly once per plane per type.
     */
    void AddOrientationMarkerOverlay(overlays::OrientationMarkerOverlay* ov);
    void AddCornerAnnotationOverlay(overlays::CornerAnnotationOverlay* ov);
    void AddFPSOverlay(overlays::FPSOverlay* ov);

    /**
     * @brief Synchronise all registered overlays to the given layout definition.
     *
     * Updates viewport fractions (shared-viewport mode) and widget visibility
     * for all registered overlays. User-enable state is preserved.
     */
    void Sync(const ui::ViewportLayoutDefinition& def);

    /**
     * @brief Notify the adapter that the user has toggled orientation marker
     *        enable state for plane @p planeIndex.
     *
     * The adapter recomputes the widget visibility as (userEnabled AND layoutVisible).
     */
    void SetOrientationMarkerUserEnabled(int planeIndex, bool enabled);
    void SetCornerAnnotationUserEnabled(int planeIndex, bool enabled);

    /**
     * @brief Apply user-enable for ALL planes at once (e.g., global checkbox).
     */
    void SetAllOrientationMarkersUserEnabled(bool enabled);
    void SetAllCornerAnnotationsUserEnabled(bool enabled);

  private:
    static constexpr int kNumPlanes = 3;

    void _ApplyVisibility(overlays::OrientationMarkerOverlay* ov,
                          bool userEnabled, bool layoutVisible,
                          const ui::ViewportPaneConfig* pane);
    void _ApplyVisibility(overlays::CornerAnnotationOverlay* ov,
                          bool userEnabled, bool layoutVisible,
                          const ui::ViewportPaneConfig* pane);

    bool m_sharedViewport{true};

    // Per-plane pointers (index = planeIndex).
    std::vector<overlays::OrientationMarkerOverlay*> m_orientationOverlays;
    std::vector<overlays::CornerAnnotationOverlay*> m_cornerAnnotationOverlays;
    std::vector<overlays::FPSOverlay*> m_fpsOverlays;

    // User-enable cache: true = user wants to see this overlay.
    // Initialised to true (overlays are visible by default).
    std::array<bool, kNumPlanes> m_orientationUserEnabled{true, true, true};
    std::array<bool, kNumPlanes> m_cornerUserEnabled{true, true, true};

    // Current layout visibility per plane (updated by Sync).
    std::array<bool, kNumPlanes> m_layoutVisible{true, true, true};

    // Current pane configs (for fraction re-application on user-enable toggle).
    std::array<const ui::ViewportPaneConfig*, kNumPlanes> m_currentPane{nullptr, nullptr, nullptr};

    // Storage for pane configs (copied from layout definition during Sync).
    std::array<ui::ViewportPaneConfig, kNumPlanes> m_paneStorage{};
};

}  // namespace adapters

#endif  // OVERLAYLAYOUTADAPTER_HPP
