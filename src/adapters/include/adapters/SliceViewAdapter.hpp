#ifndef XQVtkViewport_SLICEVIEWADAPTER_HPP
#define XQVtkViewport_SLICEVIEWADAPTER_HPP

#include <vtkSmartPointer.h>

#include <QObject>
#include <XQVtkViewport/RenderStats.hpp>
#include <memory>
#include <string>

class vtkImageData;
class vtkObject;
class vtkRenderer;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkResliceImageViewer;
class vtkCallbackCommand;
class vtkInteractorStyle;

namespace qvv {
class RenderStatsOverlay;
}

namespace adapters {

/**
 * @brief Wraps a vtkResliceImageViewer with timing callbacks and an optional stats overlay.
 *
 * Two operating modes:
 *
 * 1) Multi-window mode — BindCallbacks():
 *    The viewer manages its own renderer on the given render window.
 *
 * 2) Viewport mode — SetViewportMode():
 *    Operates on a shared render window; the viewer receives the shared window,
 *    interactor, and a pre-configured renderer from ViewportManager.
 *
 * In both modes EnableStatsOverlay(label) activates the top-right corner overlay.
 */
class SliceViewAdapter : public QObject {
    Q_OBJECT

  public:
    enum class Orientation {
        Axial,
        Coronal,
        Sagittal
    };

    explicit SliceViewAdapter(QObject* parent = nullptr);
    ~SliceViewAdapter() override;

    // ── Multi-window mode ───────────────────────────────────────────────────

    /**
     * @brief Attaches timing and release callbacks after external viewer setup.
     *
     * Expected call order:
     *   viewer->SetRenderWindow(rw);
     *   widget->setRenderWindow(adapter->GetRenderWindow());
     *   viewer->SetupInteractor(widget->interactor());
     *   adapter->BindCallbacks(rw, widget->interactor());
     */
    void BindCallbacks(vtkRenderWindow* renderWindow,
                       vtkRenderWindowInteractor* interactor);

    // ── Viewport mode ───────────────────────────────────────────────────────

    /**
     * @brief Configures the viewer for viewport mode on a shared render window.
     *
     * @param renderWindow  The shared render window from ViewportManager.
     * @param interactor    The widget's interactor.
     * @param renderer      The renderer obtained from ViewportManager::GetRenderer(i).
     */
    void SetViewportMode(vtkRenderWindow* renderWindow,
                         vtkRenderWindowInteractor* interactor,
                         vtkRenderer* renderer);

    // ── Stats overlay ───────────────────────────────────────────────────────

    /**
     * @brief Enables the top-right FPS/ms overlay.
     *
     * Must be called after BindCallbacks() or SetViewportMode() so that the
     * renderer is available.
     * @param label Viewport label, e.g. "Axial".
     */
    void EnableStatsOverlay(const std::string& label);

    /** @brief Disables the stats overlay. */
    void DisableStatsOverlay();

    /** @brief Returns the render statistics from the last frame. */
    [[nodiscard]] qvv::RenderStats GetStats() const;

  Q_SIGNALS:
    /**
     * @brief Emitted at the end of every frame (used for multi-window stats).
     * @param stats Current render statistics.
     */
    void frameRendered(const qvv::RenderStats& stats);

    // ── Common API ──────────────────────────────────────────────────────────

  public:
    /** @brief Binds the loaded vtkImageData to the viewer. */
    void SetImageData(vtkImageData* imageData);

    /** @brief Sets the slice orientation (Axial / Coronal / Sagittal). */
    void SetOrientation(Orientation orientation);

    /** @brief Updates the slice position in world coordinates. */
    void SetSlicePosition(double position);

    [[nodiscard]] Orientation GetOrientation() const;
    [[nodiscard]] vtkResliceImageViewer* GetViewer() const;
    [[nodiscard]] vtkRenderer* GetRenderer() const;
    [[nodiscard]] vtkRenderWindow* GetRenderWindow() const;

    /** @brief Adjusts the camera so the image fills the viewport exactly. */
    void FitToView();

    /** @brief Alias for FitToView(). */
    void ResetCamera();

  private:
    void AttachTimingCallbacks(vtkRenderWindow* renderWindow);

    static void OnRenderStart(vtkObject* caller, unsigned long, void* clientData, void*);
    static void OnRenderEnd(vtkObject* caller, unsigned long, void* clientData, void*);
    static void OnInteractorRelease(vtkObject* caller,
                                    unsigned long eventId,
                                    void* clientData,
                                    void* callData);
    static void OnWindowResize(vtkObject* caller, unsigned long, void* clientData, void*);

    vtkSmartPointer<vtkResliceImageViewer> m_viewer;
    vtkSmartPointer<vtkCallbackCommand> m_releaseCmd;
    vtkSmartPointer<vtkCallbackCommand> m_startCmd;
    vtkSmartPointer<vtkCallbackCommand> m_endCmd;
    vtkSmartPointer<vtkCallbackCommand> m_resizeCmd;
    vtkInteractorStyle* m_observedStyle{nullptr};
    Orientation m_orientation{Orientation::Axial};

    std::unique_ptr<qvv::RenderStatsOverlay> m_overlay;
    std::string m_overlayLabel;
    bool m_overlayEnabled{false};
    qvv::RenderStats m_stats;
    std::chrono::steady_clock::time_point m_lastFrameEnd;
    bool m_firstFrame{true};

    vtkRenderWindow* m_timedWindow{nullptr};  // tracks the window timing callbacks are attached to, preventing double-registration
};

}  // namespace adapters

#endif  // XQVtkViewport_SLICEVIEWADAPTER_HPP
