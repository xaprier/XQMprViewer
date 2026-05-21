#ifndef XQVtkViewport_MAINWINDOW_HPP
#define XQVtkViewport_MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>

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
class ViewportLayoutManager;
class ViewportLayoutSelector;

/**
 * @brief Top-level Qt Widgets main window.
 *
 * Hosts a QTabWidget with a Viewport Mode tab and a Multi-Window Mode tab,
 * plus a shared ControllerPanel for DICOM loading and sphere controls,
 * and a ViewportLayoutSelector toolbar below the tab widget that drives
 * layout changes across both views via ViewportLayoutManager.
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
    void _BuildLayoutPanel();
    void _ConnectSignals();
    void _ConnectLayoutSignals();
    void _Notification(const QString& message);

    template <typename OverlayT, typename Fn>
    void _ForEachOverlay(Fn&& fn);

    controllers::DicomController* m_dicomController{nullptr};
    adapters::DicomMetaDataAdapter* m_metaDataAdapter{nullptr};
    QTabWidget* m_tabWidget{nullptr};
    MultiWindowView* m_multiWindowView{nullptr};
    ViewportView* m_viewportView{nullptr};
    ControllerPanel* m_controllerPanel{nullptr};
    DicomMetaDataPanel* m_metaDataPanel{nullptr};
    ViewportLayoutSelector* m_layoutSelector{nullptr};
    std::unique_ptr<ViewportLayoutManager> m_layoutManager;
};

}  // namespace ui

#endif  // XQVtkViewport_MAINWINDOW_HPP
