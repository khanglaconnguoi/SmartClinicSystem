#include "PatientDetailDialog.h"

PatientDetailDialog::PatientDetailDialog(
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<PharmacyService> pharmacyService,
    std::shared_ptr<MedicalRecordService> medicalRecordService,
    int patientId,
    QWidget *parent)
    : QDialog(parent),
      m_patientService(patientService),
      m_pharmacyService(pharmacyService),
      m_medicalRecordService(medicalRecordService),
      m_patientId(patientId)
{
    setWindowTitle("Thông tin bệnh nhân");
    setMinimumSize(1200, 800);
    setStyleSheet("QDialog { background-color: #F8FAFC; font-family: 'Segoe UI'; }");
    
    setupUi();
    loadData();
}

void PatientDetailDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    m_lblHeader = new QLabel("THÔNG TIN BỆNH NHÂN", this);
    m_lblHeader->setStyleSheet("font-size: 18px; font-weight: bold; color: #0F172A; border: none;");
    m_lblHeader->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_lblHeader);

    m_infoWidget = new PatientInfoWidget(this);
    m_infoWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_historyWidget = new PatientRecordHistoryWidget(m_pharmacyService, m_medicalRecordService, this);

    mainLayout->addWidget(m_infoWidget);
    mainLayout->addWidget(m_historyWidget);
}

void PatientDetailDialog::loadData() {
    if (!m_patientService) return;

    auto detailOpt = m_patientService->getPatientById(m_patientId);
    if (detailOpt.has_value()) {
        const auto& detail = detailOpt.value();
        m_infoWidget->loadPatientData(detail);
        m_historyWidget->loadPatientHistory(m_patientId, detail.fullName, detail.patientCode);
    }
}
