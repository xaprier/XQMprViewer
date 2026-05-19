#ifndef XQVtkViewport_MAINWINDOW_HPP
#define XQVtkViewport_MAINWINDOW_HPP

#include <vtkSmartPointer.h>

#include <QMainWindow>

class QTabWidget;

namespace adapters {
class DicomMetaDataAdapter;
}  // namespace adapters

namespace controllers {
class DicomController;
}  // namespace controllers

namespace overlays {
class FPSOverlay;
class OrientationMarkerOverlay;
}  // namespace overlays

namespace ui {

class ControllerPanel;
class DicomMetaDataPanel;
class MultiWindowView;
class ViewportView;

/**
 * @brief Top-level Qt Widgets main window.
 *
 * Hosts a QTabWidget with a Viewport Mode tab and a Multi-Window Mode tab,
 * plus a shared ControllerPanel for DICOM loading and sphere controls.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

  private:
    void _BuildUi();
    void _BuildViewportTab();
    void _BuildMultiWindowTab();
    void _ConnectSignals();
    void _Notification(const QString& message);

    /**
     * @brief Apply @p fn to every overlay of type @p OverlayT across both views.
     *
     * IView::GetOverlays<T>() dispatches to the correct vector based on T.
     * Adding a new overlay type only requires adding a GetOverlays<T>()
     * specialisation to IView — no changes needed here.
     */
    template <typename OverlayT, typename Fn>
    void _ForEachOverlay(Fn&& fn);

    controllers::DicomController* m_dicomController{nullptr};
    adapters::DicomMetaDataAdapter* m_metaDataAdapter{nullptr};
    QTabWidget* m_tabWidget{nullptr};
    MultiWindowView* m_multiWindowView{nullptr};
    ViewportView* m_viewportView{nullptr};
    ControllerPanel* m_controllerPanel{nullptr};
    DicomMetaDataPanel* m_metaDataPanel{nullptr};
};

}  // namespace ui

#endif  // XQVtkViewport_MAINWINDOW_HPP
