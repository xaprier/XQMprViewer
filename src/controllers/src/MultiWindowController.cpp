#include "controllers/MultiWindowController.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkResliceImageViewer.h>

#include <QDebug>
#include <QString>
#include <array>

#include "controllers/SphereController.hpp"
#include "render/RenderScheduler.hpp"

namespace controllers {

MultiWindowController::MultiWindowController(QObject* parent)
    : IViewController(parent) {
    m_scheduler = std::make_unique<render::RenderScheduler>();
}

void MultiWindowController::_Initialize(const std::vector<QVTKOpenGLNativeWidget*> vtkWidgets) {
    if (m_initialized)
        return;

    m_vtkWidgets.resize(3);
    m_renderWindows.resize(3);
    m_interactors.resize(3);
    m_renderers.resize(3);

    for (int i = 0; i < 3; ++i) {
        if (!vtkWidgets[i]) {
            emit StatusChanged(tr("Cannot initialize setup in MultiWindow for view %1.").arg(i));
            return;
        }
        m_vtkWidgets[i] = vtkWidgets[i];
        m_renderWindows[i] = vtkWidgets[i]->renderWindow();
        m_interactors[i] = vtkWidgets[i]->interactor();
    }

    m_sliceController = std::make_unique<SliceController>(this);
    m_sliceController->Initialize(m_vtkWidgets, m_scheduler.get());

    m_initialized = true;
    emit ViewersReady();
}

void MultiWindowController::_AddSphere() {
    qDebug() << "_AddSphere";
    if (!m_initialized) {
        emit StatusChanged(tr("Cannot add sphere. MultiWindowController is not initialized."));
        return;
    }

    if (!m_sliceController) {
        emit StatusChanged(tr("Cannot add sphere. SliceController is not initialized."));
        return;
    }

    if (m_sphereAdded) {
        emit StatusChanged(tr("Sphere is already added to MultiWindow."));
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
        if (m_interactors[i])
            m_sphereController->AddInteractor(m_interactors[i]);
    }

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

void MultiWindowController::_RemoveSphere() {
    qDebug() << "_RemoveSphere";
    if (!m_initialized) {
        emit StatusChanged(tr("Cannot remove sphere. MultiWindowController is not initialized."));
        return;
    }

    if (m_sphereController) {
        m_sphereController->Cleanup();
        m_sphereController.reset();
    }

    m_sphereAdded = false;
    emit StatusChanged(tr("Sphere removed from MultiWindow."));
    m_sliceController->RenderAll();
}

void MultiWindowController::_SetSphereRadius(double radius) {
    if (!m_initialized) {
        emit StatusChanged(tr("Cannot set sphere radius. MultiWindowController is not initialized."));
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

void MultiWindowController::_SetSphereColor(const std::array<double, 3> color) {
    if (!m_initialized) {
        emit StatusChanged(tr("Cannot set sphere color. MultiWindowController is not initialized."));
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

void MultiWindowController::_Render() {
    m_scheduler->RequestRenderAll();
    m_scheduler->Flush();
}

void MultiWindowController::_SetImageData(vtkImageData* imageData) {
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
    emit StatusChanged(tr("DICOM image loaded successfully into MultiWindow."));
}

void MultiWindowController::_SetupPipeline(vtkImageData* imageData) {
    if (!imageData)
        return;

    if (m_sphereController) {
        QObject::connect(
            m_sphereController.get(), &SphereController::SphereMoved,
            m_sliceController.get(), &SliceController::OnSphereUpdated);
    }

    m_sliceController->SetImageData(imageData);
}

void MultiWindowController::_ResetCameraClippingRange() {
    for (auto renderWindow : m_renderWindows) {
        if (renderWindow) {
            auto* renderer = renderWindow->GetRenderers()->GetFirstRenderer();
            if (renderer)
                renderer->ResetCameraClippingRange();
        }
    }
}

}  // namespace controllers
