#ifndef RESLICEIMAGEVIEWERINTERACTORSTYLE_HPP
#define RESLICEIMAGEVIEWERINTERACTORSTYLE_HPP

#include <vtkCommand.h>
#include <vtkInteractorObserver.h>
#include <vtkResliceImageViewer.h>
#include <vtkSmartPointer.h>

#include <unordered_map>
#include <vector>

namespace render {
class RenderScheduler;
}

namespace controllers {

/**
 * @brief Synchronises window/level settings across all vtkResliceImageViewer
 *        instances in multi-window mode.
 *
 * Registered as a vtkCommand observer on each viewer's interactor style.
 * When a WindowLevelEvent fires on any style, Execute() propagates the new
 * colour window and level to every other viewer so all slices stay visually
 * consistent.
 */
class ResliceImageViewerInteractorStyle : public vtkCommand {
  public:
    static ResliceImageViewerInteractorStyle* New() {
        return new ResliceImageViewerInteractorStyle();
    }

    /**
     * @brief Dispatches incoming VTK events to the appropriate handler.
     *
     * Handles vtkCommand::WindowLevelEvent and
     * vtkResliceCursorWidget::WindowLevelEvent.  The source viewer is resolved
     * either by a direct SafeDownCast to vtkResliceImageViewer or via the
     * @p caller-to-viewer map built by RegisterInteractorStyle().
     */
    void Execute(vtkObject* caller, unsigned long eventId, void* callData) override;

    /**
     * @brief Sets the full list of viewers that should be kept in sync.
     *
     * Should be called once after all viewers are created and before any
     * interaction events fire.
     */
    void SetViewers(const std::vector<vtkSmartPointer<vtkResliceImageViewer>>& viewers);

    /**
     * @brief Wires the render scheduler used to request renders after sync.
     *
     * Non-owning pointer; lifetime must exceed this object.  Should be set
     * before any interaction events fire.
     */
    void SetRenderScheduler(render::RenderScheduler* scheduler);

    /**
     * @brief Maps an interactor style to its associated viewer.
     *
     * Required so Execute() can resolve the source viewer when the caller is a
     * vtkInteractorObserver rather than a vtkResliceImageViewer directly.
     * @param style  Interactor style that fires window/level events.
     * @param viewer Viewer that owns @p style.
     */
    void RegisterInteractorStyle(vtkInteractorObserver* style, vtkResliceImageViewer* viewer);

  private:
    /** @brief Copies the colour window and level from @p source to all other viewers. */
    void _SyncWindowLevel(vtkResliceImageViewer* source);
    /** @brief Requests a render for every viewer window via the scheduler and then flushes. */
    void _Render();

    std::vector<vtkSmartPointer<vtkResliceImageViewer>> m_viewers;

    std::unordered_map<
        vtkInteractorObserver*,
        vtkResliceImageViewer*>
        m_styleToViewer;

    render::RenderScheduler* m_scheduler{nullptr};  // non-owning, owned by IViewController
};

}  // namespace controllers

#endif  // RESLICEIMAGEVIEWERINTERACTORSTYLE_HPP
