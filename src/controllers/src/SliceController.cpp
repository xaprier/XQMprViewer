#include "controllers/SliceController.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceImageViewer.h>

#include <algorithm>

#include "controllers/ResliceImageViewerInteractorStyle.hpp"
#include "render/RenderScheduler.hpp"

namespace controllers {

SliceController::SliceController(QObject* parent)
    : IControllerBase(parent) {}

SliceController::~SliceController() = default;

void SliceController::Initialize(const std::vector<QVTKOpenGLNativeWidget*>& vtkWidgets,
                                 render::RenderScheduler* scheduler) {
    m_vtkWidgets = vtkWidgets;
    m_scheduler = scheduler;
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
    if (!viewportMode)
        m_rivStyle = vtkSmartPointer<ResliceImageViewerInteractorStyle>::New();

    for (size_t i = 0; i < 3; ++i) {
        QVTKOpenGLNativeWidget* widget = nullptr;
        if (viewportMode) {
            widget = m_vtkWidgets[0];
        } else if (i < m_vtkWidgets.size()) {
            widget = m_vtkWidgets[i];
        }

        if (!widget || !widget->renderWindow() || !widget->interactor()) {
            emit StatusChanged(tr("Cannot setup viewers for view %1").arg(i));
            return;
        }

        auto riv = vtkSmartPointer<vtkResliceImageViewer>::New();

        riv->SetRenderWindow(widget->renderWindow());
        if (!viewportMode)
            riv->SetupInteractor(widget->interactor());

        auto* style = widget->interactor()->GetInteractorStyle();
        if (m_rivStyle && style) {  // style will be null in viewport mode since SetupInteractor() is not called
            style->AddObserver(
                vtkCommand::WindowLevelEvent,
                m_rivStyle);

            style->AddObserver(
                vtkCommand::StartWindowLevelEvent,
                m_rivStyle);

            style->AddObserver(
                vtkCommand::EndWindowLevelEvent,
                m_rivStyle);

            m_rivStyle->RegisterInteractorStyle(style, riv);
        }

        riv->SetSliceOrientation(kOrientations[i]);
        m_rivs.push_back(riv);

        // Register each RIV with the scheduler. In viewport mode all three RIVs
        // share the same vtkRenderWindow; the scheduler deduplicates by window so
        // only one ExecuteRender() fires per Flush(). The first registered RIV
        // target wins for that window — it calls riv->Render() then window->Render().
        // Subsequent RegisterRiv() calls for the same window are de-duplicated
        // (they return the existing handle), so only the first RIV's pipeline runs.
        // For viewport mode this is acceptable because all three RIVs share one
        // render window and vtkResliceImageViewer::Render() on any of them syncs
        // the shared pipeline.
        if (m_scheduler)
            m_scheduler->RegisterRiv(riv.Get());
    }

    if (m_rivStyle) {  // style will not be active in viewport mode
        m_rivStyle->SetViewers(m_rivs);
        m_rivStyle->SetRenderScheduler(m_scheduler);
    }
}

void SliceController::SetImageData(vtkImageData* image) {
    if (!image)
        return;

    m_image = vtkSmartPointer<vtkImageData>::New();
    m_image->ShallowCopy(image);

    SetupPipeline();

    RequestRenderAll();
}

void SliceController::GetRenderers(std::vector<vtkSmartPointer<vtkRenderer>>& outRenderers) const {
    outRenderers.clear();
    for (const auto& riv : m_rivs) {
        if (riv)
            outRenderers.push_back(riv->GetRenderer());
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
        riv->SetSliceOrientation(kOrientations[i]);

        {
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
            case 0:
                width = bounds[1] - bounds[0];
                height = bounds[3] - bounds[2];
                break;
            case 1:
                width = bounds[1] - bounds[0];
                height = bounds[5] - bounds[4];
                break;
            case 2:
                width = bounds[3] - bounds[2];
                height = bounds[5] - bounds[4];
                break;
        }

        if (width <= 0.0 || height <= 0.0)
            continue;

        const int* size = renderWindow->GetSize();

        double vp[4];
        renderer->GetViewport(vp);
        const double vpWidth = vp[2] - vp[0];
        const double vpHeight = vp[3] - vp[1];

        const double viewportAspect =
            (size && size[0] > 0 && size[1] > 0 && vpHeight > 0.0)
                ? (static_cast<double>(size[0]) * vpWidth) / (static_cast<double>(size[1]) * vpHeight)
                : 1.0;

        const double imageAspect = width / height;
        double parallelScale = (imageAspect > viewportAspect) ? width / (2.0 * viewportAspect) : height / 2.0;
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

    static constexpr int kSliceAxis[3] = {2, 1, 0};

    for (int i = 0; i < 3; ++i) {
        if (!m_rivs[i])
            continue;

        const int ax = kSliceAxis[i];
        if (spacing[ax] == 0.0)
            continue;

        const int sliceIdx = static_cast<int>((worldPos[ax] - origin[ax]) / spacing[ax] + 0.5);
        const int clamped = std::max(m_rivs[i]->GetSliceMin(), std::min(m_rivs[i]->GetSliceMax(), sliceIdx));
        m_rivs[i]->SetSlice(clamped);
    }

    RenderAll();
}

void SliceController::RequestRenderAll() {
    if (!m_scheduler)
        return;
    for (const auto& riv : m_rivs) {
        if (riv && riv->GetRenderWindow())
            m_scheduler->RequestRender(riv->GetRenderWindow());
    }
}

void SliceController::RenderAll() {
    RequestRenderAll();
    if (m_scheduler)
        m_scheduler->Flush();
}

render::RenderScheduler* SliceController::Scheduler() const {
    return m_scheduler;
}

const std::vector<vtkSmartPointer<vtkResliceImageViewer>>&
SliceController::GetViewers() const {
    return m_rivs;
}

}  // namespace controllers
