#include "XQVtkViewport/RenderStatsOverlay.hpp"

#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <cstdio>
#include <string>

namespace qvv {

RenderStatsOverlay::RenderStatsOverlay(vtkRenderer* renderer) {
    m_textActor = vtkSmartPointer<vtkTextActor>::New();

    auto* prop = m_textActor->GetTextProperty();
    prop->SetFontSize(12);
    prop->SetColor(0.9, 0.9, 0.2);
    prop->SetBackgroundColor(0.0, 0.0, 0.0);
    prop->SetBackgroundOpacity(0.5);
    prop->SetJustificationToRight();

    // Normalized viewport coordinates: top-right corner
    m_textActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    m_textActor->SetPosition(0.98, 0.98);
    m_textActor->GetTextProperty()->SetVerticalJustificationToTop();

    m_textActor->SetVisibility(m_visible ? 1 : 0);

    if (renderer)
        renderer->AddActor(m_textActor);
}

RenderStatsOverlay::~RenderStatsOverlay() = default;

void RenderStatsOverlay::SetVisible(bool visible) {
    m_visible = visible;
    m_textActor->SetVisibility(visible ? 1 : 0);
}

bool RenderStatsOverlay::IsVisible() const {
    return m_visible;
}

void RenderStatsOverlay::Update(const std::string& label, const RenderStats& stats) {
    char buf[128];
    std::snprintf(buf, sizeof(buf),
                  "%s\nFPS: %.1f\nFrame: %.2f ms\nRender: %.2f ms",
                  label.c_str(),
                  stats.AverageFps(),
                  stats.frameTimeMs,
                  stats.renderTimeMs);
    m_textActor->SetInput(buf);
}

}  // namespace qvv
