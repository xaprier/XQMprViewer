#ifndef XQVtkViewport_DICOMSERIESMODEL_HPP
#define XQVtkViewport_DICOMSERIESMODEL_HPP

#include <vtkSmartPointer.h>

#include <QAbstractListModel>
#include <QString>
#include <string>
#include <vector>

class vtkDICOMDirectory;
class vtkImageData;

namespace adapters {

/**
 * @brief Exposes a DICOM series list as a Qt list model.
 *
 * Call ScanDirectory() to populate the model from a folder; call
 * LoadSeries() to read the selected series as a vtkImageData.
 */
class DicomSeriesModel : public QAbstractListModel {
    Q_OBJECT

  public:
    explicit DicomSeriesModel(QObject* parent = nullptr);
    ~DicomSeriesModel() override;

    // ── QAbstractListModel ───────────────────────────────────────────────
    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // ── Public API ───────────────────────────────────────────────────────
    /**
     * @brief Scans @p directory for DICOM series and populates the model.
     * @param directory Path to the directory to scan.
     */
    void ScanDirectory(const QString& directory);

    /**
     * @brief Loads the series at @p index and emits imageDataLoaded.
     * @param index Model row index.
     */
    void LoadSeries(int index);

    /** @brief Returns the number of series found in the last ScanDirectory() call. */
    [[nodiscard]] int GetSeriesCount() const;

    /**
     * @brief Returns the vtkImageData from the last LoadSeries() call.
     * @return nullptr if LoadSeries() has not been called yet.
     */
    [[nodiscard]] vtkImageData* GetImageData() const;

  Q_SIGNALS:
    void imageDataLoaded(vtkImageData* imageData);
    void scanCompleted(int seriesCount);
    void errorOccurred(const QString& message);

  private:
    struct SeriesEntry {
        std::string uid;
        std::string description;
        std::string directory;
        int fileCount{0};
    };

    std::vector<SeriesEntry> m_series;
    vtkSmartPointer<vtkImageData> m_imageData;
};

}  // namespace adapters

#endif  // XQVtkViewport_DICOMSERIESMODEL_HPP
