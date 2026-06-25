#pragma once
#include <QString>
#include <QDateTime>
#include <vector>

struct Diagnosis {
    QString icdCode;        // Mã ICD-10
    QString description;
    QString severity;       // mild / moderate / severe
};

struct VitalSigns {
    double  temperature = 37.0;    // °C
    int     bloodPressureSystolic = 120;
    int     bloodPressureDiastolic = 80;
    int     heartRate = 75;      // bpm
    double  weight = 60.0;         // kg
    double  height = 170.0;         // cm
};

class MedicalRecord {
private:
    int             m_recordId = -1;
    int             m_patientId = -1;
    int             m_doctorId = -1;
    QDateTime       m_visitDateTime;
    VitalSigns      m_vitals;
    QString         m_chiefComplaint;    // Lý do đến khám
    QString         m_clinicalNotes;     // Ghi chú lâm sàng
    std::vector<Diagnosis> m_diagnoses;
    QString         m_treatment;
    QString         m_testResults;
    QDateTime       m_nextVisitDate;

public:
    MedicalRecord() = default;
    explicit MedicalRecord(int patientId, int doctorId);

    // --- Getters ---
    int           getRecordId()     const { return m_recordId; }
    int           getPatientId()    const { return m_patientId; }
    int           getDoctorId()     const { return m_doctorId; }
    QDateTime     getVisitDateTime()const { return m_visitDateTime; }
    VitalSigns    getVitals()       const { return m_vitals; }
    QString       getClinicalNotes()const { return m_clinicalNotes; }
    QString       getChiefComplaint()const{ return m_chiefComplaint; }
    std::vector<Diagnosis> getDiagnoses() const { return m_diagnoses; }
    QString       getTreatment()    const { return m_treatment; }
    QString       getTestResults()  const { return m_testResults; }
    QDateTime     getNextVisitDate()const { return m_nextVisitDate; }

    // --- Setters ---
    void setRecordId(int id) { m_recordId = id; }
    void setVitals(const VitalSigns& vitals)    { m_vitals = vitals; }
    void setClinicalNotes(const QString& notes) { m_clinicalNotes = notes; }
    void setChiefComplaint(const QString& complaint) { m_chiefComplaint = complaint; }
    void addDiagnosis(const Diagnosis& diag)    { m_diagnoses.push_back(diag); }
    void setTreatment(const QString& treatment) { m_treatment = treatment; }
    void setTestResults(const QString& results) { m_testResults = results; }
    void setNextVisitDate(const QDateTime& d)   { m_nextVisitDate = d; }
    void setVisitDateTime(const QDateTime& d)   { m_visitDateTime = d; }

    // --- Business ---
    bool isComplete() const; // Kiểm tra đủ thông tin bắt buộc
    double calculateBMI() const;
    QString generateSummary() const;
};
