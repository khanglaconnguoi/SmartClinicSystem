// File: model/medical_record.h
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
    double  temperature;    // °C
    int     bloodPressureSystolic;
    int     bloodPressureDiastolic;
    int     heartRate;      // bpm
    double  weight;         // kg
    double  height;         // cm
};

class MedicalRecord {
private:
    int             m_recordId;
    int             m_patientId;
    int             m_doctorId;
    QDateTime       m_visitDateTime;
    VitalSigns      m_vitals;
    QString         m_chiefComplaint;    // Lý do đến khám
    QString         m_clinicalNotes;     // Ghi chú lâm sàng
    std::vector<Diagnosis> m_diagnoses;
    QString         m_treatment;
    QDateTime       m_nextVisitDate;

public:
    explicit MedicalRecord(int patientId, int doctorId);
    ~MedicalRecord() = default;
    // --- Getters ---
    int           getRecordId()     const { return m_recordId; }
    int           getPatientId()    const { return m_patientId; }
    QDateTime     getVisitDateTime()const { return m_visitDateTime; }
    VitalSigns    getVitals()       const { return m_vitals; }
    QString       getClinicalNotes()const { return m_clinicalNotes; }

    // --- Setters ---
    void setVitals(const VitalSigns& vitals)    { m_vitals = vitals; }
    void setClinicalNotes(const QString& notes) { m_clinicalNotes = notes; }
    void addDiagnosis(const Diagnosis& diag)    { m_diagnoses.push_back(diag); }

    // --- Business ---
    // bool isComplete() const; // Kiểm tra đủ thông tin bắt buộc
    // double calculateBMI() const;
    // QString generateSummary() const;
};
