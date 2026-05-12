#ifndef RIVRENDERTARGET_HPP
#define RIVRENDERTARGET_HPP

#include "render/IRenderTarget.hpp"

class vtkResliceImageViewer;

namespace render {

// RIV-backed target — calls riv->Render() first so the reslice pipeline
// (UpdateDisplayExtent / NeedToRenderOn / BuildRepresentation) runs before
// the window composite, then calls renderWindow->Render() for the final frame.
// Without the riv->Render() step, image actors can show a blank viewport
// because the display extent is stale.
class RivRenderTarget final : public IRenderTarget {
  public:
    explicit RivRenderTarget(vtkResliceImageViewer* riv);

    void ExecuteRender() override;

  private:
    vtkResliceImageViewer* m_riv{nullptr};  // weak — owned by SliceController
};

}  // namespace render

#endif  // RIVRENDERTARGET_HPP
