#pragma once

#include <QDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <memory>

#include "PatientInfoWidget.h"
#include "PatientRecordHistoryWidget.h"
#include "service/PatientService.h"
#include "service/PharmacyService.h"
#include "service/MedicalRecordService.h"

class PatientDetailDialog : public QDialog {
    Q_OBJECT

public:
    explicit PatientDetailDialog(
        std::shared_ptr<PatientService> patientService,
        std::shared_ptr<PharmacyService> pharmacyService,
        std::shared_ptr<MedicalRecordService> medicalRecordService,
        int patientId,
        QWidget *parent = nullptr
    );
    ~PatientDetailDialog() override = default;

private:
    void setupUi();
    void loadData();

    std::shared_ptr<PatientService> m_patientService;
    std::shared_ptr<PharmacyService> m_pharmacyService;
    std::shared_ptr<MedicalRecordService> m_medicalRecordService;
    int m_patientId;

    QLabel* m_lblHeader;

    PatientInfoWidget* m_infoWidget;
    PatientRecordHistoryWidget* m_historyWidget;
};
