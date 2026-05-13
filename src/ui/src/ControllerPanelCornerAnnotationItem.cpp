#include "ui/ControllerPanelCornerAnnotationItem.hpp"

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

ControllerPanelCornerAnnotationItem::ControllerPanelCornerAnnotationItem(QWidget* parent)
    : QWidget(parent) {
    _setupUi();
}

ControllerPanelCornerAnnotationItem::~ControllerPanelCornerAnnotationItem() = default;

void ControllerPanelCornerAnnotationItem::AddOverlay(overlays::CornerAnnotationOverlay* overlay) {
    if (!overlay || m_overlays.contains(overlay))
        return;
    m_overlays.append(overlay);
    overlay->SetEnabled(m_enableCheck->isChecked());
    overlay->SetPosition(static_cast<overlays::OverlayPosition>(m_positionCombo->currentIndex()));
    overlay->SetShowSliceInfo(m_showSliceInfoCheck->isChecked());
    overlay->SetShowWindowLevel(m_showWindowLevelCheck->isChecked());
    overlay->SetShowSpacing(m_showSpacingCheck->isChecked());
    overlay->SetShowViewName(m_showViewNameCheck->isChecked());
    overlay->SetFontSize(m_fontSizeSpin->value());
    overlay->SetMargin(m_marginSpin->value());
    overlay->SetTextColor(m_textColor);
}

void ControllerPanelCornerAnnotationItem::ClearOverlays() {
    m_overlays.clear();
}

void ControllerPanelCornerAnnotationItem::_setupUi() {
    auto* group = new QGroupBox(tr("Corner Annotations"), this);
    auto* form  = new QFormLayout(group);
    form->setSpacing(4);

    m_enableCheck = new QCheckBox(tr("Enable"), group);
    m_enableCheck->setChecked(true);

    m_positionCombo = new QComboBox(group);
    m_positionCombo->addItems(overlays::GetOverlayPositionNames());
    m_positionCombo->setCurrentIndex(static_cast<int>(overlays::OverlayPosition::BottomRight));

    m_showSliceInfoCheck = new QCheckBox(tr("Slice info"), group);
    m_showSliceInfoCheck->setChecked(true);

    m_showWindowLevelCheck = new QCheckBox(tr("Window / Level"), group);
    m_showWindowLevelCheck->setChecked(true);

    m_showSpacingCheck = new QCheckBox(tr("Spacing"), group);
    m_showSpacingCheck->setChecked(true);

    m_showViewNameCheck = new QCheckBox(tr("View name"), group);
    m_showViewNameCheck->setChecked(true);

    m_fontSizeSpin = new QSpinBox(group);
    m_fontSizeSpin->setRange(6, 48);
    m_fontSizeSpin->setValue(10);
    m_fontSizeSpin->setSuffix(tr(" pt"));

    m_marginSpin = new QSpinBox(group);
    m_marginSpin->setRange(0, 64);
    m_marginSpin->setValue(8);
    m_marginSpin->setSuffix(tr(" px"));

    m_colorButton = new QPushButton(group);
    m_colorButton->setFixedHeight(22);
    m_colorButton->setStyleSheet(
        QString("background-color: %1;").arg(m_textColor.name()));
    m_colorButton->setToolTip(tr("Pick annotation text color"));

    form->addRow(m_enableCheck);
    form->addRow(tr("Position:"),   m_positionCombo);
    form->addRow(m_showSliceInfoCheck);
    form->addRow(m_showWindowLevelCheck);
    form->addRow(m_showSpacingCheck);
    form->addRow(m_showViewNameCheck);
    form->addRow(tr("Font size:"),  m_fontSizeSpin);
    form->addRow(tr("Margin:"),     m_marginSpin);
    form->addRow(tr("Text color:"), m_colorButton);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(group);

    connect(m_enableCheck, &QCheckBox::toggled,
            this, &ControllerPanelCornerAnnotationItem::_onEnabledToggled);
    connect(m_positionCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ControllerPanelCornerAnnotationItem::_onPositionChanged);
    connect(m_showSliceInfoCheck, &QCheckBox::toggled,
            this, &ControllerPanelCornerAnnotationItem::_onShowSliceInfoToggled);
    connect(m_showWindowLevelCheck, &QCheckBox::toggled,
            this, &ControllerPanelCornerAnnotationItem::_onShowWindowLevelToggled);
    connect(m_showSpacingCheck, &QCheckBox::toggled,
            this, &ControllerPanelCornerAnnotationItem::_onShowSpacingToggled);
    connect(m_showViewNameCheck, &QCheckBox::toggled,
            this, &ControllerPanelCornerAnnotationItem::_onShowViewNameToggled);
    connect(m_fontSizeSpin, qOverload<int>(&QSpinBox::valueChanged),
            this, &ControllerPanelCornerAnnotationItem::_onFontSizeChanged);
    connect(m_marginSpin, qOverload<int>(&QSpinBox::valueChanged),
            this, &ControllerPanelCornerAnnotationItem::_onMarginChanged);
    connect(m_colorButton, &QPushButton::clicked,
            this, &ControllerPanelCornerAnnotationItem::_onColorPicked);
}

void ControllerPanelCornerAnnotationItem::_onEnabledToggled(bool checked) {
    for (auto* ov : m_overlays)
        ov->SetEnabled(checked);
    emit CornerAnnotationEnableChanged(checked);
}

void ControllerPanelCornerAnnotationItem::_onPositionChanged(int index) {
    const auto pos = static_cast<overlays::OverlayPosition>(index);
    for (auto* ov : m_overlays)
        ov->SetPosition(pos);
    emit CornerAnnotationPositionChanged(pos);
}

void ControllerPanelCornerAnnotationItem::_onShowSliceInfoToggled(bool checked) {
    for (auto* ov : m_overlays)
        ov->SetShowSliceInfo(checked);
    emit CornerAnnotationShowSliceInfoChanged(checked);
}

void ControllerPanelCornerAnnotationItem::_onShowWindowLevelToggled(bool checked) {
    for (auto* ov : m_overlays)
        ov->SetShowWindowLevel(checked);
    emit CornerAnnotationShowWindowLevelChanged(checked);
}

void ControllerPanelCornerAnnotationItem::_onShowSpacingToggled(bool checked) {
    for (auto* ov : m_overlays)
        ov->SetShowSpacing(checked);
    emit CornerAnnotationShowSpacingChanged(checked);
}

void ControllerPanelCornerAnnotationItem::_onShowViewNameToggled(bool checked) {
    for (auto* ov : m_overlays)
        ov->SetShowViewName(checked);
    emit CornerAnnotationShowViewNameChanged(checked);
}

void ControllerPanelCornerAnnotationItem::_onFontSizeChanged(int pt) {
    for (auto* ov : m_overlays)
        ov->SetFontSize(pt);
    emit CornerAnnotationFontSizeChanged(pt);
}

void ControllerPanelCornerAnnotationItem::_onMarginChanged(int px) {
    for (auto* ov : m_overlays)
        ov->SetMargin(px);
    emit CornerAnnotationMarginChanged(px);
}

void ControllerPanelCornerAnnotationItem::_onColorPicked() {
    QColor color = QColorDialog::getColor(m_textColor, this, tr("Select Annotation Color"));
    if (!color.isValid())
        return;
    m_textColor = color;
    m_colorButton->setStyleSheet(
        QString("background-color: %1;").arg(color.name()));
    for (auto* ov : m_overlays)
        ov->SetTextColor(color);
    emit CornerAnnotationColorChanged(color);
}

}  // namespace ui
