// File: model/medical_record.h
#pragma once
#include "Patient.h"
#include <QString>
#include <QDateTime>
#include <vector>

struct Diagnosis {
    QString icdCode;        // Mã ICD-10
    QString description;
    QString severity;       // mild / moderate / severe
    bool    isPrimary;
};

struct VitalSigns {
    double  temperature_celcius;    // °C
    int     bloodPressureSystolic;
    int     bloodPressureDiastolic;
    int     heartRate;      // bpm
    double  weight;         // kg
    double  height;         // cm
};

class MedicalRecord {
private:
    int                 m_recordId;
    int                 m_patientId;
    int                 m_doctorId;
    int                 m_departmentId;
    PatientType         m_encouterType;
    QDateTime           m_visitDateTime;
    VitalSigns          m_vitals;
    QString             m_chiefComplaint;    // Lý do đến khám
    QString             m_clinicalNotes;     // Ghi chú lâm sàng
    QList<Diagnosis>    m_diagnoses;
    QString             m_treatmentPlan;
    QDateTime           m_nextVisitDate;

public:
    explicit MedicalRecord(
        int                 recordId,
        int                 patientId,
        int                 doctorId,
        QDateTime           visitDateTime,
        VitalSigns          vitals,
        QString             chiefComplaint,    // Lý do đến khám
        QString             clinicalNotes,     // Ghi chú lâm sàng
        QList<Diagnosis>    diagnoses,
        QString             treatmentPlan,
        QDateTime           nextVisitDate
    ) : 
        m_recordId(recordId),
        m_patientId(patientId),
        m_doctorId(doctorId),
        m_visitDateTime(visitDateTime),
        m_vitals(vitals),
        m_chiefComplaint(chiefComplaint),
        m_clinicalNotes(clinicalNotes),
        m_diagnoses(diagnoses),
        m_treatmentPlan(treatmentPlan),
        m_nextVisitDate(nextVisitDate)
    {}
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
    bool isComplete() const;
    double calculateBMI() const;
    QString generateSummary() const;
};
