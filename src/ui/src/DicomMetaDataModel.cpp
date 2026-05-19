#include "ui/DicomMetaDataModel.hpp"

namespace ui {

DicomMetaDataModel::DicomMetaDataModel(QObject* parent)
    : QAbstractTableModel(parent) {}

void DicomMetaDataModel::Refresh(QVector<QPair<QString, QString>> rows) {
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
}

void DicomMetaDataModel::Clear() {
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

int DicomMetaDataModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_rows.size());
}

int DicomMetaDataModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return 2;
}

QVariant DicomMetaDataModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_rows.size())
        return {};

    const auto& row = m_rows[index.row()];

    if (role == Qt::DisplayRole) {
        return index.column() == 0 ? row.first : row.second;
    }

    if (role == Qt::ToolTipRole && index.column() == 1) {
        return row.second;
    }

    return {};
}

QVariant DicomMetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return {};

    return section == 0 ? tr("Field") : tr("Value");
}

}  // namespace ui
