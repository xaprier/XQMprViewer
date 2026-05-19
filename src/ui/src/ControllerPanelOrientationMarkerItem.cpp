#include "ui/ControllerPanelOrientationMarkerItem.hpp"

#include <qboxlayout.h>

#include <QCheckBox>
#include <QColorDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace ui {

ControllerPanelOrientationMarkerItem::ControllerPanelOrientationMarkerItem(QWidget* parent)
    : QWidget(parent) {
    _setupUi();
}

ControllerPanelOrientationMarkerItem::~ControllerPanelOrientationMarkerItem() = default;

void ControllerPanelOrientationMarkerItem::AddOverlay(overlays::OrientationMarkerOverlay* overlay) {
    if (!overlay || m_overlays.contains(overlay))
        return;
    m_overlays.append(overlay);
    overlay->SetEnabled(m_enableCheck->isChecked());
    overlay->SetLongLabels(m_longLabelsCheck->isChecked());
    overlay->SetFontSize(m_fontSizeSpin->value());
    overlay->SetTextColor(m_textColor);
}

void ControllerPanelOrientationMarkerItem::ClearOverlays() {
    m_overlays.clear();
}

void ControllerPanelOrientationMarkerItem::_setupUi() {
    auto* group = new QGroupBox(tr("Orientation Markers"), this);
    auto* form = new QFormLayout(group);
    QHBoxLayout* hb = new QHBoxLayout();  // for grouping check boxes in 2 columns 1 row
    form->setSpacing(4);

    m_enableCheck = new QCheckBox(tr("Enable"), group);
    m_enableCheck->setChecked(true);

    m_longLabelsCheck = new QCheckBox(tr("Long labels"), group);
    m_longLabelsCheck->setChecked(false);
    m_longLabelsCheck->setToolTip(tr("Show full names (Left/Right/…) instead of abbreviations (L/R/…)"));

    hb->addWidget(m_enableCheck);
    hb->addWidget(m_longLabelsCheck);

    m_fontSizeSpin = new QSpinBox(group);
    m_fontSizeSpin->setRange(6, 48);
    m_fontSizeSpin->setValue(12);
    m_fontSizeSpin->setSuffix(" pt");

    m_colorButton = new QPushButton(group);
    m_colorButton->setFixedHeight(22);
    m_colorButton->setStyleSheet(
        QString("background-color: %1;").arg(m_textColor.name()));
    m_colorButton->setToolTip(tr("Pick label color"));

    form->addRow(hb);
    form->addRow(tr("Font size:"), m_fontSizeSpin);
    form->addRow(tr("Label color:"), m_colorButton);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(group);

    connect(m_enableCheck, &QCheckBox::toggled,
            this, &ControllerPanelOrientationMarkerItem::_onEnabledToggled);
    connect(m_longLabelsCheck, &QCheckBox::toggled,
            this, &ControllerPanelOrientationMarkerItem::_onLongLabelsToggled);
    connect(m_fontSizeSpin, qOverload<int>(&QSpinBox::valueChanged),
            this, &ControllerPanelOrientationMarkerItem::_onFontSizeChanged);
    connect(m_colorButton, &QPushButton::clicked,
            this, &ControllerPanelOrientationMarkerItem::_onColorPicked);
}

void ControllerPanelOrientationMarkerItem::_onEnabledToggled(bool checked) {
    for (auto* ov : m_overlays)
        ov->SetEnabled(checked);
    emit OrientationMarkerEnableChanged(checked);
}

void ControllerPanelOrientationMarkerItem::_onLongLabelsToggled(bool checked) {
    for (auto* ov : m_overlays)
        ov->SetLongLabels(checked);
    emit OrientationMarkerLongLabelsChanged(checked);
}

void ControllerPanelOrientationMarkerItem::_onFontSizeChanged(int pt) {
    for (auto* ov : m_overlays)
        ov->SetFontSize(pt);
    emit OrientationMarkerFontSizeChanged(pt);
}

void ControllerPanelOrientationMarkerItem::_onColorPicked() {
    QColor color = QColorDialog::getColor(m_textColor, this, tr("Select Label Color"));
    if (!color.isValid())
        return;
    m_textColor = color;
    m_colorButton->setStyleSheet(
        QString("background-color: %1;").arg(color.name()));
    for (auto* ov : m_overlays)
        ov->SetTextColor(color);
    emit OrientationMarkerColorChanged(color);
}

}  // namespace ui
