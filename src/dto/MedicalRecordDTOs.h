#pragma once

#include "model/MedicalRecord.h"
#include <QDateTime>
#include <QList>
#include <QString>
#include <optional>

struct MedicalRecordInsertDTO {
  int patientId;
  int doctorId;
  std::optional<int> appointmentId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  std::optional<QDate> nextVisitDate;
  QList<Diagnosis> diagnoses;
};

struct MedicalRecordResultDTO {
  int recordId;
  int patientId;
  int doctorId;
  std::optional<int> appointmentId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  std::optional<QDate> nextVisitDate;
  QList<Diagnosis> diagnoses;
};

struct MedicalRecordUpdateDTO {
  int recordId;
  int doctorId;
  std::optional<int> appointmentId;
  QDateTime visitDateTime;
  VitalSigns vitals;
  QString chiefComplaint;
  QString clinicalNotes;
  QString treatment;
  std::optional<QDate> nextVisitDate;
  QList<Diagnosis> diagnoses;
};