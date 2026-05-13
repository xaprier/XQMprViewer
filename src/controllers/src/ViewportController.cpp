#include "controllers/ViewportController.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <QDebug>
#include <XQVtkViewport/ViewportLayout.hpp>

#include "controllers/SliceController.hpp"
#include "controllers/SphereController.hpp"
#include "controllers/ViewportInteractorStyle.hpp"
#include "render/RenderScheduler.hpp"

namespace controllers {

ViewportController::ViewportController(QObject* parent)
    : IViewController(parent) {
    m_scheduler = std::make_unique<render::RenderScheduler>();
}

void ViewportController::_Initialize(const std::vector<QVTKOpenGLNativeWidget*> vtkWidgets) {
    if (m_initialized)
        return;

    if (vtkWidgets.empty() || !vtkWidgets[0]) {
        emit StatusChanged(tr("Cannot initialize ViewportController: no widget provided."));
        return;
    }

    auto* widget = vtkWidgets[0];

    m_vtkWidgets = {widget};
    m_renderWindows = {widget->renderWindow()};
    m_interactors = {widget->interactor()};

    m_initialized = true;
}

void ViewportController::_AddSphere() {
    qDebug() << "ViewportController::_AddSphere";
    if (!m_initialized) {
        emit StatusChanged(tr("Cannot add sphere. ViewportController is not initialized."));
        return;
    }

    if (!m_sliceController) {
        emit StatusChanged(tr("Cannot add sphere. SliceController is not initialized."));
        return;
    }

    if (m_sphereAdded) {
        emit StatusChanged(tr("Sphere is already added to Viewport."));
        return;
    }

    static constexpr SphereController::DragPlane kPlanes[3] = {
        SphereController::DragPlane::Axial,
        SphereController::DragPlane::Coronal,
        SphereController::DragPlane::Sagittal,
    };

    m_sphereController = std::make_unique<SphereController>();
    m_sphereController->SetScheduler(m_scheduler.get());

    std::vector<vtkSmartPointer<vtkRenderer>> renderers;
    m_sliceController->GetRenderers(renderers);

    for (int i = 0; i < 3; ++i) {
        if (renderers[i])
            m_sphereController->AddRenderer(renderers[i], kPlanes[i]);
    }

    if (m_interactors[0])
        m_sphereController->AddInteractor(m_interactors[0]);

    QObject::connect(
        m_sphereController.get(), &SphereController::SphereMoved,
        m_sliceController.get(), &SliceController::OnSphereUpdated);

    if (m_dicomLoaded) {
        double bounds[6];
        m_imageData->GetBounds(bounds);
        m_sphereController->SetPosition({
            (bounds[0] + bounds[1]) * 0.5,
            (bounds[2] + bounds[3]) * 0.5,
            (bounds[4] + bounds[5]) * 0.5,
        });
    }

    m_sphereController->SetRadius(m_sphereRadius);
    m_sphereController->SetColor(m_sphereColor);

    m_sphereAdded = true;
    m_sliceController->RenderAll();
}

void ViewportController::_RemoveSphere() {
    qDebug() << "ViewportController::_RemoveSphere";
    if (!m_initialized) {
        emit StatusChanged(tr("Cannot remove sphere. ViewportController is not initialized."));
        return;
    }

    if (m_sphereController) {
        m_sphereController->Cleanup();
        m_sphereController.reset();
    }

    m_sphereAdded = false;
    emit StatusChanged(tr("Sphere removed from Viewport."));
    m_sliceController->RenderAll();
}

void ViewportController::_SetSphereRadius(double radius) {
    if (!m_initialized) {
        emit StatusChanged(tr("Cannot set sphere radius. ViewportController is not initialized."));
        return;
    }

    if (!m_sphereAdded || !m_sphereController) {
        emit StatusChanged(tr("No sphere to set radius for."));
        return;
    }

    m_sphereRadius = radius;
    m_sphereController->SetRadius(radius);
    m_sliceController->RenderAll();
}

void ViewportController::_SetSphereColor(const std::array<double, 3> color) {
    if (!m_initialized) {
        emit StatusChanged(tr("Cannot set sphere color. ViewportController is not initialized."));
        return;
    }

    if (!m_sphereAdded || !m_sphereController) {
        emit StatusChanged(tr("No sphere to set color for."));
        return;
    }

    m_sphereColor = color;
    m_sphereController->SetColor(m_sphereColor);
    m_sliceController->RenderAll();
}

void ViewportController::_Render() {
    m_scheduler->RequestRenderAll();
    m_scheduler->Flush();
}

void ViewportController::_SetImageData(vtkImageData* imageData) {
    if (!m_initialized || !imageData)
        return;

    m_imageData = vtkSmartPointer<vtkImageData>::New();
    m_imageData->ShallowCopy(imageData);

    if (!m_dicomLoaded) {
        _SetupPipeline(imageData);
        m_dicomLoaded = true;
    } else {
        if (m_sliceController)
            m_sliceController->SetImageData(imageData);
    }

    if (m_sphereController) {
        double bounds[6];
        imageData->GetBounds(bounds);
        m_sphereController->SetPosition({
            (bounds[0] + bounds[1]) * 0.5,
            (bounds[2] + bounds[3]) * 0.5,
            (bounds[4] + bounds[5]) * 0.5,
        });
    }

    _Render();
    emit StatusChanged(tr("DICOM image loaded successfully into Viewport."));
}

void ViewportController::_SetupPipeline(vtkImageData* imageData) {
    if (!imageData)
        return;

    if (!m_sliceController) {
        m_sliceController = std::make_unique<SliceController>();
        m_sliceController->Initialize(m_vtkWidgets, m_scheduler.get());

        _ApplyViewportLayout();

        m_interactorStyle = vtkSmartPointer<ViewportInteractorStyle>::New();
        m_interactorStyle->SetViewers(m_sliceController->GetViewers());
        m_interactorStyle->SetScheduler(m_scheduler.get());
        m_interactors[0]->SetInteractorStyle(m_interactorStyle);

        if (m_sphereController) {
            QObject::connect(
                m_sphereController.get(), &SphereController::SphereMoved,
                m_sliceController.get(), &SliceController::OnSphereUpdated);
        }

        emit ViewersReady();
    }

    m_sliceController->SetImageData(imageData);
}

void ViewportController::_ApplyViewportLayout() {
    if (!m_sliceController)
        return;

    const auto configs = qvv::ViewportLayout::HorizontalSplit(
        3, {0.1, 0.1, 0.1}, {"Axial", "Coronal", "Sagittal"});

    std::vector<vtkSmartPointer<vtkRenderer>> renderers;
    m_sliceController->GetRenderers(renderers);

    for (int i = 0; i < 3 && i < static_cast<int>(renderers.size()); ++i) {
        if (!renderers[i])
            continue;
        const auto& cfg = configs[i];
        renderers[i]->SetViewport(cfg.xMin, cfg.yMin, cfg.xMax, cfg.yMax);
        renderers[i]->SetBackground(cfg.background[0], cfg.background[1], cfg.background[2]);
    }
}

}  // namespace controllers
