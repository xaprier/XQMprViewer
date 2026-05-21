#include "ui/ViewportLayoutTypes.hpp"

namespace ui {

namespace {

constexpr std::array<double, 3> kBg{0.1, 0.1, 0.1};

static const char* kPlaneLabels[3] = {"Axial", "Coronal", "Sagittal"};

ViewportPaneConfig MakePane(double xMin, double yMin, double xMax, double yMax,
                            int slotIndex, int planeIndex) {
    qvv::ViewportConfig cfg;
    cfg.xMin = xMin;
    cfg.yMin = yMin;
    cfg.xMax = xMax;
    cfg.yMax = yMax;
    cfg.background = kBg;
    cfg.label = (planeIndex >= 0 && planeIndex < 3) ? kPlaneLabels[planeIndex] : "";
    return {cfg, planeIndex, slotIndex};
}

// Apply slotPlanes override: remap pane planeIndex values by slot order.
void ApplySlotPlanes(ViewportLayoutDefinition& def, const std::vector<int>& slotPlanes) {
    if (slotPlanes.empty()) return;
    for (auto& pane : def.panes) {
        if (pane.slotIndex >= 0 && pane.slotIndex < static_cast<int>(slotPlanes.size())) {
            pane.planeIndex = slotPlanes[pane.slotIndex];
            if (pane.planeIndex >= 0 && pane.planeIndex < 3)
                pane.viewport.label = kPlaneLabels[pane.planeIndex];
        }
    }
    def.slotPlanes = slotPlanes;
}

}  // namespace

ViewportLayoutDefinition MakeLayoutDefinition(ViewportLayoutType type,
                                               const std::vector<int>& slotPlanes) {
    ViewportLayoutDefinition def;
    def.type = type;

    switch (type) {
        case ViewportLayoutType::SingleAxial:
            def.label = "Axial";
            def.panes = {MakePane(0.0, 0.0, 1.0, 1.0, 0, 0)};
            break;

        case ViewportLayoutType::SingleCoronal:
            def.label = "Coronal";
            def.panes = {MakePane(0.0, 0.0, 1.0, 1.0, 0, 1)};
            break;

        case ViewportLayoutType::SingleSagittal:
            def.label = "Sagittal";
            def.panes = {MakePane(0.0, 0.0, 1.0, 1.0, 0, 2)};
            break;

        case ViewportLayoutType::HorizontalSplit:
            // Three equal columns (Axial | Coronal | Sagittal)
            def.label = "Horizontal";
            def.panes = {
                MakePane(0.0,        0.0, 1.0 / 3.0, 1.0, 0, 0),
                MakePane(1.0 / 3.0,  0.0, 2.0 / 3.0, 1.0, 1, 1),
                MakePane(2.0 / 3.0,  0.0, 1.0,        1.0, 2, 2),
            };
            break;

        case ViewportLayoutType::VerticalSplit:
            // Three equal rows (bottom=Axial, mid=Coronal, top=Sagittal — VTK y=0 is bottom)
            def.label = "Vertical";
            def.panes = {
                MakePane(0.0, 0.0,        1.0, 1.0 / 3.0, 0, 0),
                MakePane(0.0, 1.0 / 3.0,  1.0, 2.0 / 3.0, 1, 1),
                MakePane(0.0, 2.0 / 3.0,  1.0, 1.0,        2, 2),
            };
            break;

        case ViewportLayoutType::TwoPlusOne:
            // Left half: large (slot 0); right half: top (slot 1) + bottom (slot 2)
            def.label = "2+1";
            def.panes = {
                MakePane(0.0, 0.0, 0.5, 1.0, 0, 0),
                MakePane(0.5, 0.5, 1.0, 1.0, 1, 1),
                MakePane(0.5, 0.0, 1.0, 0.5, 2, 2),
            };
            break;

        case ViewportLayoutType::TwoPlusOneReversed:
            // Top row: left (slot 0) + right (slot 1); bottom row: full-width (slot 2)
            def.label = "1+2";
            def.panes = {
                MakePane(0.0, 0.5, 0.5, 1.0, 0, 0),
                MakePane(0.5, 0.5, 1.0, 1.0, 1, 1),
                MakePane(0.0, 0.0, 1.0, 0.5, 2, 2),
            };
            break;
    }

    ApplySlotPlanes(def, slotPlanes);
    return def;
}

}  // namespace ui
