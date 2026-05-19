#ifndef DICOMCONTROLLER_HPP
#define DICOMCONTROLLER_HPP

#include <vtkSmartPointer.h>

#include <QPointer>
#include <string>
#include <vector>

#include "controllers/IControllerBase.hpp"

class vtkImageData;
class vtkDICOMDirectory;
class vtkDICOMReader;
class vtkDICOMImageReader;
class vtkDICOMToRAS;
class vtkDICOMFileSorter;
class QThread;

namespace controllers {

/**
 * @brief Scans a directory for DICOM series and loads them asynchronously.
 *
 * Call LoadDicom() to start a background scan; connect SeriesListReady to
 * learn the number of series found. Call LoadSeries() to read a specific
 * series and connect ImageDataReady to receive the resulting vtkImageData.
 */
class DicomController : public IControllerBase {
    Q_OBJECT

  public:
    /** @brief Metadata for a single DICOM series found during a directory scan. */
    struct SeriesInfo {
        int index{-1};
        int dicomSeriesIndex{-1};
        std::string uid;
        std::string description;
        std::string directory;
        int fileCount{0};
    };

    explicit DicomController(QObject* parent = nullptr);
    ~DicomController() override;

    /**
     * @brief Starts an asynchronous scan of @p directory for DICOM series.
     * Emits SeriesListReady when complete.
     */
    void LoadDicom(const std::string& directory);

    /**
     * @brief Loads the series at @p seriesIndex from the last scanned directory.
     * Emits ImageDataReady when the image is ready.
     */
    void LoadSeries(int seriesIndex);

    /** @brief Returns the series list populated by the last LoadDicom() call. */
    [[nodiscard]] const std::vector<SeriesInfo>& GetSeries() const;

    /** @brief Returns the underlying VTK reader after a series has been loaded, nullptr otherwise. */
    [[nodiscard]] vtkDICOMReader* GetReader() const;

  Q_SIGNALS:
    /** @brief Emitted after a directory scan completes. @param seriesCount Number of series found. */
    void SeriesListReady(int seriesCount);

    /** @brief Emitted after a series has been loaded. @param imageData The loaded image. */
    void ImageDataReady(vtkImageData* imageData);

  private:
    enum class Task {
        None,
        ScanDirectory,
        LoadSeries
    };

    void _run();

    Task m_task{Task::None};
    int m_currentSeries{-1};
    std::string m_currentDirectory;
    std::vector<SeriesInfo> m_series;
    vtkSmartPointer<vtkDICOMDirectory> m_dir;
    vtkSmartPointer<vtkDICOMReader> m_reader;
    vtkSmartPointer<vtkDICOMImageReader> m_imageReader;
    vtkSmartPointer<vtkDICOMToRAS> m_dicomToRAS;
    vtkSmartPointer<vtkDICOMFileSorter> m_fileSorter;
    vtkSmartPointer<vtkImageData> m_imageData;
    QPointer<QThread> m_workerThread;
};

}  // namespace controllers

#endif  // DICOMCONTROLLER_HPP
