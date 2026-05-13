#include "controllers/ResliceImageViewerInteractorStyle.hpp"

#include <vtkImageData.h>
#include <vtkResliceCursorWidget.h>
#include <vtkResliceImageViewer.h>

#include "render/RenderScheduler.hpp"

namespace controllers {

void ResliceImageViewerInteractorStyle::Execute(vtkObject* caller, unsigned long eventId, void* callData) {
    vtkResliceImageViewer* source = nullptr;

    if (auto* riv = vtkResliceImageViewer::SafeDownCast(caller)) {
        source = riv;
    } else if (auto* style = vtkInteractorObserver::SafeDownCast(caller)) {
        auto it = m_styleToViewer.find(style);

        if (it != m_styleToViewer.end())
            source = it->second;
    }

    if (!source)
        return;

    switch (eventId) {
        case vtkCommand::WindowLevelEvent:
        case vtkResliceCursorWidget::WindowLevelEvent: {
            _SyncWindowLevel(source);
            break;
        }

        default:
            break;
    }
}

void ResliceImageViewerInteractorStyle::SetViewers(const std::vector<vtkSmartPointer<vtkResliceImageViewer>>& viewers) {
    m_viewers = viewers;
}

void ResliceImageViewerInteractorStyle::SetRenderScheduler(render::RenderScheduler* scheduler) {
    m_scheduler = scheduler;
}

void ResliceImageViewerInteractorStyle::RegisterInteractorStyle(vtkInteractorObserver* style, vtkResliceImageViewer* viewer) {
    if (!style || !viewer)
        return;

    m_styleToViewer[style] = viewer;
}

void ResliceImageViewerInteractorStyle::_SyncWindowLevel(vtkResliceImageViewer* source) {
    if (!source) return;

    for (auto& viewer : m_viewers) {
        if (viewer && viewer != source) {
            viewer->SetColorWindow(source->GetColorWindow());
            viewer->SetColorLevel(source->GetColorLevel());
        }
    }

    _Render();
}

void ResliceImageViewerInteractorStyle::_Render() {
    if (!m_scheduler)
        return;

    for (const auto& riv : m_viewers) {
        if (riv && riv->GetRenderWindow())
            m_scheduler->RequestRender(riv->GetRenderWindow());
    }

    if (m_scheduler)
        m_scheduler->Flush();
}

}  // namespace controllers
