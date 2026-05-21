#include "ui/OverlayLayoutAdapter.hpp"

#include "overlays/CornerAnnotationOverlay.hpp"
#include "overlays/FPSOverlay.hpp"
#include "overlays/OrientationMarkerOverlay.hpp"

namespace ui {

OverlayLayoutAdapter::OverlayLayoutAdapter(bool sharedViewport)
    : m_sharedViewport(sharedViewport) {
    m_currentPane.fill(nullptr);
}

void OverlayLayoutAdapter::AddOrientationMarkerOverlay(overlays::OrientationMarkerOverlay* ov) {
    if (ov)
        m_orientationOverlays.push_back(ov);
}

void OverlayLayoutAdapter::AddCornerAnnotationOverlay(overlays::CornerAnnotationOverlay* ov) {
    if (ov)
        m_cornerAnnotationOverlays.push_back(ov);
}

void OverlayLayoutAdapter::AddFPSOverlay(overlays::FPSOverlay* ov) {
    if (ov)
        m_fpsOverlays.push_back(ov);
}

void OverlayLayoutAdapter::Sync(const ViewportLayoutDefinition& def) {
    // Rebuild plane→pane map and cache pane data.
    m_currentPane.fill(nullptr);
    for (const auto& pane : def.panes) {
        const int idx = pane.planeIndex;
        if (idx >= 0 && idx < kNumPlanes && !m_currentPane[idx]) {
            m_paneStorage[idx] = pane;
            m_currentPane[idx] = &m_paneStorage[idx];
        }
    }

    for (int i = 0; i < kNumPlanes; ++i)
        m_layoutVisible[i] = (m_currentPane[i] != nullptr);

    // Apply to orientation overlays.
    for (int i = 0; i < static_cast<int>(m_orientationOverlays.size()); ++i) {
        _ApplyVisibility(m_orientationOverlays[i],
                         i < kNumPlanes ? m_orientationUserEnabled[i] : false,
                         i < kNumPlanes ? m_layoutVisible[i] : false,
                         i < kNumPlanes ? m_currentPane[i] : nullptr);
    }

    // Apply to corner annotation overlays.
    for (int i = 0; i < static_cast<int>(m_cornerAnnotationOverlays.size()); ++i) {
        _ApplyVisibility(m_cornerAnnotationOverlays[i],
                         i < kNumPlanes ? m_cornerUserEnabled[i] : false,
                         i < kNumPlanes ? m_layoutVisible[i] : false,
                         i < kNumPlanes ? m_currentPane[i] : nullptr);
    }
}

void OverlayLayoutAdapter::SetOrientationMarkerUserEnabled(int planeIndex, bool enabled) {
    if (planeIndex < 0 || planeIndex >= kNumPlanes)
        return;
    m_orientationUserEnabled[planeIndex] = enabled;
    if (planeIndex < static_cast<int>(m_orientationOverlays.size())) {
        _ApplyVisibility(m_orientationOverlays[planeIndex],
                         enabled, m_layoutVisible[planeIndex], m_currentPane[planeIndex]);
    }
}

void OverlayLayoutAdapter::SetCornerAnnotationUserEnabled(int planeIndex, bool enabled) {
    if (planeIndex < 0 || planeIndex >= kNumPlanes)
        return;
    m_cornerUserEnabled[planeIndex] = enabled;
    if (planeIndex < static_cast<int>(m_cornerAnnotationOverlays.size())) {
        _ApplyVisibility(m_cornerAnnotationOverlays[planeIndex],
                         enabled, m_layoutVisible[planeIndex], m_currentPane[planeIndex]);
    }
}

void OverlayLayoutAdapter::SetAllOrientationMarkersUserEnabled(bool enabled) {
    for (int i = 0; i < kNumPlanes; ++i)
        SetOrientationMarkerUserEnabled(i, enabled);
}

void OverlayLayoutAdapter::SetAllCornerAnnotationsUserEnabled(bool enabled) {
    for (int i = 0; i < kNumPlanes; ++i)
        SetCornerAnnotationUserEnabled(i, enabled);
}

// VTK viewport uses bottom-left origin; Qt uses top-left.
// Convert: qtYMin = 1 - vtkYMax,  qtYMax = 1 - vtkYMin
static void ApplyRect(overlays::OrientationMarkerOverlay* ov,
                      bool sharedViewport, const ViewportPaneConfig* pane) {
    if (sharedViewport && pane) {
        const auto& vp = pane->viewport;
        ov->SetViewportRect(vp.xMin, 1.0 - vp.yMax, vp.xMax, 1.0 - vp.yMin);
    } else {
        ov->SetViewportRect(0.0, 0.0, 1.0, 1.0);
    }
}

static void ApplyRect(overlays::CornerAnnotationOverlay* ov,
                      bool sharedViewport, const ViewportPaneConfig* pane) {
    if (sharedViewport && pane) {
        const auto& vp = pane->viewport;
        ov->SetViewportRect(vp.xMin, 1.0 - vp.yMax, vp.xMax, 1.0 - vp.yMin);
    } else {
        ov->SetViewportRect(0.0, 0.0, 1.0, 1.0);
    }
}

void OverlayLayoutAdapter::_ApplyVisibility(overlays::OrientationMarkerOverlay* ov,
                                            bool userEnabled, bool layoutVisible,
                                            const ViewportPaneConfig* pane) {
    if (!ov)
        return;
    const bool show = userEnabled && layoutVisible;
    // Use Qt setVisible — bypasses IOverlay::m_enabled so user toggle state is
    // unaffected. The adapter owns the FINAL visibility decision.
    static_cast<QWidget*>(ov)->setVisible(show);
    if (show)
        ApplyRect(ov, m_sharedViewport, pane);
}

void OverlayLayoutAdapter::_ApplyVisibility(overlays::CornerAnnotationOverlay* ov,
                                            bool userEnabled, bool layoutVisible,
                                            const ViewportPaneConfig* pane) {
    if (!ov)
        return;
    const bool show = userEnabled && layoutVisible;
    static_cast<QWidget*>(ov)->setVisible(show);
    if (show)
        ApplyRect(ov, m_sharedViewport, pane);
}

}  // namespace ui
