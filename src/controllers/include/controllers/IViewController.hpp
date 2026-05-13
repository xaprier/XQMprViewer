#ifndef IVIEWCONTROLLER_HPP
#define IVIEWCONTROLLER_HPP

#include <vtkImageData.h>
#include <vtkSmartPointer.h>

#include <memory>
#include <vector>

#include "controllers/IControllerBase.hpp"
#include "controllers/SliceController.hpp"
#include "controllers/SphereController.hpp"
#include "render/RenderScheduler.hpp"

class vtkRenderWindow;
class vtkRenderWindowInteractor;
class QVTKOpenGLNativeWidget;

namespace controllers {

/**
 * @brief Abstract base for view controllers that manage DICOM slice and sphere rendering.
 *
 * All concrete subclasses are registered in a shared instance list so that
 * sphere and image-data calls broadcast automatically to every active controller
 * without the UI needing to know how many instances exist.
 */
class IViewController : public IControllerBase {
    Q_OBJECT

  public:
    explicit IViewController(QObject* parent = nullptr) : IControllerBase(parent) {
        m_instances.push_back(this);
    }

    ~IViewController() {
        auto it = std::find(m_instances.begin(), m_instances.end(), this);
        if (it != m_instances.end()) {
            m_instances.erase(it);
        }
    }

    /**
     * @brief Initialises the controller with the supplied widgets. No-op if already initialised.
     */
    void Initialize(const std::vector<QVTKOpenGLNativeWidget*>& vtkWidgets) {
        if (!m_initialized) {
            _Initialize(vtkWidgets);
        }
    }

    /** @brief Returns true if Initialize() has been called successfully. */
    bool IsInitialized() const { return m_initialized; }

    /** @brief Adds a sphere actor to every registered controller instance. No-op if already added. */
    void AddSphere() {
        if (!m_sphereAdded) {
            for (auto* instance : m_instances) {
                instance->_AddSphere();
            }
        }
    }

    /** @brief Removes the sphere from every registered controller instance. */
    void RemoveSphere() {
        if (m_sphereAdded)
            for (auto* instance : m_instances)
                instance->_RemoveSphere();
    }

    /** @brief Returns true if a sphere actor is currently active. */
    bool IsSphereAdded() const { return m_sphereAdded; }

    /** @brief Toggles the sphere on or off across all instances. */
    void ToggleSphere() {
        if (m_sphereAdded) {
            RemoveSphere();
        } else {
            AddSphere();
        }
    }

    /** @brief Sets the sphere radius on all instances. No-op if @p radius is unchanged or <= 0. */
    void SetSphereRadius(double radius) {
        if (radius > 0 && radius != m_sphereRadius) {
            for (auto* instance : m_instances) {
                instance->_SetSphereRadius(radius);
            }
        }
    }

    /** @brief Returns the current sphere radius. */
    double GetSphereRadius() const { return m_sphereRadius; }

    /** @brief Sets the sphere RGB colour [0,1] on all instances. No-op if colour is unchanged or out of range. */
    void SetSphereColor(const std::array<double, 3> color) {
        if ((color[0] != m_sphereColor[0] || color[1] != m_sphereColor[1] || color[2] != m_sphereColor[2]) &&
            color[0] >= 0 && color[0] <= 1 && color[1] >= 0 && color[1] <= 1 && color[2] >= 0 && color[2] <= 1) {
            for (auto* instance : m_instances) {
                instance->_SetSphereColor(color);
            }
        }
    }

    /** @brief Returns the current sphere colour components. */
    void GetSphereColor(double& r, double& g, double& b) const {
        r = m_sphereColor[0];
        g = m_sphereColor[1];
        b = m_sphereColor[2];
    }

    /** @brief Returns the slice controller (valid after Initialize()). May be null for ViewportController until first image load. */
    SliceController* GetSliceController() const { return m_sliceController.get(); }

  Q_SIGNALS:
    /**
     * @brief Emitted once the vtkResliceImageViewer instances are created and ready.
     *
     * MultiWindowController emits this at the end of _Initialize().
     * ViewportController emits this inside _SetupPipeline() (first image load).
     * Connect to this signal to safely call GetSliceController()->GetViewers().
     */
    void ViewersReady();

  public slots:
    /** @brief Pushes @p data into every registered controller instance. No-op if @p data is null. */
    void SetImageData(vtkImageData* data) {
        if (data) {
            for (auto* instance : m_instances) {
                instance->_SetImageData(data);
            }
        }
    }

  protected:
    virtual void _Initialize(const std::vector<QVTKOpenGLNativeWidget*> vtkWidgets) = 0;
    virtual void _AddSphere() = 0;
    virtual void _RemoveSphere() = 0;
    virtual void _SetSphereRadius(double radius) = 0;
    virtual void _SetSphereColor(const std::array<double, 3> color) = 0;
    virtual void _Render() = 0;
    virtual void _SetImageData(vtkImageData* data) = 0;
    virtual void _SetupPipeline(vtkImageData* imageData) = 0;

    // ── Render scheduling ────────────────────────────────────────────────────
    // Each IViewController owns exactly one RenderScheduler. Constructed in
    // each concrete subclass constructor (after the full type is visible).
    // Subclasses register their render windows/RIVs in _Initialize(), then
    // call m_scheduler->RequestRender() / Flush() instead of Render() calls.
    std::unique_ptr<render::RenderScheduler> m_scheduler;

    bool m_initialized{false};
    bool m_sphereAdded{false};
    bool m_dicomLoaded{false};
    double m_sphereRadius{3.0};
    std::array<double, 3> m_sphereColor{1.0, 0.3, 0.3};
    vtkSmartPointer<vtkImageData> m_imageData;
    std::unique_ptr<SphereController> m_sphereController;
    std::unique_ptr<SliceController> m_sliceController;

    std::vector<QVTKOpenGLNativeWidget*> m_vtkWidgets;
    std::vector<vtkRenderWindow*> m_renderWindows;
    std::vector<vtkRenderWindowInteractor*> m_interactors;
    std::vector<vtkRenderer*> m_renderers;

  private:
    static std::vector<IViewController*> m_instances;
};

}  // namespace controllers

#endif  // IVIEWCONTROLLER_HPP
