#ifndef VIEWPORTINTERACTORSTYLE_HPP
#define VIEWPORTINTERACTORSTYLE_HPP

#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkResliceImageViewer;

namespace render {
class RenderScheduler;
}

namespace controllers {

/**
 * @brief Routes mouse events to the correct vtkResliceImageViewer in viewport mode.
 *
 * When a single render window is split into multiple viewport regions, the
 * default vtkInteractorStyleImage always routes to the last viewer whose
 * SetupInteractor() was called. This style overrides the mouse handlers to
 * first identify which viewport the cursor is over, then delegates to the
 * appropriate viewer.
 */
class ViewportInteractorStyle : public vtkInteractorStyleImage {
  public:
    static ViewportInteractorStyle* New();
    vtkTypeMacro(ViewportInteractorStyle, vtkInteractorStyleImage);

    /**
     * @brief Supplies the ordered list of viewers whose viewport bounds are matched at runtime.
     */
    void SetViewers(const std::vector<vtkSmartPointer<vtkResliceImageViewer>>& viewers);

    /**
     * @brief Wire the scheduler so that interaction events request a render
     *        instead of calling riv->Render() directly. Should be set before
     *        any interaction events fire.
     */
    void SetScheduler(render::RenderScheduler* scheduler);

    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;

    /**
     * @brief Propagates the active viewer's colour window/level to all other viewers.
     *
     * Called by the base class during a left-button drag.  After delegating to
     * the superclass implementation, reads the updated colour window and level
     * from the active viewer's image actor and applies them to every other viewer
     * so all viewports stay visually consistent.
     */
    void WindowLevel() override;

  private:
    /** @brief Finds the viewer whose viewport contains the current cursor position. */
    bool _UpdateActiveViewer();

    /** @brief Requests a render for the active viewer's window via the scheduler (or directly if no scheduler). */
    void _RequestRender();

    render::RenderScheduler* m_scheduler{nullptr};  // non-owning, optional

    std::vector<vtkSmartPointer<vtkResliceImageViewer>> m_viewers;

    vtkResliceImageViewer* m_activeViewer = nullptr;
    vtkRenderer* m_activeRenderer = nullptr;
};
}  // namespace controllers

#endif  // VIEWPORTINTERACTORSTYLE_HPP
