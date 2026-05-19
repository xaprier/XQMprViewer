#include "adapters/DicomMetaDataAdapter.hpp"

#include <algorithm>

#include <vtkDICOMMetaData.h>
#include <vtkDICOMReader.h>
#include <vtkDICOMTag.h>

#include "controllers/DicomController.hpp"

namespace adapters {

DicomMetaDataAdapter::DicomMetaDataAdapter(controllers::DicomController* controller,
                                           QObject* parent)
    : QObject(parent), m_controller(controller) {
    connect(m_controller, &controllers::DicomController::ImageDataReady,
            this, &DicomMetaDataAdapter::_onImageDataReady);
}

void DicomMetaDataAdapter::_onImageDataReady() {
    vtkDICOMMetaData* meta = m_controller->GetReader()
                                 ? m_controller->GetReader()->GetMetaData()
                                 : nullptr;
    if (!meta) {
        emit MetaDataReady({});
        return;
    }

    auto get = [&](vtkDICOMTag tag) -> QString {
        if (!meta->Has(tag))
            return {};
        return QString::fromStdString(meta->Get(tag).AsString());
    };

    QVector<QPair<QString, QString>> rows;
    rows.reserve(15);

    // Patient
    rows.append({tr("Patient Name"),      get(DC::PatientName)});
    rows.append({tr("Patient ID"),        get(DC::PatientID)});
    rows.append({tr("Patient Sex"),       get(DC::PatientSex)});
    rows.append({tr("Patient Age"),       get(DC::PatientAge)});
    rows.append({tr("Birth Date"),        get(DC::PatientBirthDate)});

    // Study
    rows.append({tr("Study Date"),        get(DC::StudyDate)});
    rows.append({tr("Study Description"), get(DC::StudyDescription)});
    rows.append({tr("Study ID"),          get(DC::StudyID)});

    // Series
    rows.append({tr("Series Description"),get(DC::SeriesDescription)});
    rows.append({tr("Series Number"),     get(DC::SeriesNumber)});

    // Acquisition / equipment
    rows.append({tr("Modality"),          get(DC::Modality)});
    rows.append({tr("Manufacturer"),      get(DC::Manufacturer)});
    rows.append({tr("Institution Name"),  get(DC::InstitutionName)});
    rows.append({tr("Slice Thickness"),   get(DC::SliceThickness)});
    rows.append({tr("Pixel Spacing"),     get(DC::PixelSpacing)});

    rows.erase(std::remove_if(rows.begin(), rows.end(),
                              [](const QPair<QString, QString>& r) { return r.second.isEmpty(); }),
               rows.end());

    emit MetaDataReady(rows);
}

}  // namespace adapters
