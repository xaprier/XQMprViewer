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
#include "overlays/CornerAnnotationOverlay.hpp"
#include "ui/ControllerPanel.hpp"
#include "ui/ControllerPanelCornerAnnotationItem.hpp"
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

    // Register all corner annotation overlays with the controller panel item
    // so their initial settings are applied immediately.
    auto* caItem = m_controllerPanel->GetCornerAnnotationItem();
    for (auto* ov : m_viewportView->GetOverlays<overlays::CornerAnnotationOverlay>())
        caItem->AddOverlay(ov);
    for (auto* ov : m_multiWindowView->GetOverlays<overlays::CornerAnnotationOverlay>())
        caItem->AddOverlay(ov);

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

    // IViewController::SetImageData signal from DicomController is connected to MultiWindowController, which will trigger renders in both views.
    connect(m_dicomController, &controllers::DicomController::ImageDataReady,
            m_multiWindowView->GetController(), &controllers::MultiWindowController::SetImageData);

    connect(m_controllerPanel, &ControllerPanel::FPSOverlayEnableChanged, this, [this](bool enabled) {
        _ForEachOverlay<overlays::FPSOverlay>([enabled](auto* ov) { ov->SetEnabled(enabled); });
    });
    connect(m_controllerPanel, &ControllerPanel::FPSOverlayColorChanged, this, [this](const QColor& color) {
        _ForEachOverlay<overlays::FPSOverlay>([&color](auto* ov) { ov->SetTextColor(color); });
    });
    connect(m_controllerPanel, &ControllerPanel::FPSOverlayPositionChanged, this, [this](const overlays::OverlayPosition& position) {
        _ForEachOverlay<overlays::FPSOverlay>([&position](auto* ov) { ov->SetPosition(position); });
    });
    connect(m_controllerPanel, &ControllerPanel::FPSOverlayMarginChanged, this, [this](int margin) {
        _ForEachOverlay<overlays::FPSOverlay>([margin](auto* ov) { ov->SetMargin(margin); });
    });
    connect(m_controllerPanel, &ControllerPanel::FPSOverlayFontSizeChanged, this, [this](int fontSize) {
        _ForEachOverlay<overlays::FPSOverlay>([fontSize](auto* ov) { ov->SetFontSize(fontSize); });
    });

    connect(m_controllerPanel, &ControllerPanel::OrientationMarkerEnableChanged, this, [this](bool enabled) {
        _ForEachOverlay<overlays::OrientationMarkerOverlay>([enabled](auto* ov) { ov->SetEnabled(enabled); });
    });
    connect(m_controllerPanel, &ControllerPanel::OrientationMarkerColorChanged, this, [this](const QColor& color) {
        _ForEachOverlay<overlays::OrientationMarkerOverlay>([&color](auto* ov) { ov->SetTextColor(color); });
    });
    connect(m_controllerPanel, &ControllerPanel::OrientationMarkerLongLabelsChanged, this, [this](bool longLabels) {
        _ForEachOverlay<overlays::OrientationMarkerOverlay>([longLabels](auto* ov) { ov->SetLongLabels(longLabels); });
    });
    connect(m_controllerPanel, &ControllerPanel::OrientationMarkerFontSizeChanged, this, [this](int fontSize) {
        _ForEachOverlay<overlays::OrientationMarkerOverlay>([fontSize](auto* ov) { ov->SetFontSize(fontSize); });
    });

    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationEnableChanged, this, [this](bool enabled) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([enabled](auto* ov) { ov->SetEnabled(enabled); });
    });
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationPositionChanged, this, [this](const overlays::OverlayPosition& position) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([&position](auto* ov) { ov->SetPosition(position); });
    });
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationColorChanged, this, [this](const QColor& color) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([&color](auto* ov) { ov->SetTextColor(color); });
    });
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationFontSizeChanged, this, [this](int fontSize) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([fontSize](auto* ov) { ov->SetFontSize(fontSize); });
    });
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationMarginChanged, this, [this](int margin) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([margin](auto* ov) { ov->SetMargin(margin); });
    });
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationShowSliceInfoChanged, this, [this](bool show) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([show](auto* ov) { ov->SetShowSliceInfo(show); });
    });
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationShowWindowLevelChanged, this, [this](bool show) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([show](auto* ov) { ov->SetShowWindowLevel(show); });
    });
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationShowSpacingChanged, this, [this](bool show) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([show](auto* ov) { ov->SetShowSpacing(show); });
    });
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationShowViewNameChanged, this, [this](bool show) {
        _ForEachOverlay<overlays::CornerAnnotationOverlay>([show](auto* ov) { ov->SetShowViewName(show); });
    });

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

template <typename OverlayT, typename Fn>
void MainWindow::_ForEachOverlay(Fn&& fn) {
    for (auto* ov : m_viewportView->GetOverlays<OverlayT>())
        fn(ov);
    for (auto* ov : m_multiWindowView->GetOverlays<OverlayT>())
        fn(ov);
}

}  // namespace ui
