#include "ui/ControllerPanelDicomItem.hpp"

#include <qboxlayout.h>

#include <QEasingCurve>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>

namespace ui {

ControllerPanelDicomItem::ControllerPanelDicomItem(QWidget* parent)
    : QWidget(parent) {
    _setupUi();
}

ControllerPanelDicomItem::~ControllerPanelDicomItem() = default;

void ControllerPanelDicomItem::SetSeries(const QStringList& seriesNames) {
    m_seriesList->clear();
    m_seriesList->addItems(seriesNames);

    const bool hasSeries = !seriesNames.isEmpty();
    m_seriesToggleButton->setEnabled(hasSeries);
    m_loadButton->setEnabled(false);
    m_selectedIndex = -1;
    m_selectedLabel->setText(tr("No series selected"));

    if (hasSeries) {
        _SetListVisible(true);
    }
}

void ControllerPanelDicomItem::_OnSeriesToggle() {
    _SetListVisible(!m_listExpanded);
}

void ControllerPanelDicomItem::_OnSeriesItemClicked(int row) {
    m_selectedIndex = row;
    const QString name = m_seriesList->item(row)->text();
    m_selectedLabel->setText(name);
    m_loadButton->setEnabled(true);
    _SetListVisible(false);
}

void ControllerPanelDicomItem::_setupUi() {
    auto* group = new QGroupBox(tr("DICOM"), this);
    auto* form = new QFormLayout(group);
    form->setSpacing(4);

    m_browseButton = new QPushButton(tr("Select Folder"), this);
    m_browseButton->setToolTip(tr("Select DICOM folder"));

    m_seriesToggleButton = new QPushButton(tr("Select Series ▼"), this);
    m_seriesToggleButton->setEnabled(false);
    m_seriesToggleButton->setToolTip(tr("Show/hide series list"));

    m_seriesList = new QListWidget(this);
    m_seriesList->setVisible(false);
    m_seriesList->setMaximumHeight(0);
    m_seriesList->setMinimumWidth(0);
    m_seriesList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_seriesList->setTextElideMode(Qt::ElideRight);
    m_seriesList->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_listAnimation = new QPropertyAnimation(m_seriesList, "maximumHeight", this);
    m_listAnimation->setDuration(200);
    m_listAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_listAnimation, &QPropertyAnimation::finished, this, [this]() {
        if (!m_listExpanded) {
            m_seriesList->hide();
        }
        m_seriesList->setMaximumHeight(m_listExpanded ? QWIDGETSIZE_MAX : 0);
    });

    m_selectedLabel = new QLabel(tr("No series selected"), this);
    m_selectedLabel->setWordWrap(true);
    m_selectedLabel->setStyleSheet("color: gray; font-size: 11px;");

    m_loadButton = new QPushButton(tr("Load"), this);
    m_loadButton->setToolTip(tr("Load selected series"));
    m_loadButton->setEnabled(false);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_browseButton);
    buttonLayout->addWidget(m_seriesToggleButton);
    buttonLayout->addStretch();

    form->addRow(buttonLayout);
    form->addRow(m_seriesList);
    form->addRow(tr("Selected Series:"), m_selectedLabel);
    form->addRow(m_loadButton);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(group);

    connect(m_browseButton, &QPushButton::clicked, this, [this]() {
        const QString directory = QFileDialog::getExistingDirectory(
            this, tr("Select DICOM Folder"), m_currentDirectory);
        if (directory.isEmpty()) {
            return;
        }
        m_currentDirectory = directory;
        m_browseButton->setToolTip(tr("Directory: %1").arg(directory));
        m_seriesList->blockSignals(true);
        m_seriesList->clear();
        m_seriesList->blockSignals(false);
        m_seriesToggleButton->setEnabled(false);
        m_loadButton->setEnabled(false);
        m_selectedIndex = -1;
        m_selectedLabel->setText(tr("No series selected"));
        _SetListVisible(false);
        emit DirectorySelected(directory);
    });

    connect(m_seriesToggleButton, &QPushButton::clicked,
            this, &ControllerPanelDicomItem::_OnSeriesToggle);

    connect(m_seriesList, &QListWidget::currentRowChanged,
            this, &ControllerPanelDicomItem::_OnSeriesItemClicked);

    connect(m_loadButton, &QPushButton::clicked, this, [this]() {
        if (m_selectedIndex >= 0) {
            emit SeriesLoadRequested(m_selectedIndex);
        }
    });
}

void ControllerPanelDicomItem::_SetListVisible(bool visible) {
    if (m_listExpanded == visible)
        return;
    m_listExpanded = visible;
    m_seriesToggleButton->setText(visible ? tr("Select Series ▲") : tr("Select Series ▼"));

    m_listAnimation->stop();
    const int startH = m_seriesList->maximumHeight() >= QWIDGETSIZE_MAX ? m_seriesList->height() : m_seriesList->maximumHeight();
    const int endH = visible ? 160 : 0;

    if (visible) {
        m_seriesList->setMaximumHeight(startH);
        m_seriesList->show();
    }

    m_listAnimation->setStartValue(startH);
    m_listAnimation->setEndValue(endH);
    m_listAnimation->start();
}

}  // namespace ui
