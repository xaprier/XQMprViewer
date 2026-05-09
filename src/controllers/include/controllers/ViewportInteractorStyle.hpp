#ifndef VIEWPORTINTERACTORSTYLE_HPP
#define VIEWPORTINTERACTORSTYLE_HPP

#include <vtkInteractorStyleImage.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkResliceImageViewer;

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

    void OnMouseMove() override;
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMouseWheelForward() override;
    void OnMouseWheelBackward() override;

  private:
    /** @brief Finds the viewer whose viewport contains the current cursor position. */
    bool _UpdateActiveViewer();

    std::vector<vtkSmartPointer<vtkResliceImageViewer>> m_viewers;

    vtkResliceImageViewer* m_activeViewer = nullptr;
    vtkRenderer* m_activeRenderer = nullptr;

    bool m_windowLeveling = false;
};
}  // namespace controllers

#endif  // VIEWPORTINTERACTORSTYLE_HPP
