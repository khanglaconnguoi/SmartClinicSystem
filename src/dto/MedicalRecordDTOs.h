#pragma once

#include <QDateTime>
#include <QString>
#include <model/MedicalRecord.h>
#include <optional>


struct MedicalRecordInsertDTO {
  int patientId;
  int doctorId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QVector<Diagnosis> diagnoses;
  QString treatment;
  std::optional<QDate> nextVisitDate;
};

struct MedicalRecordResultDTO {
  int recordId;
  int patientId;
  int doctorId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QVector<Diagnosis> diagnoses;
  QString treatment;
};