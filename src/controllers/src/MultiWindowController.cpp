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

namespace controllers {

MultiWindowController::MultiWindowController(QObject* parent)
    : IViewController(parent) {
}

MultiWindowController::~MultiWindowController() = default;

void MultiWindowController::_Initialize(const std::vector<QVTKOpenGLNativeWidget*> vtkWidgets) {
    if (m_initialized) {
        return;
    }

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

    m_initialized = true;
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

    std::vector<vtkSmartPointer<vtkRenderer>> renderers;
    m_sliceController->GetRenderers(renderers);

    for (int i = 0; i < 3; ++i) {
        if (renderers[i]) {
            m_sphereController->AddRenderer(renderers[i], kPlanes[i]);
        }
        if (m_interactors[i]) {
            m_sphereController->AddInteractor(m_interactors[i]);
        }
    }

    // Connect before SetPosition so the first position change routes through
    // SliceController::OnSphereMoved → RenderAll() → riv->Render(), which calls
    // NeedToRenderOn() + BuildRepresentation() + UpdateDisplayExtent().
    // Without this ordering, SetPosition triggers SphereController::RenderAll()
    // (renderWindow->Render()) before any riv->Render() call, leaving the
    // image actor with a stale DisplayExtent and a blank viewport.
    QObject::connect(
        m_sphereController.get(), &SphereController::SphereMoved,
        m_sliceController.get(), &SliceController::OnSphereUpdated);

    // we should get position from image data bounds if it exists, otherwise default to origin.
    if (m_dicomLoaded) {
        // update sphere to get centered on the new image data if it exists.
        double bounds[6];
        m_imageData->GetBounds(bounds);
        const double centerX = (bounds[0] + bounds[1]) * 0.5;
        const double centerY = (bounds[2] + bounds[3]) * 0.5;
        const double centerZ = (bounds[4] + bounds[5]) * 0.5;
        qDebug() << "Setting initial sphere position to image center:" << centerX << centerY << centerZ;
        m_sphereController->SetPosition({centerX, centerY, centerZ});
    }

    // SetRadius/SetColor call SphereController::RenderAll() → renderWindow->Render().
    // That is safe here because SetPosition above already triggered riv->Render()
    // via the signal chain, so the reslice pipeline state is correctly initialised.
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
    // Always prefer riv->Render() paths; fall back to raw window render only
    // before SliceController (and therefore vtkResliceImageViewer) is set up.
    // if (m_sliceController) {
    //     m_sliceController->RenderAll();
    //     // return;
    // }
    for (auto& view : m_vtkWidgets) {
        if (view && view->renderWindow()) {
            view->renderWindow()->Render();
        }
    }
}

void MultiWindowController::_SetImageData(vtkImageData* imageData) {
    if (!m_initialized || !imageData) {
        return;
    }
    m_imageData = vtkSmartPointer<vtkImageData>::New();
    m_imageData->ShallowCopy(imageData);

    // we will initialize adapters on first image load, so that we can pass the image data to them.
    if (!m_dicomLoaded) {
        _SetupPipeline(imageData);
        m_dicomLoaded = true;
    } else {
        if (m_sliceController)
            m_sliceController->SetImageData(imageData);
    }

    if (m_sphereController) {
        // update sphere to get centered on the new image data if it exists.
        double bounds[6];
        imageData->GetBounds(bounds);
        const double centerX = (bounds[0] + bounds[1]) * 0.5;
        const double centerY = (bounds[2] + bounds[3]) * 0.5;
        const double centerZ = (bounds[4] + bounds[5]) * 0.5;
        m_sphereController->SetPosition({centerX, centerY, centerZ});
    }

    _Render();
    emit StatusChanged(tr("DICOM image loaded successfully into MultiWindow."));
}

void MultiWindowController::_SetupPipeline(vtkImageData* imageData) {
    if (!imageData)
        return;

    if (!m_sliceController) {
        m_sliceController = std::make_unique<SliceController>();

        m_sliceController->Initialize(m_vtkWidgets);

        if (m_sphereController) {
            QObject::connect(
                m_sphereController.get(), &SphereController::SphereMoved,
                m_sliceController.get(), &SliceController::OnSphereUpdated);
        }
    }

    m_sliceController->SetImageData(imageData);
}

void MultiWindowController::_ResetCameraClippingRange() {
    for (auto renderWindow : m_renderWindows) {
        if (renderWindow) {
            auto* renderer = renderWindow->GetRenderers()->GetFirstRenderer();
            if (renderer) {
                renderer->ResetCameraClippingRange();
            }
        }
    }
}
}  // namespace controllers
