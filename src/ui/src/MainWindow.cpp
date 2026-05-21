#include "ui/MainWindow.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <QApplication>
#include <QDockWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "adapters/ColorAdapter.hpp"
#include "adapters/DicomMetaDataAdapter.hpp"
#include "adapters/OverlayLayoutAdapter.hpp"
#include "controllers/DicomController.hpp"
#include "controllers/MultiWindowController.hpp"
#include "overlays/CornerAnnotationOverlay.hpp"
#include "overlays/FPSOverlay.hpp"
#include "overlays/OrientationMarkerOverlay.hpp"
#include "ui/AboutDialog.hpp"
#include "ui/ControllerPanel.hpp"
#include "ui/ControllerPanelCornerAnnotationItem.hpp"
#include "ui/ControllerPanelSphereItem.hpp"
#include "ui/DicomMetaDataPanel.hpp"
#include "ui/MultiWindowView.hpp"
#include "ui/ViewportLayoutManager.hpp"
#include "ui/ViewportLayoutSelector.hpp"
#include "ui/ViewportLayoutTypes.hpp"
#include "ui/ViewportView.hpp"

namespace ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_dicomController(new controllers::DicomController(this)),
      m_layoutManager(std::make_unique<ViewportLayoutManager>(this)) {
    setWindowTitle(tr("%1 — Widgets Demo").arg(qApp->applicationName()));
    resize(1400, 860);
    _BuildUi();
    _ConnectSignals();
    _ConnectLayoutSignals();
}

MainWindow::~MainWindow() = default;

void MainWindow::_BuildUi() {
    // ── Left dock: controller panel ──────────────────────────────────────────
    m_controllerPanel = new ControllerPanel(this);
    auto* leftDock = new QDockWidget(tr("Controller Panel"), this);
    leftDock->setWidget(m_controllerPanel);
    leftDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    leftDock->setMinimumWidth(220);
    leftDock->setMaximumWidth(280);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    // ── Central area: layout selector + tab widget ───────────────────────────
    // We wrap the tab widget and layout selector in a vertical layout inside
    // a central container widget so the selector sits BELOW the tab bar.
    auto* centralContainer = new QWidget(this);
    auto* centralLayout = new QVBoxLayout(centralContainer);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    m_tabWidget = new QTabWidget(centralContainer);
    centralLayout->addWidget(m_tabWidget, 1);

    _BuildViewportTab();
    _BuildMultiWindowTab();
    _BuildLayoutPanel();

    centralLayout->addWidget(m_layoutSelector);

    setCentralWidget(centralContainer);

    // ── Right dock: DICOM metadata ───────────────────────────────────────────
    m_metaDataAdapter = new adapters::DicomMetaDataAdapter(m_dicomController, this);
    m_metaDataPanel = new DicomMetaDataPanel(m_metaDataAdapter, this);
    auto* rightDock = new QDockWidget(tr("DICOM Metadata"), this);
    rightDock->setWidget(m_metaDataPanel);
    rightDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    rightDock->setMinimumWidth(260);
    rightDock->setMaximumWidth(420);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);

    auto* viewMenu = menuBar()->addMenu(tr("View"));
    viewMenu->addAction(rightDock->toggleViewAction());
    viewMenu->addAction(leftDock->toggleViewAction());

    auto* layoutPanelAction = new QAction(tr("Layout Panel"), this);
    layoutPanelAction->setCheckable(true);
    layoutPanelAction->setChecked(true);
    connect(layoutPanelAction, &QAction::toggled, m_layoutSelector, &QWidget::setVisible);
    viewMenu->addAction(layoutPanelAction);

    // Register all corner annotation overlays with their ControllerPanel item.
    auto* caItem = m_controllerPanel->GetCornerAnnotationItem();
    for (auto* ov : m_viewportView->GetOverlays<overlays::CornerAnnotationOverlay>())
        caItem->AddOverlay(ov);
    for (auto* ov : m_multiWindowView->GetOverlays<overlays::CornerAnnotationOverlay>())
        caItem->AddOverlay(ov);

    auto* helpMenu = menuBar()->addMenu(tr("Help"));
    auto* aboutAction = new QAction(tr("About %1…").arg(qApp->applicationName()), this);
    connect(aboutAction, &QAction::triggered, this, [this]() {
        AboutDialog dlg(this);
        dlg.exec();
    });
    helpMenu->addAction(aboutAction);

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

void MainWindow::_BuildLayoutPanel() {
    m_layoutSelector = new ViewportLayoutSelector(this);

    // Register both views as layout targets.
    m_layoutManager->RegisterTarget(m_viewportView);
    m_layoutManager->RegisterTarget(m_multiWindowView);

    // Build and register overlay adapters for ViewportView (shared-viewport mode).
    {
        auto adapter = std::make_unique<adapters::OverlayLayoutAdapter>(true);
        for (auto* ov : m_viewportView->GetOverlays<overlays::OrientationMarkerOverlay>())
            adapter->AddOrientationMarkerOverlay(ov);
        for (auto* ov : m_viewportView->GetOverlays<overlays::CornerAnnotationOverlay>())
            adapter->AddCornerAnnotationOverlay(ov);
        for (auto* ov : m_viewportView->GetOverlays<overlays::FPSOverlay>())
            adapter->AddFPSOverlay(ov);
        m_layoutManager->RegisterOverlayAdapter(std::move(adapter));
    }

    // Build and register overlay adapters for MultiWindowView (per-widget mode).
    {
        auto adapter = std::make_unique<adapters::OverlayLayoutAdapter>(false);
        for (auto* ov : m_multiWindowView->GetOverlays<overlays::OrientationMarkerOverlay>())
            adapter->AddOrientationMarkerOverlay(ov);
        for (auto* ov : m_multiWindowView->GetOverlays<overlays::CornerAnnotationOverlay>())
            adapter->AddCornerAnnotationOverlay(ov);
        for (auto* ov : m_multiWindowView->GetOverlays<overlays::FPSOverlay>())
            adapter->AddFPSOverlay(ov);
        m_layoutManager->RegisterOverlayAdapter(std::move(adapter));
    }
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

    connect(m_dicomController, &controllers::DicomController::StatusChanged, this, [this](const QString& message) {
        _Notification("DICOM: " + message);
    });

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

    // Orientation marker signals: forward to overlays AND notify layout manager
    // so it can recompute combined (userEnabled && layoutVisible) state.
    connect(m_controllerPanel, &ControllerPanel::OrientationMarkerEnableChanged, this, [this](bool enabled) {
        m_layoutManager->SetOrientationMarkersEnabled(enabled);
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

    // Corner annotation enable goes through layout manager too.
    connect(m_controllerPanel, &ControllerPanel::CornerAnnotationEnableChanged, this, [this](bool enabled) {
        m_layoutManager->SetCornerAnnotationsEnabled(enabled);
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

void MainWindow::_ConnectLayoutSignals() {
    // Layout selector → manager
    connect(m_layoutSelector, &ViewportLayoutSelector::layoutSelected,
            m_layoutManager.get(), &ViewportLayoutManager::ApplyLayout);

    // Right-click slot assignment → manager
    connect(m_layoutSelector, &ViewportLayoutSelector::slotAssignmentRequested,
            m_layoutManager.get(), &ViewportLayoutManager::ApplySlotAssignment);

    // Keep selector highlight in sync if layout is changed programmatically.
    // Also update the active button's definition so pane colors reflect the assignment.
    connect(m_layoutManager.get(), &ViewportLayoutManager::layoutChanged,
            this, [this](const ViewportLayoutDefinition& def) {
                m_layoutSelector->SetActiveLayout(def.type);
                m_layoutSelector->UpdateActiveDefinition(def);
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
