#pragma once

#include <QDate>
#include <QList>
#include <QPair>
#include <QString>
#include <QTime>
#include <memory>

#include "dto/AppointmentDTOs.h"
#include "repository/AppointmentRepository.h"
#include "repository/PatientRepository.h"
#include "repository/StaffRepository.h"

class AppointmentService {
   private:
    std::shared_ptr<AppointmentRepository> m_appointmentRepository;
    std::shared_ptr<PatientRepository> m_patientRepository;
    std::shared_ptr<StaffRepository> m_staffRepository;

    // Normalization helper
    static void normalizeAppointmentInput(AppointmentInputDTO& input);

    // Aggregate validator
    QString validateAppointmentInput(const AppointmentInputDTO& input, int excludeId = -1) const;

   public:
    explicit AppointmentService(std::shared_ptr<AppointmentRepository> appointmentRepo,
            std::shared_ptr<PatientRepository> patientRepo = nullptr,
            std::shared_ptr<StaffRepository> staffRepo = nullptr)
        : m_appointmentRepository(std::move(appointmentRepo)),
          m_patientRepository(std::move(patientRepo)),
          m_staffRepository(std::move(staffRepo)) {}

    // Public static format validators (reusable by UI)
    static QString validateAppointmentDate(const QDate& date);
    static QString validateStartTime(const QTime& startTime);
    static QString validateEndTime(const QTime& endTime, const QTime& startTime);
    static QString validateAppointmentStatus(const QString& status);

    // Public non-static DB/existence/conflict validators
    QString validatePatientExists(int patientId) const;
    QString validateDoctorExists(int doctorId) const;
    QString validateDoctorConflict(int doctorId,
            const QDate& date,
            const QTime& startTime,
            const QTime& endTime,
            int excludeId = -1) const;
    QString validatePatientConflict(int patientId,
            const QDate& date,
            const QTime& startTime,
            const QTime& endTime,
            int excludeId = -1) const;

    QList<AppointmentRecordDTO> getDoctorAppointments(int doctorId, const QDate& date = QDate()) const;
    // QList<AppointmentRecordDTO> getDoctorAppointments(
    //         const QString& doctorId, const QDate& date) const;
    QList<AppointmentRecordDTO> getPatientAppointments(int patientId) const;
    QList<AppointmentRecordDTO> getAppointmentsByDate(const QDate& date) const;
    QString updateAppointmentStatus(int appointmentId, const QString& status) const;
    QPair<QString, int> checkInPatient(int appointmentId) const;
    QString cancelAppointment(int appointmentId) const;

    QString createAppointment(AppointmentInputDTO input) const;
    // bool createAppointment(const AppointmentInputDTO& input) const;
};
