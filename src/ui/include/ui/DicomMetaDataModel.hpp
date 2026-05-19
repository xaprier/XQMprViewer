#ifndef DICOMMETADATAMODEL_HPP
#define DICOMMETADATAMODEL_HPP

#include <QAbstractTableModel>
#include <QPair>
#include <QString>
#include <QVector>

namespace ui {

/**
 * @brief Qt table adapter exposing DICOM metadata rows to a QTableView.
 *
 * Rows are loaded by calling Refresh() with a flat list of {field, value}
 * pairs sourced from DicomController::GetMetaData(). The model notifies
 * attached views via the standard QAbstractTableModel reset mechanism.
 */
class DicomMetaDataModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    explicit DicomMetaDataModel(QObject* parent = nullptr);

    /** @brief Replace the current rows and notify attached views. */
    void Refresh(QVector<QPair<QString, QString>> rows);

    /** @brief Clear all rows (e.g. before a new series is loaded). */
    void Clear();

    // QAbstractTableModel interface
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                     int role = Qt::DisplayRole) const override;

  private:
    QVector<QPair<QString, QString>> m_rows;
};

}  // namespace ui

#endif  // DICOMMETADATAMODEL_HPP
