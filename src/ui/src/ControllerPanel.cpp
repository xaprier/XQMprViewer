#include "ui/ControllerPanel.hpp"

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

#include "ui/ControllerPanelCornerAnnotationItem.hpp"
#include "ui/ControllerPanelDicomItem.hpp"
#include "ui/ControllerPanelFPSOverlayItem.hpp"
#include "ui/ControllerPanelOrientationMarkerItem.hpp"
#include "ui/ControllerPanelSphereItem.hpp"

namespace ui {

ControllerPanel::ControllerPanel(QWidget* parent)
    : QWidget(parent) {
    _setupUi();
}

ControllerPanel::~ControllerPanel() = default;

void ControllerPanel::SetSeries(const QStringList& seriesNames) {
    m_dicomItem->SetSeries(seriesNames);
}

ControllerPanelDicomItem* ControllerPanel::GetDicomItem() const {
    return m_dicomItem;
}

ControllerPanelSphereItem* ControllerPanel::GetSphereItem() const {
    return m_sphereItem;
}

ControllerPanelFPSOverlayItem* ControllerPanel::GetFPSOverlayItem() const {
    return m_fpsOverlayItem;
}

ControllerPanelOrientationMarkerItem* ControllerPanel::GetOrientationMarkerItem() const {
    return m_orientationMarkerItem;
}

ControllerPanelCornerAnnotationItem* ControllerPanel::GetCornerAnnotationItem() const {
    return m_cornerAnnotationItem;
}

void ControllerPanel::_setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_dicomItem = new ControllerPanelDicomItem(this);
    m_sphereItem = new ControllerPanelSphereItem(this);
    m_fpsOverlayItem = new ControllerPanelFPSOverlayItem(this);
    m_orientationMarkerItem = new ControllerPanelOrientationMarkerItem(this);
    m_cornerAnnotationItem = new ControllerPanelCornerAnnotationItem(this);

    auto makeSep = [this]() -> QFrame* {
        auto* sep = new QFrame(this);
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        return sep;
    };

    layout->addWidget(m_dicomItem);
    layout->addWidget(makeSep());
    layout->addWidget(m_sphereItem);
    layout->addWidget(makeSep());
    layout->addWidget(m_fpsOverlayItem);
    layout->addWidget(makeSep());
    layout->addWidget(m_orientationMarkerItem);
    layout->addWidget(makeSep());
    layout->addWidget(m_cornerAnnotationItem);
    layout->addStretch(1);

    connect(m_dicomItem, &ControllerPanelDicomItem::DirectorySelected,
            this, &ControllerPanel::DirectorySelected);
    connect(m_dicomItem, &ControllerPanelDicomItem::SeriesLoadRequested,
            this, &ControllerPanel::SeriesLoadRequested);
    connect(m_sphereItem, &ControllerPanelSphereItem::SphereAddRemoveClicked,
            this, &ControllerPanel::SphereAddRemoveClicked);
    connect(m_sphereItem, &ControllerPanelSphereItem::SphereRadiusChanged,
            this, &ControllerPanel::SphereRadiusChanged);
    connect(m_sphereItem, &ControllerPanelSphereItem::SphereColorChanged,
            this, &ControllerPanel::SphereColorChanged);
    connect(m_fpsOverlayItem, &ControllerPanelFPSOverlayItem::FPSOverlayEnableChanged,
            this, &ControllerPanel::FPSOverlayEnableChanged);
    connect(m_fpsOverlayItem, &ControllerPanelFPSOverlayItem::FPSOverlayColorChanged,
            this, &ControllerPanel::FPSOverlayColorChanged);
    connect(m_fpsOverlayItem, &ControllerPanelFPSOverlayItem::FPSOverlayPositionChanged,
            this, &ControllerPanel::FPSOverlayPositionChanged);
    connect(m_fpsOverlayItem, &ControllerPanelFPSOverlayItem::FPSOverlayMarginChanged,
            this, &ControllerPanel::FPSOverlayMarginChanged);
    connect(m_fpsOverlayItem, &ControllerPanelFPSOverlayItem::FPSOverlayFontSizeChanged,
            this, &ControllerPanel::FPSOverlayFontSizeChanged);
    connect(m_orientationMarkerItem, &ControllerPanelOrientationMarkerItem::OrientationMarkerEnableChanged,
            this, &ControllerPanel::OrientationMarkerEnableChanged);
    connect(m_orientationMarkerItem, &ControllerPanelOrientationMarkerItem::OrientationMarkerLongLabelsChanged,
            this, &ControllerPanel::OrientationMarkerLongLabelsChanged);
    connect(m_orientationMarkerItem, &ControllerPanelOrientationMarkerItem::OrientationMarkerColorChanged,
            this, &ControllerPanel::OrientationMarkerColorChanged);
    connect(m_orientationMarkerItem, &ControllerPanelOrientationMarkerItem::OrientationMarkerFontSizeChanged,
            this, &ControllerPanel::OrientationMarkerFontSizeChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationEnableChanged,
            this, &ControllerPanel::CornerAnnotationEnableChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationPositionChanged,
            this, &ControllerPanel::CornerAnnotationPositionChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationColorChanged,
            this, &ControllerPanel::CornerAnnotationColorChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationFontSizeChanged,
            this, &ControllerPanel::CornerAnnotationFontSizeChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationMarginChanged,
            this, &ControllerPanel::CornerAnnotationMarginChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationShowSliceInfoChanged,
            this, &ControllerPanel::CornerAnnotationShowSliceInfoChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationShowWindowLevelChanged,
            this, &ControllerPanel::CornerAnnotationShowWindowLevelChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationShowSpacingChanged,
            this, &ControllerPanel::CornerAnnotationShowSpacingChanged);
    connect(m_cornerAnnotationItem, &ControllerPanelCornerAnnotationItem::CornerAnnotationShowViewNameChanged,
            this, &ControllerPanel::CornerAnnotationShowViewNameChanged);
}

}  // namespace ui
