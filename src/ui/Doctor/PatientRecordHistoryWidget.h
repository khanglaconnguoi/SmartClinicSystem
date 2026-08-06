#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <memory>

#include "dto/PatientDTOs.h"
#include "dto/PrescriptionDTOs.h"
#include "service/MedicalRecordService.h"
#include "service/PharmacyService.h"

class PatientRecordHistoryWidget : public QWidget {
    Q_OBJECT

public:
    explicit PatientRecordHistoryWidget(
        std::shared_ptr<PharmacyService> pharmacyService = nullptr,
        std::shared_ptr<MedicalRecordService> medicalRecordService = nullptr,
        QWidget *parent = nullptr
    );
    ~PatientRecordHistoryWidget() override = default;

    void loadPatientHistory(int patientId, const QString &patientName, const QString &patientCode);

private slots:
    void onRecordSelected(int row, int column);

private:
    void setupUI();

    std::shared_ptr<PharmacyService> m_pharmacyService;
    std::shared_ptr<MedicalRecordService> m_medicalRecordService;
    int m_currentPatientId = 0;
    QList<MedicalRecordResultDTO> m_records;
    QList<PrescriptionResultDTO> m_prescriptions;

    QTableWidget *tblRecordList;
    QLineEdit    *txtRecordDate;
    QLineEdit    *txtDoctorName;
    QLineEdit    *txtDiagnosis;
    QTextEdit    *txtTreatmentAdvice;
    QTableWidget *tblOldPrescription;
};
