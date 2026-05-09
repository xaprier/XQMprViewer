#include "controllers/ViewportInteractorStyle.hpp"

#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkResliceImageViewer.h>

vtkStandardNewMacro(controllers::ViewportInteractorStyle);

namespace controllers {

void ViewportInteractorStyle::SetViewers(const std::vector<vtkSmartPointer<vtkResliceImageViewer>>& viewers) {
    m_viewers = viewers;
}

bool ViewportInteractorStyle::_UpdateActiveViewer() {
    if (!this->Interactor)
        return false;

    int* pos = this->Interactor->GetEventPosition();

    int* windowSize = this->Interactor->GetRenderWindow()->GetSize();

    const double normX =
        static_cast<double>(pos[0]) / windowSize[0];

    const double normY =
        static_cast<double>(pos[1]) / windowSize[1];

    for (auto& riv : m_viewers) {
        if (!riv)
            continue;

        auto* renderer = riv->GetRenderer();

        double vp[4];
        renderer->GetViewport(vp);

        if (normX >= vp[0] &&
            normX <= vp[2] &&
            normY >= vp[1] &&
            normY <= vp[3]) {
            m_activeViewer = riv;
            m_activeRenderer = renderer;

            this->SetCurrentRenderer(renderer);

            return true;
        }
    }

    return false;
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

    if (m_activeViewer) {
        m_activeViewer->Render();
    }
}

void ViewportInteractorStyle::OnMouseWheelForward() {
    if (!_UpdateActiveViewer() || !m_activeViewer)
        return;

    int slice = m_activeViewer->GetSlice();

    m_activeViewer->SetSlice(slice + 1);

    m_activeViewer->Render();
}

void ViewportInteractorStyle::OnMouseWheelBackward() {
    if (!_UpdateActiveViewer() || !m_activeViewer)
        return;

    int slice = m_activeViewer->GetSlice();

    m_activeViewer->SetSlice(slice - 1);

    m_activeViewer->Render();
}
}  // namespace controllers
