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

    // Three overlays share the single widget; each is clipped to its horizontal viewport third.
    constexpr double kFractions[3][2] = {{0.0, 1.0 / 3.0}, {1.0 / 3.0, 2.0 / 3.0}, {2.0 / 3.0, 1.0}};
    m_orientationMarkerOverlays.resize(3);
    for (int i = 0; i < 3; ++i) {
        auto* ov = new overlays::OrientationMarkerOverlay(
            widget, overlays::OrientationMarkerOverlay::kSliceOrientations[i]);
        ov->SetViewportFraction(kFractions[i][0], kFractions[i][1]);
        m_orientationMarkerOverlays[i] = ov;
    }

    // Three corner annotation overlays — each clipped to its viewport third.
    // Viewers are NOT yet available (ViewportController creates SliceController lazily
    // on first image load). Bind viewers when ViewersReady fires.
    static const QString kViewNames[3] = {"Axial", "Coronal", "Sagittal"};
    m_cornerAnnotationOverlays.resize(3);
    for (int i = 0; i < 3; ++i) {
        auto* ov = new overlays::CornerAnnotationOverlay(widget, kViewNames[i]);
        ov->SetViewportFraction(kFractions[i][0], kFractions[i][1]);
        m_cornerAnnotationOverlays[i] = ov;
    }

    m_viewportController->Initialize(m_vtkWidgets);
    connect(m_viewportController.get(), &controllers::ViewportController::StatusChanged,
            this, &ViewportView::StatusChanged);

    // Bind RIV viewers to overlays once the pipeline is first set up.
    connect(m_viewportController.get(), &controllers::ViewportController::ViewersReady,
            this, [this]() {
                const auto& viewers =
                    m_viewportController->GetSliceController()->GetViewers();
                for (int i = 0; i < static_cast<int>(m_cornerAnnotationOverlays.size()); ++i) {
                    if (i < static_cast<int>(viewers.size()))
                        m_cornerAnnotationOverlays[i]->SetViewer(viewers[i].Get());
                }
            });
}

}  // namespace ui
