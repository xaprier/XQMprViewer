#include "ui/ControllerPanelFPSOverlayItem.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "overlays/IOverlay.hpp"

namespace ui {

ControllerPanelFPSOverlayItem::ControllerPanelFPSOverlayItem(QWidget* parent)
    : QWidget(parent) {
    _setupUi();
}

ControllerPanelFPSOverlayItem::~ControllerPanelFPSOverlayItem() = default;

void ControllerPanelFPSOverlayItem::AddOverlay(overlays::FPSOverlay* overlay) {
    if (!overlay || m_overlays.contains(overlay))
        return;
    m_overlays.append(overlay);
    // Apply current panel state to the newly attached overlay.
    overlay->SetEnabled(m_enableCheck->isChecked());
    overlay->SetPosition(static_cast<overlays::OverlayPosition>(m_positionCombo->currentIndex()));
    overlay->SetMargin(m_marginSpin->value());
    overlay->SetFontSize(m_fontSizeSpin->value());
    overlay->SetTextColor(m_textColor);
}

void ControllerPanelFPSOverlayItem::ClearOverlays() {
    m_overlays.clear();
}

void ControllerPanelFPSOverlayItem::_setupUi() {
    auto* group = new QGroupBox(tr("FPS Overlay"), this);
    auto* form = new QFormLayout(group);
    form->setSpacing(4);

    m_enableCheck = new QCheckBox(tr("Enable"), group);
    m_enableCheck->setChecked(true);

    m_positionCombo = new QComboBox(group);
    m_positionCombo->addItems(overlays::GetOverlayPositionNames());

    m_marginSpin = new QSpinBox(group);
    m_marginSpin->setRange(0, 100);
    m_marginSpin->setValue(8);
    m_marginSpin->setSuffix(" px");

    m_fontSizeSpin = new QSpinBox(group);
    m_fontSizeSpin->setRange(6, 48);
    m_fontSizeSpin->setValue(10);
    m_fontSizeSpin->setSuffix(" pt");

    m_colorButton = new QPushButton(group);
    m_colorButton->setFixedHeight(22);
    m_colorButton->setStyleSheet(
        QString("background-color: %1;").arg(m_textColor.name()));
    m_colorButton->setToolTip(tr("Pick text color"));

    form->addRow(m_enableCheck);
    form->addRow(tr("Position:"), m_positionCombo);
    form->addRow(tr("Margin:"), m_marginSpin);
    form->addRow(tr("Font size:"), m_fontSizeSpin);
    form->addRow(tr("Text color:"), m_colorButton);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(group);

    connect(m_enableCheck, &QCheckBox::toggled,
            this, &ControllerPanelFPSOverlayItem::_onEnabledToggled);
    connect(m_positionCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ControllerPanelFPSOverlayItem::_onPositionChanged);
    connect(m_marginSpin, qOverload<int>(&QSpinBox::valueChanged),
            this, &ControllerPanelFPSOverlayItem::_onMarginChanged);
    connect(m_fontSizeSpin, qOverload<int>(&QSpinBox::valueChanged),
            this, &ControllerPanelFPSOverlayItem::_onFontSizeChanged);
    connect(m_colorButton, &QPushButton::clicked,
            this, &ControllerPanelFPSOverlayItem::_onColorPicked);
}

void ControllerPanelFPSOverlayItem::_onEnabledToggled(bool checked) {
    for (auto* ov : m_overlays)
        ov->SetEnabled(checked);

    emit FPSOverlayEnableChanged(checked);
}

void ControllerPanelFPSOverlayItem::_onPositionChanged(int index) {
    auto pos = static_cast<overlays::OverlayPosition>(index);
    for (auto* ov : m_overlays)
        ov->SetPosition(pos);

    emit FPSOverlayPositionChanged(pos);
}

void ControllerPanelFPSOverlayItem::_onMarginChanged(int px) {
    for (auto* ov : m_overlays)
        ov->SetMargin(px);

    emit FPSOverlayMarginChanged(px);
}

void ControllerPanelFPSOverlayItem::_onFontSizeChanged(int pt) {
    for (auto* ov : m_overlays)
        ov->SetFontSize(pt);

    emit FPSOverlayFontSizeChanged(pt);
}

void ControllerPanelFPSOverlayItem::_onColorPicked() {
    QColor color = QColorDialog::getColor(m_textColor, this, tr("Select Text Color"));
    if (!color.isValid())
        return;
    m_textColor = color;
    m_colorButton->setStyleSheet(
        QString("background-color: %1;").arg(color.name()));
    for (auto* ov : m_overlays)
        ov->SetTextColor(color);

    emit FPSOverlayColorChanged(color);
}

}  // namespace ui
