#include "ui/ViewportView.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <QHBoxLayout>

#include <vtkResliceImageViewer.h>

#include "controllers/SliceController.hpp"
#include "controllers/ViewportController.hpp"
#include "overlays/CornerAnnotationOverlay.hpp"
#include "overlays/FPSOverlay.hpp"
#include "overlays/OrientationMarkerOverlay.hpp"
#include "ui/ViewportLayoutTypes.hpp"

namespace ui {

ViewportView::ViewportView(QWidget* parent)
    : IView(parent),
      m_viewportController(std::make_unique<controllers::ViewportController>()) {
    _setupUI();
}

ViewportView::~ViewportView() = default;

void ViewportView::_setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow->SetMultiSamples(0);

    auto* widget = new QVTKOpenGLNativeWidget(this);
    widget->setRenderWindow(renderWindow);
    layout->addWidget(widget);

    m_vtkWidgets = {widget};
    m_fpsOverlays = {new overlays::FPSOverlay(widget)};

    // Initial rects: HorizontalSplit (equal thirds, full height in Qt coords y=0 top)
    constexpr double kRects[3][4] = {
        {0.0,        0.0, 1.0 / 3.0, 1.0},
        {1.0 / 3.0,  0.0, 2.0 / 3.0, 1.0},
        {2.0 / 3.0,  0.0, 1.0,        1.0},
    };

    m_orientationMarkerOverlays.resize(3);
    for (int i = 0; i < 3; ++i) {
        auto* ov = new overlays::OrientationMarkerOverlay(
            widget, overlays::OrientationMarkerOverlay::kSliceOrientations[i]);
        ov->SetViewportRect(kRects[i][0], kRects[i][1], kRects[i][2], kRects[i][3]);
        m_orientationMarkerOverlays[i] = ov;
    }

    static const QString kViewNames[3] = {"Axial", "Coronal", "Sagittal"};
    m_cornerAnnotationOverlays.resize(3);
    for (int i = 0; i < 3; ++i) {
        auto* ov = new overlays::CornerAnnotationOverlay(widget, kViewNames[i]);
        ov->SetViewportRect(kRects[i][0], kRects[i][1], kRects[i][2], kRects[i][3]);
        m_cornerAnnotationOverlays[i] = ov;
    }

    m_viewportController->Initialize(m_vtkWidgets);
    connect(m_viewportController.get(), &controllers::ViewportController::StatusChanged,
            this, &ViewportView::StatusChanged);

    connect(m_viewportController.get(), &controllers::ViewportController::ViewersReady,
            this, [this]() {
                // Bind RIV viewers to corner annotation overlays.
                const auto& viewers =
                    m_viewportController->GetSliceController()->GetViewers();
                for (int i = 0; i < static_cast<int>(m_cornerAnnotationOverlays.size()); ++i) {
                    if (i < static_cast<int>(viewers.size()))
                        m_cornerAnnotationOverlays[i]->SetViewer(viewers[i].Get());
                }

                // Apply any layout that was requested before the pipeline was ready.
                if (m_pendingLayout) {
                    m_viewportController->ApplyLayout(_PanesToDesc(m_pendingLayout->panes));
                    m_pendingLayout = nullptr;
                }
            });
}

void ViewportView::ApplyLayout(const ViewportLayoutDefinition& def) {
    if (!m_viewportController->IsInitialized() ||
        !m_viewportController->GetSliceController()) {
        // Pipeline not yet ready — store and apply on ViewersReady.
        m_pendingLayoutStorage = def;
        m_pendingLayout = &m_pendingLayoutStorage;
        return;
    }
    m_viewportController->ApplyLayout(_PanesToDesc(def.panes));
}

std::vector<controllers::ViewportPaneDesc>
ViewportView::_PanesToDesc(const std::vector<ViewportPaneConfig>& panes) {
    std::vector<controllers::ViewportPaneDesc> out;
    out.reserve(panes.size());
    for (const auto& p : panes)
        out.push_back({p.viewport, p.planeIndex});
    return out;
}

}  // namespace ui
