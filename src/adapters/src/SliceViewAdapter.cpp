#include "adapters/SliceViewAdapter.hpp"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyle.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkResliceImageViewer.h>

#include <XQVtkViewport/RenderStatsOverlay.hpp>
#include <chrono>

namespace adapters {

SliceViewAdapter::SliceViewAdapter(QObject* parent)
    : QObject(parent) {
    m_viewer = vtkSmartPointer<vtkResliceImageViewer>::New();

    m_releaseCmd = vtkSmartPointer<vtkCallbackCommand>::New();
    m_releaseCmd->SetCallback(&SliceViewAdapter::OnInteractorRelease);
    m_releaseCmd->SetClientData(this);

    m_startCmd = vtkSmartPointer<vtkCallbackCommand>::New();
    m_startCmd->SetCallback(&SliceViewAdapter::OnRenderStart);
    m_startCmd->SetClientData(this);

    m_endCmd = vtkSmartPointer<vtkCallbackCommand>::New();
    m_endCmd->SetCallback(&SliceViewAdapter::OnRenderEnd);
    m_endCmd->SetClientData(this);

    m_resizeCmd = vtkSmartPointer<vtkCallbackCommand>::New();
    m_resizeCmd->SetCallback(&SliceViewAdapter::OnWindowResize);
    m_resizeCmd->SetClientData(this);
}

SliceViewAdapter::~SliceViewAdapter() = default;

// ── MultiWindow modu ──────────────────────────────────────────────────────────

void SliceViewAdapter::BindCallbacks(vtkRenderWindow* renderWindow,
                                     vtkRenderWindowInteractor* interactor) {
    AttachTimingCallbacks(renderWindow);

    if (!interactor)
        return;

    interactor->AddObserver(vtkCommand::LeftButtonReleaseEvent, m_releaseCmd, 2.0f);
    interactor->AddObserver(vtkCommand::RightButtonReleaseEvent, m_releaseCmd, 2.0f);
    interactor->AddObserver(vtkCommand::MiddleButtonReleaseEvent, m_releaseCmd, 2.0f);
    interactor->AddObserver(vtkCommand::LeaveEvent, m_releaseCmd, 2.0f);
    interactor->AddObserver(vtkCommand::EndInteractionEvent, m_releaseCmd, 2.0f);

    if (auto* style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle())) {
        if (style != m_observedStyle) {
            m_observedStyle = style;
            style->AddObserver(vtkCommand::LeftButtonReleaseEvent, m_releaseCmd, 2.0f);
            style->AddObserver(vtkCommand::RightButtonReleaseEvent, m_releaseCmd, 2.0f);
            style->AddObserver(vtkCommand::MiddleButtonReleaseEvent, m_releaseCmd, 2.0f);
            style->AddObserver(vtkCommand::LeaveEvent, m_releaseCmd, 2.0f);
            style->AddObserver(vtkCommand::EndInteractionEvent, m_releaseCmd, 2.0f);
        }
    }
}

// ── Viewport modu ─────────────────────────────────────────────────────────────

void SliceViewAdapter::SetViewportMode(vtkRenderWindow* renderWindow,
                                       vtkRenderWindowInteractor* interactor,
                                       vtkRenderer* renderer) {
    // SetRenderer önce çağrılmalı: vtkImageViewer2::SetRenderWindow() → InstallPipeline()
    // renderer null iken çalışırsa yeni bir otomatik renderer oluşturur ve pipeline yanlış
    // renderer ile kurulur. SetRenderer önce çağrılırsa InstallPipeline bizim renderer'ımızı kullanır.
    m_viewer->SetRenderer(renderer);
    m_viewer->SetRenderWindow(renderWindow);
    m_viewer->SetupInteractor(interactor);
    // Viewport modunda timing ViewportManager üzerinden yapılıyor.
    // Resize callback sadece kendi m_resizeCmd'i ile bağla — timing callback'leri ekleme.
    if (renderWindow) {
        renderWindow->AddObserver(vtkCommand::WindowResizeEvent, m_resizeCmd);
        m_timedWindow = renderWindow;
    }
}

// ── Stats overlay ─────────────────────────────────────────────────────────────

void SliceViewAdapter::EnableStatsOverlay(const std::string& label) {
    m_overlayLabel = label;
    m_overlayEnabled = true;
    // Renderer hazırsa hemen oluştur; değilse SetImageData'da lazy-init olur
    auto* renderer = m_viewer->GetRenderer();
    if (renderer && !m_overlay) {
        m_overlay = std::make_unique<qvv::RenderStatsOverlay>(renderer);
        m_overlay->SetVisible(true);
    }
}

void SliceViewAdapter::DisableStatsOverlay() {
    m_overlay.reset();
}

qvv::RenderStats SliceViewAdapter::GetStats() const {
    return m_stats;
}

// ── Ortak API ─────────────────────────────────────────────────────────────────

void SliceViewAdapter::SetImageData(vtkImageData* imageData) {
    if (!imageData)
        return;

    // FourPaneViewer pattern: önce input, sonra orientation.
    // SetSliceOrientation input mevcut olduğunda UpdateOrientation + ResetCamera
    // çalıştırır → kamera doğru yönde başlar.
    m_viewer->SetResliceModeToAxisAligned();
    m_viewer->SetInputData(imageData);

    switch (m_orientation) {
        case Orientation::Axial:
            m_viewer->SetSliceOrientation(vtkImageViewer2::SLICE_ORIENTATION_XY);
            break;
        case Orientation::Coronal:
            m_viewer->SetSliceOrientation(vtkImageViewer2::SLICE_ORIENTATION_XZ);
            break;
        case Orientation::Sagittal:
            m_viewer->SetSliceOrientation(vtkImageViewer2::SLICE_ORIENTATION_YZ);
            break;
    }

    const int minSlice = m_viewer->GetSliceMin();
    const int maxSlice = m_viewer->GetSliceMax();
    m_viewer->SetSlice((minSlice + maxSlice) / 2);

    constexpr double kColorWindow = 2000.0;
    constexpr double kColorLevel = 500.0;
    m_viewer->SetColorWindow(kColorWindow);
    m_viewer->SetColorLevel(kColorLevel);
    if (auto* actor = m_viewer->GetImageActor()) {
        actor->GetProperty()->SetColorWindow(kColorWindow);
        actor->GetProperty()->SetColorLevel(kColorLevel);
    }

    if (m_overlayEnabled && !m_overlay) {
        if (auto* renderer = m_viewer->GetRenderer()) {
            m_overlay = std::make_unique<qvv::RenderStatsOverlay>(renderer);
            m_overlay->SetVisible(true);
        }
    }

    FitToView();
}

void SliceViewAdapter::SetOrientation(Orientation orientation) {
    m_orientation = orientation;
    switch (orientation) {
        case Orientation::Axial:
            m_viewer->SetSliceOrientationToXY();
            break;
        case Orientation::Coronal:
            m_viewer->SetSliceOrientationToXZ();
            break;
        case Orientation::Sagittal:
            m_viewer->SetSliceOrientationToYZ();
            break;
    }
}

void SliceViewAdapter::SetSlicePosition(double position) {
    if (!m_viewer->GetInput())
        return;

    double bounds[6];
    m_viewer->GetInput()->GetBounds(bounds);
    double spacing[3];
    m_viewer->GetInput()->GetSpacing(spacing);

    double sliceSpacing = 1.0;
    int sliceIndex = 0;

    switch (m_orientation) {
        case Orientation::Axial:
            sliceSpacing = spacing[2];
            sliceIndex = static_cast<int>((position - bounds[4]) / sliceSpacing + 0.5);
            break;
        case Orientation::Coronal:
            sliceSpacing = spacing[1];
            sliceIndex = static_cast<int>((position - bounds[2]) / sliceSpacing + 0.5);
            break;
        case Orientation::Sagittal:
            sliceSpacing = spacing[0];
            sliceIndex = static_cast<int>((position - bounds[0]) / sliceSpacing + 0.5);
            break;
    }

    m_viewer->SetSlice(sliceIndex);
}

void SliceViewAdapter::FitToView() {
    auto* renderer = m_viewer->GetRenderer();
    if (!renderer || !m_viewer->GetInput())
        return;

    auto* cam = renderer->GetActiveCamera();
    if (!cam)
        return;

    double bounds[6];
    m_viewer->GetInput()->GetBounds(bounds);

    // Image merkezi
    const double cx = (bounds[0] + bounds[1]) * 0.5;
    const double cy = (bounds[2] + bounds[3]) * 0.5;
    const double cz = (bounds[4] + bounds[5]) * 0.5;

    // Image boyutları ve orientation'a göre kamera yönü
    double imgW = 0.0;
    double imgH = 0.0;
    double posX = 0.0, posY = 0.0, posZ = 0.0;
    double upX = 0.0, upY = 0.0, upZ = 0.0;

    switch (m_orientation) {
        case Orientation::Axial:
            // XY düzlemi: Z ekseninden bakış
            imgW = bounds[1] - bounds[0];
            imgH = bounds[3] - bounds[2];
            posX = cx;
            posY = cy;
            posZ = bounds[5] + 500.0;
            upX = 0.0;
            upY = 1.0;
            upZ = 0.0;
            break;
        case Orientation::Coronal:
            // XZ düzlemi: Y ekseninden bakış (negatif Y yönünden)
            imgW = bounds[1] - bounds[0];
            imgH = bounds[5] - bounds[4];
            posX = cx;
            posY = bounds[2] - 500.0;
            posZ = cz;
            upX = 0.0;
            upY = 0.0;
            upZ = 1.0;
            break;
        case Orientation::Sagittal:
            // YZ düzlemi: X ekseninden bakış
            imgW = bounds[3] - bounds[2];
            imgH = bounds[5] - bounds[4];
            posX = bounds[1] + 500.0;
            posY = cy;
            posZ = cz;
            upX = 0.0;
            upY = 0.0;
            upZ = 1.0;
            break;
    }

    if (imgW <= 0.0 || imgH <= 0.0)
        return;

    cam->ParallelProjectionOn();
    cam->SetFocalPoint(cx, cy, cz);
    cam->SetPosition(posX, posY, posZ);
    cam->SetViewUp(upX, upY, upZ);

    // Viewport piksel boyutunu hesapla
    double vp[4];
    renderer->GetViewport(vp);
    const double vpFracW = vp[2] - vp[0];
    const double vpFracH = vp[3] - vp[1];

    auto* win = renderer->GetRenderWindow();
    double vpPixW = 1.0;
    double vpPixH = 1.0;
    if (win) {
        const int* sz = win->GetSize();
        if (sz[0] > 0 && sz[1] > 0) {
            vpPixW = sz[0] * vpFracW;
            vpPixH = sz[1] * vpFracH;
        }
    }

    if (vpPixW <= 0.0 || vpPixH <= 0.0) {
        renderer->ResetCameraClippingRange();
        return;
    }

    const double vpAspect = vpPixW / vpPixH;
    const double imgAspect = imgW / imgH;

    double parallelScale = 0.0;
    if (imgAspect > vpAspect) {
        parallelScale = imgW / (2.0 * vpAspect);
    } else {
        parallelScale = imgH / 2.0;
    }

    cam->SetParallelScale(parallelScale);
    renderer->ResetCameraClippingRange();
}

void SliceViewAdapter::ResetCamera() {
    FitToView();
}

SliceViewAdapter::Orientation SliceViewAdapter::GetOrientation() const {
    return m_orientation;
}

vtkResliceImageViewer* SliceViewAdapter::GetViewer() const {
    return m_viewer;
}

vtkRenderer* SliceViewAdapter::GetRenderer() const {
    return m_viewer->GetRenderer();
}

vtkRenderWindow* SliceViewAdapter::GetRenderWindow() const {
    return m_viewer->GetRenderWindow();
}

// ── Private helpers ───────────────────────────────────────────────────────────

void SliceViewAdapter::AttachTimingCallbacks(vtkRenderWindow* renderWindow) {
    if (!renderWindow || renderWindow == m_timedWindow)
        return;
    m_timedWindow = renderWindow;
    renderWindow->AddObserver(vtkCommand::StartEvent, m_startCmd);
    renderWindow->AddObserver(vtkCommand::EndEvent, m_endCmd);
    // Window boyutlanınca FitToView uygula — VTK'nın kendi ResetCamera'sının
    // parallel scale'i ezmesini düzelt
    renderWindow->AddObserver(vtkCommand::WindowResizeEvent, m_resizeCmd);
}

// ── Callbacks ─────────────────────────────────────────────────────────────────

void SliceViewAdapter::OnRenderStart(vtkObject* /*caller*/, unsigned long, void* clientData, void*) {
    auto* self = static_cast<SliceViewAdapter*>(clientData);
    // renderTimeMs sıfırla; OnRenderEnd'de EndEvent - StartEvent farkı yazılacak
    self->m_stats.renderTimeMs = std::chrono::duration<double, std::milli>(
                                     std::chrono::steady_clock::now().time_since_epoch())
                                     .count();
}

void SliceViewAdapter::OnRenderEnd(vtkObject* /*caller*/, unsigned long, void* clientData, void*) {
    auto* self = static_cast<SliceViewAdapter*>(clientData);

    const auto now = std::chrono::steady_clock::now();

    // renderTimeMs: StartEvent'de kaydedilen epoch ms'den fark al
    const double endEpochMs = std::chrono::duration<double, std::milli>(
                                  now.time_since_epoch())
                                  .count();
    if (self->m_stats.renderTimeMs > 0.0)
        self->m_stats.renderTimeMs = endEpochMs - self->m_stats.renderTimeMs;

    double frameMs = 0.0;
    if (!self->m_firstFrame) {
        frameMs = std::chrono::duration<double, std::milli>(now - self->m_lastFrameEnd).count();
    }
    self->m_firstFrame = false;
    self->m_lastFrameEnd = now;
    self->m_stats.frameTimeMs = frameMs;
    self->m_stats.frameCount++;

    if (self->m_overlay)
        self->m_overlay->Update(self->m_overlayLabel, self->m_stats);

    emit self->frameRendered(self->m_stats);
}

void SliceViewAdapter::OnWindowResize(vtkObject* /*caller*/, unsigned long, void* clientData, void*) {
    auto* self = static_cast<SliceViewAdapter*>(clientData);
    if (!self || !self->m_viewer->GetInput())
        return;
    // Window yeniden boyutlandı — kamerayı görüntüye yeniden sığdır
    self->FitToView();
}

void SliceViewAdapter::OnInteractorRelease(vtkObject* caller,
                                           unsigned long eventId,
                                           void* clientData,
                                           void*) {
    auto* self = static_cast<SliceViewAdapter*>(clientData);
    if (!self)
        return;

    vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
    vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(caller);

    if (!interactor && style)
        interactor = style->GetInteractor();
    if (!interactor)
        return;
    if (!style)
        style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());
    if (!style || style->GetState() == VTKIS_NONE)
        return;

    if (eventId == vtkCommand::LeftButtonReleaseEvent) {
        style->OnLeftButtonUp();
        return;
    }
    if (eventId == vtkCommand::RightButtonReleaseEvent) {
        style->OnRightButtonUp();
        return;
    }
    if (eventId == vtkCommand::MiddleButtonReleaseEvent) {
        style->OnMiddleButtonUp();
        return;
    }
    style->OnLeftButtonUp();
    style->OnRightButtonUp();
    style->OnMiddleButtonUp();
}

}  // namespace adapters
