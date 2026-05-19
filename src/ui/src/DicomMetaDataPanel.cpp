#include "ui/DicomMetaDataPanel.hpp"

#include <QHeaderView>
#include <QTableView>
#include <QVBoxLayout>

#include "adapters/DicomMetaDataAdapter.hpp"
#include "ui/DicomMetaDataModel.hpp"

namespace ui {

DicomMetaDataPanel::DicomMetaDataPanel(adapters::DicomMetaDataAdapter* adapter, QWidget* parent)
    : QWidget(parent) {
    _setupUi();

    connect(adapter, &adapters::DicomMetaDataAdapter::MetaDataReady,
            this, &DicomMetaDataPanel::_onMetaDataReady);
}

void DicomMetaDataPanel::_setupUi() {
    m_model = new DicomMetaDataModel(this);

    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableView->setShowGrid(false);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    layout->addWidget(m_tableView);
}

void DicomMetaDataPanel::_onMetaDataReady(QVector<QPair<QString, QString>> rows) {
    m_model->Refresh(std::move(rows));
}

}  // namespace ui
