#include "MedicalRecord.h"

MedicalRecord::MedicalRecord(int patientId, int doctorId)
    : m_patientId(patientId), m_doctorId(doctorId),
      m_visitDateTime(QDateTime::currentDateTime()) {}

bool MedicalRecord::isComplete() const {
  return !m_chiefComplaint.isEmpty() && !m_clinicalNotes.isEmpty() &&
         !m_diagnoses.empty();
}

double MedicalRecord::calculateBMI() const {
  if (m_vitals.height <= 0)
    return 0;
  double heightInMeters = m_vitals.height / 100.0;
  return m_vitals.weight / (heightInMeters * heightInMeters);
}

QString MedicalRecord::generateSummary() const {
  QString summary = QString("Bệnh nhân cảm thấy: %1\n").arg(m_chiefComplaint);
  summary += QString("Ghi chú lâm sàng: %1\n").arg(m_clinicalNotes);
  if (!m_testResults.isEmpty()) {
      summary += QString("Kết quả xét nghiệm: %1\n").arg(m_testResults);
  }
  summary += "Chẩn đoán:\n";
  for (const auto &diag : m_diagnoses) {
    summary += QString("- [%1] %2 (%3)\n")
                   .arg(diag.icdCode, diag.description, diag.severity);
  }
  return summary;
}
