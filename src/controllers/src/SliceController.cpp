#include "controllers/SliceController.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceImageViewer.h>

#include <algorithm>

namespace controllers {

SliceController::SliceController(QObject* parent)
    : IControllerBase(parent) {}

SliceController::~SliceController() = default;

void SliceController::Initialize(const std::vector<QVTKOpenGLNativeWidget*>& vtkWidgets) {
    m_vtkWidgets = vtkWidgets;

    SetupViewers();
}

void SliceController::SetupViewers() {
    static constexpr int kOrientations[3] = {
        vtkImageViewer2::SLICE_ORIENTATION_XY,
        vtkImageViewer2::SLICE_ORIENTATION_XZ,
        vtkImageViewer2::SLICE_ORIENTATION_YZ,
    };

    m_rivs.clear();
    m_rivs.reserve(3);

    bool viewportMode = (m_vtkWidgets.size() == 1);

    for (size_t i = 0; i < 3; ++i) {
        QVTKOpenGLNativeWidget* widget = nullptr;
        if (viewportMode) {
            widget = m_vtkWidgets[0];
        } else if (i < m_vtkWidgets.size()) {
            widget = m_vtkWidgets[i];
        }

        if (!widget ||
            !widget->renderWindow() ||
            !widget->interactor()) {
            emit StatusChanged(
                tr("Cannot setup viewers for view %1").arg(i));
            return;
        }

        auto riv = vtkSmartPointer<vtkResliceImageViewer>::New();

        riv->SetRenderWindow(widget->renderWindow());
        if (!viewportMode)
            riv->SetupInteractor(widget->interactor());

        riv->SetSliceOrientation(kOrientations[i]);

        m_rivs.push_back(riv);
    }
}

void SliceController::SetImageData(vtkImageData* image) {
    if (!image)
        return;

    m_image = vtkSmartPointer<vtkImageData>::New();
    m_image->ShallowCopy(image);

    SetupPipeline();

    for (const auto& riv : m_rivs) {
        if (!riv)
            continue;

        // riv->GetRenderer()->ResetCamera();
        // riv->GetRenderer()->ResetCameraClippingRange();

        riv->Render();
    }
}

void SliceController::GetRenderers(std::vector<vtkSmartPointer<vtkRenderer>>& outRenderers) const {
    outRenderers.clear();
    for (const auto& riv : m_rivs) {
        if (riv) {
            outRenderers.push_back(riv->GetRenderer());
        }
    }
}

void SliceController::SetupPipeline() {
    if (!m_image)
        return;

    static constexpr int kOrientations[3] = {
        vtkImageViewer2::SLICE_ORIENTATION_XY,
        vtkImageViewer2::SLICE_ORIENTATION_XZ,
        vtkImageViewer2::SLICE_ORIENTATION_YZ,
    };

    for (size_t i = 0; i < m_rivs.size(); ++i) {
        auto& riv = m_rivs[i];
        riv->SetInputData(m_image);
        riv->SetResliceModeToAxisAligned();
        riv->SetSliceOrientation(kOrientations[i]);  // enforce after mode change

        {
            // Calculate middle slice index based on image bounds, spacing, and origin.
            // kSliceAxis maps view index to the world-axis used for slicing:
            //   Axial  (XY, i=0) → Z axis (index 2)
            //   Coronal(XZ, i=1) → Y axis (index 1)
            //   Sagittal(YZ,i=2) → X axis (index 0)
            static constexpr int kSliceAxis[3] = {2, 1, 0};

            double spacing[3];
            double origin[3];
            double bounds[6];
            m_image->GetSpacing(spacing);
            m_image->GetOrigin(origin);
            m_image->GetBounds(bounds);

            const double worldPos[3] = {
                (bounds[0] + bounds[1]) * 0.5,
                (bounds[2] + bounds[3]) * 0.5,
                (bounds[4] + bounds[5]) * 0.5,
            };

            const int ax = kSliceAxis[i];
            const int sliceIdx = static_cast<int>((worldPos[ax] - origin[ax]) / spacing[ax] + 0.5);
            const int clamped = std::max(m_rivs[i]->GetSliceMin(), std::min(m_rivs[i]->GetSliceMax(), sliceIdx));

            qDebug() << "Setting slice for view" << i << "(axis" << ax << ") to middle slice index:" << clamped;
            m_rivs[i]->SetSlice(clamped);
        }

        m_rivs[i]->GetResliceCursorWidget()->InvokeEvent(vtkCommand::InteractionEvent);
        riv->SetColorLevel(500);
        riv->SetColorWindow(2000);
        riv->GetImageActor()->GetProperty()->SetColorWindow(400);
        riv->GetImageActor()->GetProperty()->SetColorLevel(127.5);
        riv->GetRenderer()->ResetCamera();
    }

    FitToView();
    RenderAll();
}

void SliceController::FitToView() {
    if (!m_image)
        return;

    double bounds[6];
    m_image->GetBounds(bounds);

    for (size_t i = 0; i < m_rivs.size(); ++i) {
        auto& riv = m_rivs[i];

        if (!riv)
            continue;

        auto* renderer = riv->GetRenderer();
        auto* camera = renderer->GetActiveCamera();
        auto* renderWindow = renderer->GetRenderWindow();

        if (!renderer || !camera || !renderWindow)
            continue;

        double width = 0.0;
        double height = 0.0;

        switch (i) {
            case 0:  // Axial (XY)
                width = bounds[1] - bounds[0];
                height = bounds[3] - bounds[2];
                break;

            case 1:  // Coronal (XZ)
                width = bounds[1] - bounds[0];
                height = bounds[5] - bounds[4];
                break;

            case 2:  // Sagittal (YZ)
                width = bounds[3] - bounds[2];
                height = bounds[5] - bounds[4];
                break;
        }

        if (width <= 0.0 || height <= 0.0)
            continue;

        const int* size = renderWindow->GetSize();

        // Use renderer's actual viewport fraction so this works correctly for
        // both multi-window (vp={0,0,1,1}) and single-window viewport mode
        // (vp={0,0,0.33,1} etc.) where each renderer covers only a sub-region.
        double vp[4];
        renderer->GetViewport(vp);
        const double vpWidth = vp[2] - vp[0];
        const double vpHeight = vp[3] - vp[1];

        const double viewportAspect =
            (size && size[0] > 0 && size[1] > 0 && vpHeight > 0.0)
                ? (static_cast<double>(size[0]) * vpWidth) / (static_cast<double>(size[1]) * vpHeight)
                : 1.0;

        const double imageAspect = width / height;

        double parallelScale = 0.0;

        if (imageAspect > viewportAspect) {
            parallelScale = width / (2.0 * viewportAspect);
        } else {
            parallelScale = height / 2.0;
        }

        // Small margin so image doesn't touch viewport edges.
        parallelScale *= 1.05;

        camera->ParallelProjectionOn();
        camera->SetParallelScale(parallelScale);

        renderer->ResetCameraClippingRange();
    }
}

void SliceController::OnSphereUpdated(const Vec3& worldPos) {
    if (!m_image || m_rivs.size() < 3)
        return;

    double spacing[3];
    double origin[3];
    m_image->GetSpacing(spacing);
    m_image->GetOrigin(origin);

    // kSliceAxis: view index → world-axis index used for slicing.
    // Axial(XY,i=0)→Z(2), Coronal(XZ,i=1)→Y(1), Sagittal(YZ,i=2)→X(0)
    static constexpr int kSliceAxis[3] = {2, 1, 0};

    for (int i = 0; i < 3; ++i) {
        if (!m_rivs[i])
            continue;

        const int ax = kSliceAxis[i];
        if (spacing[ax] == 0.0)
            continue;

        const int sliceIdx =
            static_cast<int>((worldPos[ax] - origin[ax]) / spacing[ax] + 0.5);

        const int clamped =
            std::max(m_rivs[i]->GetSliceMin(),
                     std::min(m_rivs[i]->GetSliceMax(), sliceIdx));

        m_rivs[i]->SetSlice(clamped);

    }

    RenderAll();
}

void SliceController::RenderAll() {
    qDebug() << "SliceController::RenderAll() called";
    for (auto& riv : m_rivs) {
        // ResetCameraClippingRange after Render so 3D actors (e.g. sphere)
        // are not clipped by the tight image-plane range set by vtkResliceImageViewer.
        riv->Render();
        riv->GetRenderer()->ResetCameraClippingRange();
        riv->GetRenderWindow()->Render();
    }
}

const std::vector<vtkSmartPointer<vtkResliceImageViewer>>&
SliceController::GetViewers() const {
    return m_rivs;
}

}  // namespace controllers
