#include "ui/MultiWindowView.hpp"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <QGridLayout>
#include <QShowEvent>
#include <memory>

#include <vtkResliceImageViewer.h>

#include "controllers/MultiWindowController.hpp"
#include "controllers/SliceController.hpp"
#include "overlays/CornerAnnotationOverlay.hpp"
#include "overlays/FPSOverlay.hpp"
#include "overlays/OrientationMarkerOverlay.hpp"
#include "ui/ViewportLayoutTypes.hpp"

namespace ui {

MultiWindowView::MultiWindowView(QWidget* parent)
    : IView(parent),
      m_multiWindowController(std::make_unique<controllers::MultiWindowController>()) {
    _setupUI();
}

MultiWindowView::~MultiWindowView() = default;

void MultiWindowView::_setupUI() {
    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setSpacing(2);

    m_vtkWidgets.resize(3);
    m_fpsOverlays.resize(3);
    m_orientationMarkerOverlays.resize(3);

    for (int i = 0; i < 3; ++i) {
        auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
        renderWindow->SetMultiSamples(0);

        m_vtkWidgets[i] = new QVTKOpenGLNativeWidget(this);
        m_vtkWidgets[i]->setRenderWindow(renderWindow);

        m_fpsOverlays[i] = new overlays::FPSOverlay(m_vtkWidgets[i]);
        m_orientationMarkerOverlays[i] = new overlays::OrientationMarkerOverlay(
            m_vtkWidgets[i], overlays::OrientationMarkerOverlay::kSliceOrientations[i]);
    }

    static const QString kViewNames[3] = {"Axial", "Coronal", "Sagittal"};
    m_cornerAnnotationOverlays.resize(3);
    for (int i = 0; i < 3; ++i)
        m_cornerAnnotationOverlays[i] = new overlays::CornerAnnotationOverlay(m_vtkWidgets[i], kViewNames[i]);

    m_multiWindowController->Initialize(m_vtkWidgets);
    connect(m_multiWindowController.get(), &controllers::MultiWindowController::StatusChanged,
            this, &MultiWindowView::StatusChanged);

    const auto& viewers = m_multiWindowController->GetSliceController()->GetViewers();
    for (int i = 0; i < static_cast<int>(m_cornerAnnotationOverlays.size()); ++i) {
        if (i < static_cast<int>(viewers.size()))
            m_cornerAnnotationOverlays[i]->SetViewer(viewers[i].Get());
    }

    // Start with horizontal split (default).
    _RebuildGrid(MakeLayoutDefinition(ViewportLayoutType::HorizontalSplit));
}

void MultiWindowView::showEvent(QShowEvent* event) {
    IView::showEvent(event);
    // The widget may have been hidden when image data was first loaded, causing
    // FitToView to compute with size [0,0]. Re-run it now that the windows are
    // actually on-screen and have valid pixel dimensions.
    if (!m_multiWindowController->IsInitialized())
        return;
    auto* sc = m_multiWindowController->GetSliceController();
    if (!sc)
        return;
    sc->FitToView();
    auto* scheduler = sc->Scheduler();
    if (scheduler) {
        sc->RequestRenderAll();
        scheduler->Flush();
    }
}

void MultiWindowView::ApplyLayout(const ViewportLayoutDefinition& def) {
    _RebuildGrid(def);

    if (!m_multiWindowController->IsInitialized())
        return;

    auto* sc = m_multiWindowController->GetSliceController();
    if (!sc)
        return;

    // FitToView recomputes each plane's camera after widget resize.
    sc->FitToView();

    // Request renders through the scheduler so the RIV pipeline runs before
    // the window render (required when a sphere actor is present).
    auto* scheduler = sc->Scheduler();
    if (scheduler) {
        sc->RequestRenderAll();
        scheduler->Flush();
    }
}

void MultiWindowView::_RebuildGrid(const ViewportLayoutDefinition& def) {
    // Remove all widgets from the grid without deleting them.
    for (int i = 0; i < 3; ++i) {
        if (m_vtkWidgets[i]) {
            m_grid->removeWidget(m_vtkWidgets[i]);
            m_vtkWidgets[i]->hide();
        }
    }

    // Clear stretch factors.
    for (int r = 0; r < m_grid->rowCount(); ++r)
        m_grid->setRowStretch(r, 0);
    for (int c = 0; c < m_grid->columnCount(); ++c)
        m_grid->setColumnStretch(c, 0);

    const ViewportLayoutType type = def.type;

    switch (type) {
        case ViewportLayoutType::SingleAxial:
        case ViewportLayoutType::SingleCoronal:
        case ViewportLayoutType::SingleSagittal: {
            // One full-screen widget determined by the pane's planeIndex.
            int plane = def.panes.empty() ? 0 : def.panes[0].planeIndex;
            if (plane < 0 || plane > 2) plane = 0;
            m_grid->addWidget(m_vtkWidgets[plane], 0, 0, 1, 1);
            m_vtkWidgets[plane]->show();
            m_grid->setRowStretch(0, 1);
            m_grid->setColumnStretch(0, 1);
            break;
        }

        case ViewportLayoutType::HorizontalSplit: {
            // Three equal columns. The pane order from the definition gives us
            // the mapping: slot 0 → col 0, slot 1 → col 1, slot 2 → col 2.
            for (const auto& pane : def.panes) {
                const int plane = pane.planeIndex;
                const int col   = pane.slotIndex;
                if (plane < 0 || plane > 2 || col < 0 || col > 2) continue;
                m_grid->addWidget(m_vtkWidgets[plane], 0, col, 1, 1);
                m_vtkWidgets[plane]->show();
                m_grid->setColumnStretch(col, 1);
            }
            m_grid->setRowStretch(0, 1);
            break;
        }

        case ViewportLayoutType::VerticalSplit: {
            // Three equal rows. slot 0 → row 2 (bottom), slot 1 → row 1, slot 2 → row 0 (top).
            // VTK y=0 is bottom; visually we want slot 0 at bottom → Qt row 2.
            for (const auto& pane : def.panes) {
                const int plane  = pane.planeIndex;
                const int qtRow  = 2 - pane.slotIndex;  // invert: slot 0 = bottom row
                if (plane < 0 || plane > 2 || qtRow < 0 || qtRow > 2) continue;
                m_grid->addWidget(m_vtkWidgets[plane], qtRow, 0, 1, 1);
                m_vtkWidgets[plane]->show();
                m_grid->setRowStretch(qtRow, 1);
            }
            m_grid->setColumnStretch(0, 1);
            break;
        }

        case ViewportLayoutType::TwoPlusOne: {
            // slot 0: left full-height (col 0, rows 0-1)
            // slot 1: right top        (col 1, row 0)
            // slot 2: right bottom     (col 1, row 1)
            for (const auto& pane : def.panes) {
                const int plane = pane.planeIndex;
                if (plane < 0 || plane > 2) continue;
                if (pane.slotIndex == 0) {
                    m_grid->addWidget(m_vtkWidgets[plane], 0, 0, 2, 1);
                } else if (pane.slotIndex == 1) {
                    m_grid->addWidget(m_vtkWidgets[plane], 0, 1, 1, 1);
                } else if (pane.slotIndex == 2) {
                    m_grid->addWidget(m_vtkWidgets[plane], 1, 1, 1, 1);
                }
                m_vtkWidgets[plane]->show();
            }
            m_grid->setColumnStretch(0, 1);
            m_grid->setColumnStretch(1, 1);
            m_grid->setRowStretch(0, 1);
            m_grid->setRowStretch(1, 1);
            break;
        }

        case ViewportLayoutType::TwoPlusOneReversed: {
            // slot 0: top-left         (row 0, col 0)
            // slot 1: top-right        (row 0, col 1)
            // slot 2: bottom full-width(row 1, cols 0-1)
            for (const auto& pane : def.panes) {
                const int plane = pane.planeIndex;
                if (plane < 0 || plane > 2) continue;
                if (pane.slotIndex == 0) {
                    m_grid->addWidget(m_vtkWidgets[plane], 0, 0, 1, 1);
                } else if (pane.slotIndex == 1) {
                    m_grid->addWidget(m_vtkWidgets[plane], 0, 1, 1, 1);
                } else if (pane.slotIndex == 2) {
                    m_grid->addWidget(m_vtkWidgets[plane], 1, 0, 1, 2);
                }
                m_vtkWidgets[plane]->show();
            }
            m_grid->setColumnStretch(0, 1);
            m_grid->setColumnStretch(1, 1);
            m_grid->setRowStretch(0, 1);
            m_grid->setRowStretch(1, 1);
            break;
        }
    }
}

}  // namespace ui
