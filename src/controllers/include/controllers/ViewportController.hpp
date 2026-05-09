#ifndef XQVtkViewport_VIEWPORTCONTROLLER_HPP
#define XQVtkViewport_VIEWPORTCONTROLLER_HPP

#include <QObject>
#include <vector>

#include "controllers/IViewController.hpp"

namespace controllers {
class ViewportInteractorStyle;

/**
 * @brief View controller that displays three DICOM planes in a single render window.
 *
 * Implements the same IViewController interface as MultiWindowController.
 * Instead of three separate QVTKOpenGLNativeWidget instances, a single widget's
 * render window is split into three equal horizontal regions via qvv::ViewportLayout.
 */
class ViewportController : public IViewController {
    Q_OBJECT

  public:
    explicit ViewportController(QObject* parent = nullptr);
    ~ViewportController() override;

  private:
    void _Initialize(const std::vector<QVTKOpenGLNativeWidget*> vtkWidgets) override;
    void _AddSphere() override;
    void _RemoveSphere() override;
    void _SetSphereRadius(double radius) override;
    void _SetSphereColor(const std::array<double, 3> color) override;
    void _Render() override;
    void _SetImageData(vtkImageData* imageData) override;
    void _SetupPipeline(vtkImageData* imageData) override;

    /** @brief Assigns the three horizontal viewport regions to the SliceController's renderers. */
    void _ApplyViewportLayout();

    vtkSmartPointer<ViewportInteractorStyle> m_interactorStyle;
};

}  // namespace controllers

#endif  // XQVtkViewport_VIEWPORTCONTROLLER_HPP
