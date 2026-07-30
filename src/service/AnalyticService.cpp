#include "service/AnalyticService.h"

#include <algorithm>

int AnalyticService::countWorkingDays(const QDate& start, const QDate& end) {
    if (!start.isValid() || !end.isValid() || start > end) { return 1; }
    int workingDays = 0;
    for (QDate d = start; d <= end; d = d.addDays(1)) {
        if (d.dayOfWeek() <= 5) {  // 1: Thứ hai -> 5: Thứ sáu
            workingDays++;
        }
    }
    return (workingDays > 0) ? workingDays : 1;
}

QList<DoctorKPI> AnalyticService::getDoctorsKPI(const QDate& start, const QDate& end) const {
    QList<DoctorKPI> resultList;
    if (!m_analyticRepo || !start.isValid() || !end.isValid() || start > end) { return resultList; }

    int workingDays = countWorkingDays(start, end);
    double patientTarget = static_cast<double>(workingDays * 8);
    double revenueTarget = static_cast<double>(workingDays * 800000.0);

    QList<DoctorMetricRawData> rawMetrics = m_analyticRepo->getDoctorRawMetrics(start, end);

    for (const auto& raw : rawMetrics) {
        DoctorKPI kpiDto;
        kpiDto.name = raw.doctorName;
        kpiDto.patientCount = raw.patientCount;
        kpiDto.income = raw.consultationIncome;

        double sVolume = std::min(1.0, static_cast<double>(raw.patientCount) / patientTarget);
        double sRevenue = std::min(1.0, raw.consultationIncome / revenueTarget);

        kpiDto.kpi = (0.50 * sVolume) + (0.50 * sRevenue);

        resultList.append(kpiDto);
    }

    return resultList;
}

SpecialityDistDTO AnalyticService::getSpecialityDistribution(
        const QDate& start, const QDate& end) const {
    if (!m_analyticRepo || !start.isValid() || !end.isValid() || start > end) { return {}; }
    return m_analyticRepo->getSpecialityDistribution(start, end);
}

PatientStatsDTO AnalyticService::getPatientStats(const QDate& start, const QDate& end) const {
    if (!m_analyticRepo || !start.isValid() || !end.isValid() || start > end) { return {}; }
    return m_analyticRepo->getPatientStats(start, end);
}

IncomeStatsDTO AnalyticService::getIncomeStats(const QDate& start, const QDate& end) const {
    if (!m_analyticRepo || !start.isValid() || !end.isValid() || start > end) { return {}; }
    return m_analyticRepo->getIncomeStats(start, end);
}

WaitTimeStatsDTO AnalyticService::getWaitTimeStats(const QDate& start, const QDate& end) const {
    if (!m_analyticRepo || !start.isValid() || !end.isValid() || start > end) { return {}; }
    return m_analyticRepo->getWaitTimeStats(start, end);
}
