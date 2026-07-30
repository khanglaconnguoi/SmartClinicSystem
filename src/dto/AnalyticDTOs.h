#pragma once

#include <QDate>
#include <QList>
#include <QPair>
#include <QString>

struct PatientStatsDTO {
    int total = 0;
    QList<QPair<QDate, int>> patientsPerDate;
};

struct IncomeStatsDTO {
    double total = 0.0;
    QList<QPair<QDate, double>> incomePerDate;
};

struct WaitTimeStatsDTO {
    double avg = 0.0;
    QList<QPair<QDate, double>> waitTimePerDate;  // Wait time in minutes
};

struct SpecialityDistDTO {
    QList<QPair<QString, int>> distribution;
};

struct DoctorKPI {
    QString name;
    int patientCount = 0;
    double income = 0.0;
    double kpi = 0.0;  // Range: 0.0 - 1.0 (0% - 100%)
};
