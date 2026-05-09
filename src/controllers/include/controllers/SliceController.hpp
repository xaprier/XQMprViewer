#ifndef SLICECONTROLLER_HPP
#define SLICECONTROLLER_HPP

#include <vtkSmartPointer.h>

#include <array>
#include <vector>

#include "controllers/IControllerBase.hpp"

class vtkImageData;
class vtkResliceImageViewer;
class vtkRenderer;
class QVTKOpenGLNativeWidget;

namespace controllers {

/**
 * @brief Manages up to three vtkResliceImageViewer instances for orthogonal slice display.
 *
 * Supports two operating modes, detected automatically from the widget vector size:
 * - 3 widgets → multi-window mode: each viewer owns a separate render window.
 * - 1 widget  → viewport mode: all viewers share one render window; interaction
 *               routing is handled externally by ViewportInteractorStyle.
 */
class SliceController : public IControllerBase {
    Q_OBJECT

  public:
    enum Orientation {
        Axial = 0,
        Coronal = 1,
        Sagittal = 2
    };
    using Vec3 = std::array<double, 3>;

    explicit SliceController(QObject* parent = nullptr);
    ~SliceController() override;

    /**
     * @brief Initialises the viewers from the provided widget list.
     *
     * Pass 3 widgets for multi-window mode or 1 widget for viewport mode.
     */
    void Initialize(const std::vector<QVTKOpenGLNativeWidget*>& vtkWidgets);

    /** @brief Pushes @p image into the reslice pipeline and refreshes all views. */
    void SetImageData(vtkImageData* image);

    /** @brief Fills @p outRenderers with the renderer from each viewer. */
    void GetRenderers(std::vector<vtkSmartPointer<vtkRenderer>>& outRenderers) const;

    /** @brief Renders all views via vtkResliceImageViewer::Render() to keep the reslice pipeline consistent. */
    void RenderAll();

    /** @brief Returns the underlying viewer list, e.g. for use by ViewportInteractorStyle. */
    const std::vector<vtkSmartPointer<vtkResliceImageViewer>>& GetViewers() const;

  public slots:
    /** @brief Scrolls each slice plane to the position nearest to @p worldPos. */
    void OnSphereUpdated(const Vec3& worldPos);

  private:
    void SetupViewers();
    void SetupPipeline();
    void FitToView();

    vtkSmartPointer<vtkImageData> m_image;

    std::vector<vtkSmartPointer<vtkResliceImageViewer>> m_rivs;
    std::vector<QVTKOpenGLNativeWidget*> m_vtkWidgets;
};

}  // namespace controllers
#endif  // SLICECONTROLLER_HPP
