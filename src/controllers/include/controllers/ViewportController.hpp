#ifndef XQVtkViewport_VIEWPORTCONTROLLER_HPP
#define XQVtkViewport_VIEWPORTCONTROLLER_HPP

#include <QObject>
#include <vector>

#include <XQVtkViewport/ViewportConfig.hpp>

#include "controllers/IViewController.hpp"

namespace controllers {
class ViewportInteractorStyle;

/**
 * @brief Maps a viewport region to a slice-plane renderer index.
 *
 * planeIndex: 0=Axial, 1=Coronal, 2=Sagittal.
 */
struct ViewportPaneDesc {
    qvv::ViewportConfig viewport;
    int planeIndex{0};
};

/**
 * @brief View controller that displays three DICOM planes in a single render window.
 *
 * Implements the same IViewController interface as MultiWindowController.
 * Instead of three separate QVTKOpenGLNativeWidget instances, a single widget's
 * render window is split into three equal horizontal regions via qvv::ViewportLayout,
 * or any other runtime-selected layout via ApplyLayout().
 */
class ViewportController : public IViewController {
    Q_OBJECT

  public:
    explicit ViewportController(QObject* parent = nullptr);
    ~ViewportController() override = default;

    /**
     * @brief Applies a new viewport layout, updating renderer viewport rects.
     *
     * Safe to call before the pipeline is set up (stored and applied once the
     * pipeline initialises). After pipeline setup the change takes effect
     * immediately and a render is requested.
     */
    void ApplyLayout(const std::vector<ViewportPaneDesc>& panes);

  private:
    void _Initialize(const std::vector<QVTKOpenGLNativeWidget*> vtkWidgets) override;
    void _AddSphere() override;
    void _RemoveSphere() override;
    void _SetSphereRadius(double radius) override;
    void _SetSphereColor(const std::array<double, 3> color) override;
    void _Render() override;
    void _SetImageData(vtkImageData* imageData) override;
    void _SetupPipeline(vtkImageData* imageData) override;

    void _ApplyViewportLayout();
    void _ApplyPanes(const std::vector<ViewportPaneDesc>& panes);

    vtkSmartPointer<ViewportInteractorStyle> m_interactorStyle;
    std::vector<ViewportPaneDesc> m_pendingLayout;
};

}  // namespace controllers

#endif  // XQVtkViewport_VIEWPORTCONTROLLER_HPP
