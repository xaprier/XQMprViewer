#include "controllers/DicomController.hpp"

#include <vtkDICOMDirectory.h>
#include <vtkDICOMFileSorter.h>
#include <vtkDICOMImageReader.h>
#include <vtkDICOMMetaData.h>
#include <vtkDICOMReader.h>
#include <vtkDICOMToRAS.h>
#include <vtkImageData.h>
#include <vtkStringArray.h>

#include <QMetaObject>
#include <QThread>
#include <QTimer>
#include <string>
#include <vector>

namespace controllers {

DicomController::DicomController(QObject* parent)
    : IControllerBase(parent) {
    m_dir = vtkSmartPointer<vtkDICOMDirectory>::New();
    m_reader = vtkSmartPointer<vtkDICOMReader>::New();
    m_imageReader = vtkSmartPointer<vtkDICOMImageReader>::New();
    m_dicomToRAS = vtkSmartPointer<vtkDICOMToRAS>::New();
    m_fileSorter = vtkSmartPointer<vtkDICOMFileSorter>::New();
}

DicomController::~DicomController() {
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
        m_workerThread = nullptr;
    }
}

void DicomController::LoadDicom(const std::string& directory) {
    if (directory.empty()) {
        emit StatusChanged(tr("DICOM directory empty."));
        return;
    }
    if (m_workerThread && m_workerThread->isRunning()) {
        emit StatusChanged(tr("DICOM loading already in progress."));
        return;
    }

    m_currentDirectory = directory;
    m_currentSeries = -1;
    m_task = Task::ScanDirectory;
    emit StatusChanged(tr("Reading DICOM directory..."));
    _run();
}

void DicomController::LoadSeries(int seriesIndex) {
    if (m_currentDirectory.empty()) {
        emit StatusChanged(tr("First load a DICOM directory."));
        return;
    }
    if (seriesIndex < 0) {
        emit StatusChanged(tr("Invalid DICOM series index."));
        return;
    }
    if (m_workerThread && m_workerThread->isRunning()) {
        emit StatusChanged(tr("DICOM loading already in progress."));
        return;
    }

    m_currentSeries = seriesIndex;
    m_task = Task::LoadSeries;
    emit StatusChanged(tr("Loading DICOM series..."));
    _run();
}

const std::vector<DicomController::SeriesInfo>& DicomController::GetSeries() const {
    return m_series;
}

vtkDICOMReader* DicomController::GetReader() const {
    return m_reader.GetPointer();
}

void DicomController::_run() {
    const std::string directory = m_currentDirectory;
    const Task task = m_task;
    const int seriesIndex = m_currentSeries;

    m_workerThread = QThread::create([this, directory, task, seriesIndex]() {
        auto dir = vtkSmartPointer<vtkDICOMDirectory>::New();
        dir->SetDirectoryName(directory.c_str());
        dir->SetScanDepth(8);
        dir->Update();

        std::vector<SeriesInfo> seriesList;
        int flatSeriesIndex = 0;
        const int studyCount = dir->GetNumberOfStudies();
        for (int study = 0; study < studyCount; ++study) {
            const int firstSeries = dir->GetFirstSeriesForStudy(study);
            const int lastSeries = dir->GetLastSeriesForStudy(study);
            for (int series = firstSeries; series <= lastSeries; ++series) {
                vtkStringArray* candidateFiles = dir->GetFileNamesForSeries(series);
                if (!candidateFiles || candidateFiles->GetNumberOfValues() == 0) {
                    continue;
                }

                SeriesInfo info;
                info.index = flatSeriesIndex;
                info.dicomSeriesIndex = series;
                info.directory = directory;
                info.fileCount = static_cast<int>(candidateFiles->GetNumberOfValues());

                vtkDICOMMetaData* meta = dir->GetMetaDataForSeries(series);
                if (meta && meta->Has(DC::SeriesDescription)) {
                    info.description = meta->Get(DC::SeriesDescription).AsString();
                }
                if (meta && meta->Has(DC::SeriesInstanceUID)) {
                    info.uid = meta->Get(DC::SeriesInstanceUID).AsString();
                }

                seriesList.push_back(std::move(info));
                ++flatSeriesIndex;
            }
        }

        if (task == Task::ScanDirectory) {
            QMetaObject::invokeMethod(
                this,
                [this, dir, seriesList = std::move(seriesList)]() mutable {
                    m_dir = dir;
                    m_series = std::move(seriesList);
                    emit SeriesListReady(static_cast<int>(m_series.size()));

                    if (m_series.empty()) {
                        emit StatusChanged(tr("DICOM series not found."));
                        return;
                    }

                    emit StatusChanged(tr("%1 DICOM serise found.").arg(m_series.size()));
                    if (m_series.size() == 1) {
                        QTimer::singleShot(0, this, [this]() {
                            LoadSeries(0);
                        });
                    }
                },
                Qt::QueuedConnection);
            return;
        }

        if (seriesIndex < 0 || seriesIndex >= static_cast<int>(seriesList.size())) {
            QMetaObject::invokeMethod(
                this,
                [this]() {
                    emit StatusChanged(tr("Invalid DICOM series index."));
                },
                Qt::QueuedConnection);
            return;
        }

        vtkStringArray* files = nullptr;
        const int dicomSeriesIndex = seriesList[static_cast<size_t>(seriesIndex)].dicomSeriesIndex;
        files = dir->GetFileNamesForSeries(dicomSeriesIndex);

        if (!files || files->GetNumberOfValues() == 0) {
            QMetaObject::invokeMethod(
                this,
                [this]() {
                    emit StatusChanged(tr("DICOM series not found."));
                },
                Qt::QueuedConnection);
            return;
        }

        auto reader = vtkSmartPointer<vtkDICOMReader>::New();
        reader->SetFileNames(files);
        reader->Update();

        auto imageData = vtkSmartPointer<vtkImageData>::New();
        imageData->DeepCopy(reader->GetOutput());

        QMetaObject::invokeMethod(
            this,
            [this, dir, reader, imageData]() {
                m_dir = dir;
                m_reader = reader;
                m_imageData = imageData;
                emit ImageDataReady(m_imageData);
                emit StatusChanged(tr("DICOM data ready."));
            },
            Qt::QueuedConnection);
    });

    QThread* workerThread = m_workerThread;
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, this, [this, workerThread]() {
        if (m_workerThread == workerThread) {
            m_workerThread = nullptr;
        }
    });
    workerThread->start();
}

}  // namespace controllers
