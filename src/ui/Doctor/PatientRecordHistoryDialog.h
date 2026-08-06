#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include "PatientRecordHistoryWidget.h"

class PatientRecordHistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit PatientRecordHistoryDialog(
        std::shared_ptr<PharmacyService> pharmacyService = nullptr,
        std::shared_ptr<MedicalRecordService> medicalRecordService = nullptr,
        QWidget *parent = nullptr
    );
    ~PatientRecordHistoryDialog() override = default;

    void loadPatientHistory(int patientId, const QString &patientName, const QString &patientCode);

private:
    PatientRecordHistoryWidget* m_widget;
};
