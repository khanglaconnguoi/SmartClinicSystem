#include "PatientRecordHistoryDialog.h"

PatientRecordHistoryDialog::PatientRecordHistoryDialog(
    std::shared_ptr<PharmacyService> pharmacyService,
    std::shared_ptr<MedicalRecordService> medicalRecordService,
    QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Lịch Sử Bệnh Án & Đơn Thuốc Bệnh Nhân");
    setMinimumSize(950, 650);
    setStyleSheet("QDialog { background-color: #F8FAFC; font-family: 'Segoe UI'; }");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_widget = new PatientRecordHistoryWidget(pharmacyService, medicalRecordService, this);
    layout->addWidget(m_widget);
}

void PatientRecordHistoryDialog::loadPatientHistory(int patientId, const QString &patientName, const QString &patientCode) {
    if (m_widget) {
        m_widget->loadPatientHistory(patientId, patientName, patientCode);
    }
}
