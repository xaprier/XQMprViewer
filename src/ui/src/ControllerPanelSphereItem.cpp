#include "ui/ControllerPanelSphereItem.hpp"

#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace ui {

ControllerPanelSphereItem::ControllerPanelSphereItem(QWidget* parent)
    : QWidget(parent), m_color(Qt::red) {
    _setupUi();
}

ControllerPanelSphereItem::~ControllerPanelSphereItem() = default;

double ControllerPanelSphereItem::GetRadius() const {
    return m_radiusSpin ? m_radiusSpin->value() : 3.0;
}

QColor ControllerPanelSphereItem::GetColor() const {
    return m_color;
}

void ControllerPanelSphereItem::_setupUi() {
    auto* group = new QGroupBox(tr("Sphere"), this);
    auto* form = new QFormLayout(group);
    form->setSpacing(4);

    m_enableButton = new QPushButton(tr("Add/Remove Sphere"), this);

    auto* radiusRow = new QHBoxLayout;
    auto* radiusLabel = new QLabel(tr("Radius"), this);
    m_radiusSpin = new QDoubleSpinBox(this);
    m_radiusSpin->setRange(0.5, 50.0);
    m_radiusSpin->setDecimals(1);
    m_radiusSpin->setSingleStep(0.5);
    m_radiusSpin->setValue(3.0);
    radiusRow->addWidget(radiusLabel);
    radiusRow->addWidget(m_radiusSpin, 1);

    m_colorButton = new QPushButton(tr("Color"), this);
    m_colorButton->setAutoFillBackground(true);
    m_colorButton->setStyleSheet(QString("background-color: %1").arg(m_color.name()));

    form->addRow(m_enableButton);
    form->addRow(radiusRow);
    form->addRow(m_colorButton);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(group);

    connect(m_enableButton, &QPushButton::clicked, this, &ControllerPanelSphereItem::SphereAddRemoveClicked);
    connect(m_radiusSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ControllerPanelSphereItem::SphereRadiusChanged);
    connect(m_colorButton, &QPushButton::clicked, this, [this]() {
        const QColor selected = QColorDialog::getColor(m_color, this, tr("Sphere Color"));
        if (!selected.isValid()) {
            return;
        }

        m_color = selected;
        m_colorButton->setStyleSheet(QString("background-color: %1").arg(m_color.name()));
        emit SphereColorChanged(m_color);
    });
}

}  // namespace ui
