#ifndef RENDER_IRENDERTARGET_HPP
#define RENDER_IRENDERTARGET_HPP

class vtkRenderWindow;

namespace render {

/**
 * @brief Abstract render target managed by RenderScheduler.
 *
 * Each concrete subclass wraps one vtkRenderWindow and knows how to:
 *   - report its identity (GetRenderWindow)
 *   - execute one actual render (ExecuteRender)
 *
 * Subclasses exist for:
 *   - WindowRenderTarget   — plain vtkRenderWindow (multi-window mode)
 *   - RivRenderTarget      — vtkResliceImageViewer-backed window, which
 *                            must call riv->Render() before window->Render()
 *                            to keep the reslice pipeline consistent
 *
 * The scheduler deduplicates by vtkRenderWindow pointer so that no matter how
 * many RequestRender() calls arrive for the same window in one flush cycle,
 * ExecuteRender() is called exactly once.
 */
class IRenderTarget {
  public:
    explicit IRenderTarget(vtkRenderWindow* window);
    virtual ~IRenderTarget() = default;

    vtkRenderWindow* GetRenderWindow() const;

    /**
     * @brief Perform the actual render for this target.
     *
     * Called once per flush cycle when this target is dirty.
     * Implementations must call renderWindow->Render() (not renderer->Render()).
     */
    virtual void ExecuteRender() = 0;

  protected:
    vtkRenderWindow* m_window{nullptr};
};

}  // namespace render

#endif  // RENDER_IRENDERTARGET_HPP
