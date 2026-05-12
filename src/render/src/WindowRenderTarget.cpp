#include "render/WindowRenderTarget.hpp"

#include <vtkRenderWindow.h>

namespace render {

WindowRenderTarget::WindowRenderTarget(vtkRenderWindow* window)
    : IRenderTarget(window) {}

void WindowRenderTarget::ExecuteRender() {
    if (m_window)
        m_window->Render();
}

}  // namespace render
