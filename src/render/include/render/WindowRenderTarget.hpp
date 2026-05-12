#ifndef WINDOWRENDERTARGET_HPP
#define WINDOWRENDERTARGET_HPP

#include "render/IRenderTarget.hpp"

namespace render {

// Plain window target — used in multi-window mode and for the shared window
// in viewport mode. Calls renderWindow->Render() directly.
class WindowRenderTarget final : public IRenderTarget {
  public:
    explicit WindowRenderTarget(vtkRenderWindow* window);

    void ExecuteRender() override;
};

}  // namespace render

#endif  // WINDOWRENDERTARGET_HPP
