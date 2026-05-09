#ifndef XQVtkViewport_VIEWPORTMANAGER_HPP
#define XQVtkViewport_VIEWPORTMANAGER_HPP

#include <QObject>
#include <memory>

#include "IViewportObserver.hpp"
#include "RenderStats.hpp"
#include "ViewportConfig.hpp"
#include "XQVtkViewport_export.hpp"

class vtkRenderWindow;
class vtkRenderer;

namespace qvv {

/**
 * @brief Manages multiple viewports inside a single vtkRenderWindow.
 *
 * Each viewport is defined by a normalised [0,1] rect and owns a vtkRenderer.
 * The implementation is hidden behind a pImpl to minimise header dependencies.
 *
 * @code
 * auto mgr = std::make_unique<qvv::ViewportManager>();
 * mgr->SetRenderWindow(myWindow);
 * int idx = mgr->AddViewport({{0.0, 0.0, 0.33, 1.0}, {0.1,0.1,0.1}, 0, "Axial"});
 * vtkRenderer* ren = mgr->GetRenderer(idx);
 * // attach pipeline actors to ren, then:
 * mgr->Render();
 * @endcode
 */
class QVV_EXPORT ViewportManager : public QObject {
    Q_OBJECT

  public:
    explicit ViewportManager(QObject* parent = nullptr);
    ~ViewportManager() override;

    ViewportManager(const ViewportManager&) = delete;
    ViewportManager& operator=(const ViewportManager&) = delete;

    // ── RenderWindow ─────────────────────────────────────────────────────────

    /**
     * @brief Assigns an externally created vtkRenderWindow.
     *
     * If not called before the first AddViewport(), a window is created
     * automatically. The manager does not take ownership of @p window.
     */
    void SetRenderWindow(vtkRenderWindow* window);

    /**
     * @brief Returns the managed vtkRenderWindow.
     * @return nullptr if no viewport has been added yet.
     */
    [[nodiscard]] vtkRenderWindow* GetRenderWindow() const;

    // ── Viewport CRUD ─────────────────────────────────────────────────────────

    /**
     * @brief Adds a new viewport and configures its renderer.
     * @param config Position, background colour and layer configuration.
     * @return Index of the newly added viewport.
     */
    int AddViewport(const ViewportConfig& config);

    /**
     * @brief Removes the viewport at @p index.
     * @param index Value returned by AddViewport().
     */
    void RemoveViewport(int index);

    /**
     * @brief Replaces the configuration of an existing viewport.
     * @param index  Target viewport index.
     * @param config New configuration to apply.
     */
    void SetViewportConfig(int index, const ViewportConfig& config);

    /**
     * @brief Returns the current configuration of a viewport.
     * @param index Viewport index to query.
     */
    [[nodiscard]] ViewportConfig GetViewportConfig(int index) const;

    /**
     * @brief Returns the total number of managed viewports.
     */
    [[nodiscard]] int GetViewportCount() const;

    // ── Renderer access ───────────────────────────────────────────────────────

    /**
     * @brief Returns the vtkRenderer associated with @p index.
     *
     * The caller may add actors and pipeline objects to this renderer. Ownership
     * remains with the manager; the caller must not delete it.
     */
    [[nodiscard]] vtkRenderer* GetRenderer(int index) const;

    // ── Stats overlay ─────────────────────────────────────────────────────────

    /**
     * @brief Shows or hides the per-viewport render statistics overlay.
     * @param enable true to show, false to hide.
     */
    void EnableStatsOverlay(bool enable);

    /**
     * @brief Returns whether the stats overlay is currently enabled.
     */
    [[nodiscard]] bool IsStatsOverlayEnabled() const;

    /**
     * @brief Returns the last-frame render statistics for @p index.
     */
    [[nodiscard]] RenderStats GetStats(int index) const;

    // ── Observer pattern ──────────────────────────────────────────────────────

    /**
     * @brief Registers an event observer. Adding the same observer twice is a no-op.
     * @param observer Lifetime is managed by the caller.
     */
    void AddObserver(IViewportObserver* observer);

    /**
     * @brief Unregisters a previously added observer.
     */
    void RemoveObserver(IViewportObserver* observer);

    // ── Render ────────────────────────────────────────────────────────────────

    /**
     * @brief Renders all viewports in a single pass.
     */
    void Render();

    /**
     * @brief Resets the camera of every managed renderer.
     */
    void ResetAllCameras();

  Q_SIGNALS:
    /**
     * @brief Emitted after each frame is rendered.
     * @param index       Viewport index that was rendered.
     * @param frameTimeMs Duration of this frame in milliseconds.
     */
    void frameRendered(int index, double frameTimeMs);

  private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace qvv

#endif  // XQVtkViewport_VIEWPORTMANAGER_HPP
