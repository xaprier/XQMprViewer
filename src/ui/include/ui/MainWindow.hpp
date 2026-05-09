#ifndef XQVtkViewport_MAINWINDOW_HPP
#define XQVtkViewport_MAINWINDOW_HPP

#include <vtkSmartPointer.h>

#include <QMainWindow>

class QTabWidget;

namespace controllers {
class DicomController;
}  // namespace controllers

namespace ui {

class ControllerPanel;
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

    controllers::DicomController* m_dicomController{nullptr};
    QTabWidget* m_tabWidget{nullptr};
    MultiWindowView* m_multiWindowView{nullptr};
    ViewportView* m_viewportView{nullptr};
    ControllerPanel* m_controllerPanel{nullptr};
};

}  // namespace ui

#endif  // XQVtkViewport_MAINWINDOW_HPP
