#include "ui/ControllerPanel.hpp"

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

#include "ui/ControllerPanelDicomItem.hpp"
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

void ControllerPanel::_setupUi() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(8);

    m_dicomItem = new ControllerPanelDicomItem(this);
    m_sphereItem = new ControllerPanelSphereItem(this);

    auto* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    auto* statusSeparator = new QFrame(this);
    statusSeparator->setFrameShape(QFrame::HLine);
    statusSeparator->setFrameShadow(QFrame::Sunken);

    layout->addWidget(m_dicomItem);
    layout->addWidget(separator);
    layout->addWidget(m_sphereItem);
    layout->addStretch(1);
    layout->addWidget(statusSeparator);

    connect(m_dicomItem, &ControllerPanelDicomItem::DirectorySelected, this, &ControllerPanel::DirectorySelected);
    connect(m_dicomItem, &ControllerPanelDicomItem::SeriesLoadRequested, this, &ControllerPanel::SeriesLoadRequested);
    connect(m_sphereItem, &ControllerPanelSphereItem::SphereAddRemoveClicked, this, &ControllerPanel::SphereAddRemoveClicked);
    connect(m_sphereItem, &ControllerPanelSphereItem::SphereRadiusChanged, this, &ControllerPanel::SphereRadiusChanged);
    connect(m_sphereItem, &ControllerPanelSphereItem::SphereColorChanged, this, &ControllerPanel::SphereColorChanged);
}

}  // namespace ui
