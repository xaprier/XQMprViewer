#include "render/RivRenderTarget.hpp"

#include <vtkRenderWindow.h>
#include <vtkResliceImageViewer.h>

namespace render {
RivRenderTarget::RivRenderTarget(vtkResliceImageViewer* riv)
    : IRenderTarget(riv->GetRenderWindow()), m_riv(riv) {}

void RivRenderTarget::ExecuteRender() {
    if (m_riv)
        m_riv->Render();
    if (m_window)
        m_window->Render();
}

}  // namespace render
