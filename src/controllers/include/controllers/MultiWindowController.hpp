#ifndef XQVtkViewport_MULTIWINDOWCONTROLLER_HPP
#define XQVtkViewport_MULTIWINDOWCONTROLLER_HPP

#include "controllers/IViewController.hpp"

namespace controllers {

/**
 * @brief View controller that uses one render window per DICOM plane.
 *
 * Expects three QVTKOpenGLNativeWidget instances (Axial, Coronal, Sagittal),
 * each backed by its own vtkGenericOpenGLRenderWindow and interactor.
 */
class MultiWindowController : public IViewController {
    Q_OBJECT

  public:
    explicit MultiWindowController(QObject* parent = nullptr);
    ~MultiWindowController() override;

  private:
    void _Initialize(const std::vector<QVTKOpenGLNativeWidget*> vtkWidgets) override;
    void _AddSphere() override;
    void _RemoveSphere() override;
    void _SetSphereRadius(double radius) override;
    void _SetSphereColor(const std::array<double, 3> color) override;
    void _Render() override;
    void _SetImageData(vtkImageData* imageData) override;
    void _SetupPipeline(vtkImageData* imageData) override;

    void _ResetCameraClippingRange();
};

}  // namespace controllers

#endif  // XQVtkViewport_MULTIWINDOWCONTROLLER_HPP
