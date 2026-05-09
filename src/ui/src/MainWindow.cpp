#include "ui/MainWindow.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <QDockWidget>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "adapters/ColorAdapter.hpp"
#include "controllers/DicomController.hpp"
#include "controllers/MultiWindowController.hpp"
#include "ui/ControllerPanel.hpp"
#include "ui/ControllerPanelSphereItem.hpp"
#include "ui/MultiWindowView.hpp"
#include "ui/ViewportView.hpp"

namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_dicomController(new controllers::DicomController(this)) {
    setWindowTitle("XQVtkViewport — Widgets Demo");
    resize(1400, 800);
    _BuildUi();
    _ConnectSignals();
}

MainWindow::~MainWindow() = default;

void MainWindow::_BuildUi() {
    m_controllerPanel = new ControllerPanel(this);
    auto* leftDock = new QDockWidget(this);
    leftDock->setWidget(m_controllerPanel);
    leftDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    leftDock->setMinimumWidth(220);
    leftDock->setMaximumWidth(280);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    _BuildViewportTab();
    _BuildMultiWindowTab();

    statusBar()->showMessage("Ready to load DICOM series.");
}

void MainWindow::_BuildViewportTab() {
    m_viewportView = new ViewportView(this);
    m_tabWidget->addTab(m_viewportView, "Viewport Mode");
}

void MainWindow::_BuildMultiWindowTab() {
    m_multiWindowView = new MultiWindowView(this);

    m_tabWidget->addTab(m_multiWindowView, "MultiWindow Mode");
}

void MainWindow::_ConnectSignals() {
    connect(m_controllerPanel, &ControllerPanel::DirectorySelected, this, [this](const QString& dir) {
        m_dicomController->LoadDicom(dir.toStdString());
    });

    connect(m_dicomController, &controllers::DicomController::SeriesListReady, this, [this](int /*count*/) {
        QStringList names;
        for (const auto& s : m_dicomController->GetSeries()) {
            const QString desc = QString::fromStdString(s.description);
            const QString label = desc.isEmpty()
                                      ? tr("Series %1 (%2 files)").arg(s.index + 1).arg(s.fileCount)
                                      : tr("%1 (%2 files)").arg(desc).arg(s.fileCount);
            names.append(label);
        }
        m_controllerPanel->SetSeries(names);
    });

    connect(m_controllerPanel, &ControllerPanel::SeriesLoadRequested, this, [this](int seriesIndex) -> void {
        m_dicomController->LoadSeries(seriesIndex);
    });

    connect(m_controllerPanel, &ControllerPanel::SphereAddRemoveClicked, this, [this]() {
        m_multiWindowView->GetController()->ToggleSphere();
    });

    connect(m_controllerPanel, &ControllerPanel::SphereRadiusChanged, this, [this](double radius) {
        m_multiWindowView->GetController()->SetSphereRadius(radius);
    });

    connect(m_controllerPanel, &ControllerPanel::SphereColorChanged, this, [this](const QColor& color) {
        std::array<double, 3> rgb;
        adapters::ColorAdapter::QColorToRGB(color, rgb);
        m_multiWindowView->GetController()->SetSphereColor(rgb);
    });

    // Statuses
    connect(m_dicomController, &controllers::DicomController::StatusChanged, this, [this](const QString& message) {
        QString fullMessage = "DICOM: " + message;
        _Notification(fullMessage);
    });

    // IViewController::SetImageData tek çağrıyla tüm m_instances'a yayılır;
    // MultiWindowController'a bağlamak ViewportController'ı da besler.
    connect(m_dicomController, &controllers::DicomController::ImageDataReady,
            m_multiWindowView->GetController(), &controllers::MultiWindowController::SetImageData);

    connect(m_viewportView, &ViewportView::StatusChanged, this, [this](const QString& message) {
        _Notification("Viewport: " + message);
    });

    connect(m_multiWindowView, &MultiWindowView::StatusChanged, this, [this](const QString& message) {
        _Notification("MultiWindow: " + message);
    });
}
void MainWindow::_Notification(const QString& message) {
    statusBar()->showMessage(message);
}

}  // namespace ui
