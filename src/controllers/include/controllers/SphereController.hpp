#ifndef XQVtkViewport_SPHERECONTROLLER_HPP
#define XQVtkViewport_SPHERECONTROLLER_HPP

#include <vtkSmartPointer.h>

#include <array>
#include <vector>

#include "controllers/IControllerBase.hpp"

class vtkActor;
class vtkCallbackCommand;
class vtkCellPicker;
class vtkObject;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkRenderWindow;
class vtkSphereSource;

namespace controllers {

/**
 * @brief Draggable sphere actor that works in both viewport and multi-window modes.
 *
 * Usage:
 *   1. AddRenderer(renderer, plane)  — call once per viewport renderer.
 *   2. AddInteractor(interactor)     — call once per interactor (1 in viewport mode, 3 in multi-window).
 *   3. SetPosition / SetRadius / SetColor as needed.
 *   4. Cleanup() before removing the sphere from the scene.
 */
class SphereController : public IControllerBase {
    Q_OBJECT

  public:
    enum class DragPlane { Axial,
                           Coronal,
                           Sagittal };
    using Vec3 = std::array<double, 3>;

    explicit SphereController(QObject* parent = nullptr);
    ~SphereController() override;

    /** @brief Registers a renderer and the slice plane it represents. */
    void AddRenderer(vtkSmartPointer<vtkRenderer> renderer, DragPlane plane);

    /** @brief Registers an interactor and attaches mouse-event observers. */
    void AddInteractor(vtkSmartPointer<vtkRenderWindowInteractor> interactor);

    /** @brief Removes the actor from all renderers and detaches all observers. */
    void Cleanup();

    void SetPosition(const Vec3& pos);
    void SetRadius(double radius);
    void SetColor(const Vec3& rgb);

    [[nodiscard]] double GetRadius() const;
    [[nodiscard]] Vec3 GetPosition() const;
    [[nodiscard]] Vec3 GetColor() const;
    [[nodiscard]] bool IsDragging() const;

  Q_SIGNALS:
    void SphereMoved(const Vec3& worldPos);

  private:
    struct RendererEntry {
        vtkSmartPointer<vtkRenderer> renderer;
        DragPlane plane;
    };

    DragPlane PlaneFor(vtkRenderer* renderer) const;
    void RenderAll();

    static void OnLeftButtonDown(vtkObject*, unsigned long, void* clientData, void*);
    static void OnMouseMove(vtkObject*, unsigned long, void* clientData, void*);
    static void OnLeftButtonUp(vtkObject*, unsigned long, void* clientData, void*);

    vtkSmartPointer<vtkSphereSource> m_sphereSource;
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkCellPicker> m_picker;
    vtkSmartPointer<vtkCallbackCommand> m_leftDownCmd;
    vtkSmartPointer<vtkCallbackCommand> m_mouseMoveCmd;
    vtkSmartPointer<vtkCallbackCommand> m_leftUpCmd;

    std::vector<RendererEntry> m_rendererEntries;
    std::vector<vtkSmartPointer<vtkRenderWindowInteractor>> m_interactors;

    bool m_isDragging{false};
    vtkRenderer* m_activeRenderer{nullptr};
    DragPlane m_activePlane{DragPlane::Axial};
    double m_dragOffset[3]{0.0, 0.0, 0.0};
};

}  // namespace controllers

#endif  // XQVtkViewport_SPHERECONTROLLER_HPP
