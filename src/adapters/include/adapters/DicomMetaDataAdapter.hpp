#ifndef DICOMMETADATAADAPTER_HPP
#define DICOMMETADATAADAPTER_HPP

#include <QObject>
#include <QPair>
#include <QString>
#include <QVector>

namespace controllers {
class DicomController;
}  // namespace controllers

namespace adapters {

/**
 * @brief Bridge between DicomController and any Qt widget that needs metadata rows.
 *
 * Listens to DicomController::ImageDataReady, extracts the relevant DICOM tags
 * from the reader, and emits MetaDataReady with a flat {field, value} list.
 *
 * Widgets receive DicomMetaDataAdapter* — they have no knowledge of
 * DicomController or any VTK type.
 */
class DicomMetaDataAdapter : public QObject {
    Q_OBJECT

  public:
    explicit DicomMetaDataAdapter(controllers::DicomController* controller,
                                  QObject* parent = nullptr);

  Q_SIGNALS:
    /** @brief Emitted whenever a new series is ready. Carries the full metadata row list. */
    void MetaDataReady(QVector<QPair<QString, QString>> rows);

  private slots:
    void _onImageDataReady();

  private:
    controllers::DicomController* m_controller{nullptr};
};

}  // namespace adapters

#endif  // DICOMMETADATAADAPTER_HPP
