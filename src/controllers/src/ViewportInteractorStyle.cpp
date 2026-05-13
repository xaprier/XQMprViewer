#include "controllers/ViewportInteractorStyle.hpp"

#include <vtkImageActor.h>
#include <vtkImageProperty.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkResliceImageViewer.h>

#include "render/RenderScheduler.hpp"

vtkStandardNewMacro(controllers::ViewportInteractorStyle);

namespace controllers {

void ViewportInteractorStyle::SetViewers(const std::vector<vtkSmartPointer<vtkResliceImageViewer>>& viewers) {
    m_viewers = viewers;
}

void ViewportInteractorStyle::SetScheduler(render::RenderScheduler* scheduler) {
    m_scheduler = scheduler;
}

bool ViewportInteractorStyle::_UpdateActiveViewer() {
    if (!this->Interactor)
        return false;

    int* pos = this->Interactor->GetEventPosition();
    int* windowSize = this->Interactor->GetRenderWindow()->GetSize();

    const double normX = static_cast<double>(pos[0]) / windowSize[0];
    const double normY = static_cast<double>(pos[1]) / windowSize[1];

    for (auto& riv : m_viewers) {
        if (!riv)
            continue;

        auto* renderer = riv->GetRenderer();
        double vp[4];
        renderer->GetViewport(vp);

        if (normX >= vp[0] && normX <= vp[2] && normY >= vp[1] && normY <= vp[3]) {
            m_activeViewer = riv;
            m_activeRenderer = renderer;
            this->SetCurrentRenderer(renderer);
            return true;
        }
    }

    return false;
}

// Helper: request a render for the active viewer's window and flush.
// In viewport mode all viewers share the same window, so one RequestRender
// + Flush is sufficient regardless of which viewer triggered the update.
void ViewportInteractorStyle::_RequestRender() {
    if (!m_activeViewer)
        return;

    auto* rw = m_activeViewer->GetRenderWindow();
    if (!rw)
        return;

    if (m_scheduler) {
        m_scheduler->RequestRender(rw);
        m_scheduler->Flush();
    } else {
        m_activeViewer->Render();
    }
}

void ViewportInteractorStyle::OnLeftButtonDown() {
    int* pos = this->Interactor->GetEventPosition();
    this->FindPokedRenderer(pos[0], pos[1]);
    _UpdateActiveViewer();
    this->Superclass::OnLeftButtonDown();
}

void ViewportInteractorStyle::OnLeftButtonUp() {
    this->Superclass::OnLeftButtonUp();
}

void ViewportInteractorStyle::OnMouseMove() {
    int* pos = this->Interactor->GetEventPosition();
    this->FindPokedRenderer(pos[0], pos[1]);
    _UpdateActiveViewer();
    this->Superclass::OnMouseMove();
    _RequestRender();
}

void ViewportInteractorStyle::OnMouseWheelForward() {
    if (!_UpdateActiveViewer() || !m_activeViewer)
        return;

    m_activeViewer->SetSlice(m_activeViewer->GetSlice() + 1);
    _RequestRender();
}

void ViewportInteractorStyle::OnMouseWheelBackward() {
    if (!_UpdateActiveViewer() || !m_activeViewer)
        return;

    m_activeViewer->SetSlice(m_activeViewer->GetSlice() - 1);
    _RequestRender();
}

void ViewportInteractorStyle::WindowLevel() {
    if (!_UpdateActiveViewer() || !m_activeViewer)
        return;

    this->Superclass::WindowLevel();

    auto* prop = m_activeViewer->GetImageActor()->GetProperty();

    if (!prop)
        return;

    double wl = prop->GetColorWindow();
    double ll = prop->GetColorLevel();

    for (auto& v : m_viewers) {
        if (v && v != m_activeViewer) {
            auto* otherProp = v->GetImageActor()->GetProperty();
            if (!otherProp)
                continue;
            otherProp->SetColorWindow(wl);
            otherProp->SetColorLevel(ll);
        }
    }

    _RequestRender();
}

}  // namespace controllers
