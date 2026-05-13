#include "ui/MultiWindowView.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <QHBoxLayout>
#include <memory>

#include <vtkResliceImageViewer.h>

#include "controllers/MultiWindowController.hpp"
#include "controllers/SliceController.hpp"
#include "overlays/CornerAnnotationOverlay.hpp"
#include "overlays/FPSOverlay.hpp"
#include "overlays/OrientationMarkerOverlay.hpp"

namespace ui {

MultiWindowView::MultiWindowView(QWidget* parent)
    : IView(parent),
      m_multiWindowController(std::make_unique<controllers::MultiWindowController>()) {
    _setupUI();
}

MultiWindowView::~MultiWindowView() = default;

void MultiWindowView::_setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    m_vtkWidgets.resize(3);
    m_fpsOverlays.resize(3);
    m_orientationMarkerOverlays.resize(3);

    for (int i = 0; i < 3; ++i) {
        auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        renderWindow->SetMultiSamples(0);

        m_vtkWidgets[i] = new QVTKOpenGLNativeWidget(this);
        m_vtkWidgets[i]->setRenderWindow(renderWindow);

        m_fpsOverlays[i] = new overlays::FPSOverlay(m_vtkWidgets[i]);
        m_orientationMarkerOverlays[i] = new overlays::OrientationMarkerOverlay(
            m_vtkWidgets[i], overlays::OrientationMarkerOverlay::kSliceOrientations[i]);

        layout->addWidget(m_vtkWidgets[i]);
    }

    // One corner annotation overlay per window.
    // Viewers are created during Initialize() → safe to bind immediately after.
    static const QString kViewNames[3] = {"Axial", "Coronal", "Sagittal"};
    m_cornerAnnotationOverlays.resize(3);
    for (int i = 0; i < 3; ++i)
        m_cornerAnnotationOverlays[i] = new overlays::CornerAnnotationOverlay(m_vtkWidgets[i], kViewNames[i]);

    m_multiWindowController->Initialize(m_vtkWidgets);
    connect(m_multiWindowController.get(), &controllers::MultiWindowController::StatusChanged,
            this, &MultiWindowView::StatusChanged);

    // Bind viewers now — SliceController is guaranteed ready after Initialize().
    const auto& viewers = m_multiWindowController->GetSliceController()->GetViewers();
    for (int i = 0; i < static_cast<int>(m_cornerAnnotationOverlays.size()); ++i) {
        if (i < static_cast<int>(viewers.size()))
            m_cornerAnnotationOverlays[i]->SetViewer(viewers[i].Get());
    }
}

}  // namespace ui
