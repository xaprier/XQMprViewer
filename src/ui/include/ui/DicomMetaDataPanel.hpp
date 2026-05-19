#ifndef DICOMMETADATAPANEL_HPP
#define DICOMMETADATAPANEL_HPP

#include <QPair>
#include <QString>
#include <QVector>
#include <QWidget>

class QTableView;

namespace adapters {
class DicomMetaDataAdapter;
}  // namespace adapters

namespace ui {
class DicomMetaDataModel;

/**
 * @brief Read-only panel that displays DICOM metadata for the active series.
 *
 * Receives a DicomMetaDataAdapter and connects to its MetaDataReady signal.
 * Has no knowledge of DicomController or any VTK type.
 */
class DicomMetaDataPanel : public QWidget {
    Q_OBJECT

  public:
    explicit DicomMetaDataPanel(adapters::DicomMetaDataAdapter* adapter,
                                QWidget* parent = nullptr);

  private slots:
    void _onMetaDataReady(QVector<QPair<QString, QString>> rows);

  private:
    void _setupUi();

    DicomMetaDataModel* m_model{nullptr};
    QTableView* m_tableView{nullptr};
};

}  // namespace ui

#endif  // DICOMMETADATAPANEL_HPP
