#include "adapters/DicomSeriesModel.hpp"

#include <vtkDICOMDirectory.h>
#include <vtkDICOMMetaData.h>
#include <vtkDICOMReader.h>
#include <vtkImageData.h>
#include <vtkStringArray.h>

#include <QDir>

namespace adapters {

DicomSeriesModel::DicomSeriesModel(QObject* parent)
    : QAbstractListModel(parent) {
}

DicomSeriesModel::~DicomSeriesModel() = default;

int DicomSeriesModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_series.size());
}

QVariant DicomSeriesModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(m_series.size()))
        return {};

    const auto& entry = m_series[static_cast<size_t>(index.row())];
    if (role == Qt::DisplayRole) {
        const QString desc = entry.description.empty()
                                 ? QString("Series %1").arg(index.row() + 1)
                                 : QString::fromStdString(entry.description);
        return QString("%1 (%2 files)").arg(desc).arg(entry.fileCount);
    }
    if (role == Qt::ToolTipRole)
        return QString::fromStdString(entry.directory);
    return {};
}

void DicomSeriesModel::ScanDirectory(const QString& directory) {
    beginResetModel();
    m_series.clear();
    endResetModel();

    const std::string stdDir = directory.toStdString();

    auto dir = vtkSmartPointer<vtkDICOMDirectory>::New();
    dir->SetDirectoryName(stdDir.c_str());
    dir->SetScanDepth(8);
    dir->Update();

    const int numStudies = dir->GetNumberOfStudies();
    for (int s = 0; s < numStudies; ++s) {
        const int first = dir->GetFirstSeriesForStudy(s);
        const int last = dir->GetLastSeriesForStudy(s);
        for (int ser = first; ser <= last; ++ser) {
            vtkStringArray* files = dir->GetFileNamesForSeries(ser);
            if (!files || files->GetNumberOfValues() == 0)
                continue;

            SeriesEntry entry;
            entry.fileCount = static_cast<int>(files->GetNumberOfValues());
            entry.directory = stdDir;

            vtkDICOMMetaData* meta = dir->GetMetaDataForSeries(ser);
            if (meta && meta->Has(DC::SeriesDescription))
                entry.description = meta->Get(DC::SeriesDescription).AsString();
            if (meta && meta->Has(DC::SeriesInstanceUID))
                entry.uid = meta->Get(DC::SeriesInstanceUID).AsString();

            m_series.push_back(std::move(entry));
        }
    }

    beginResetModel();
    endResetModel();
    emit scanCompleted(static_cast<int>(m_series.size()));
}

void DicomSeriesModel::LoadSeries(int index) {
    if (index < 0 || index >= static_cast<int>(m_series.size())) {
        emit errorOccurred("Geçersiz seri indeksi.");
        return;
    }

    const auto& entry = m_series[static_cast<size_t>(index)];

    auto dir = vtkSmartPointer<vtkDICOMDirectory>::New();
    dir->SetDirectoryName(entry.directory.c_str());
    dir->SetScanDepth(8);
    dir->Update();

    // Find series by UID
    vtkStringArray* files = nullptr;
    const int numStudies = dir->GetNumberOfStudies();
    for (int s = 0; s < numStudies && !files; ++s) {
        const int first = dir->GetFirstSeriesForStudy(s);
        const int last = dir->GetLastSeriesForStudy(s);
        for (int ser = first; ser <= last && !files; ++ser) {
            vtkDICOMMetaData* meta = dir->GetMetaDataForSeries(ser);
            if (!meta)
                continue;
            const std::string uid = meta->Has(DC::SeriesInstanceUID)
                                        ? meta->Get(DC::SeriesInstanceUID).AsString()
                                        : std::string{};
            if (uid == entry.uid || entry.uid.empty()) {
                files = dir->GetFileNamesForSeries(ser);
            }
        }
    }

    if (!files || files->GetNumberOfValues() == 0) {
        emit errorOccurred("Seri dosyaları bulunamadı.");
        return;
    }

    auto reader = vtkSmartPointer<vtkDICOMReader>::New();
    reader->SetFileNames(files);
    reader->Update();

    m_imageData = vtkSmartPointer<vtkImageData>::New();
    m_imageData->DeepCopy(reader->GetOutput());

    emit imageDataLoaded(m_imageData);
}

int DicomSeriesModel::GetSeriesCount() const {
    return static_cast<int>(m_series.size());
}

vtkImageData* DicomSeriesModel::GetImageData() const {
    return m_imageData;
}

}  // namespace adapters
