#pragma once

#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

#include "dto/AnalyticDTOs.h"

struct DoctorMetricRawData {
    int doctorId = 0;
    QString doctorName;
    int patientCount = 0;
    double consultationIncome = 0.0;
};

class AnalyticRepository {
   public:
    AnalyticRepository() = default;
    ~AnalyticRepository() = default;

    /**
     * @brief Truy vấn dữ liệu thô (tên, số lượt khám, doanh thu tiền khám)
     *        của tất cả bác sĩ đang hoạt động trong khoảng ngày [startDate, endDate].
     */
    QList<DoctorMetricRawData> getDoctorRawMetrics(
            const QDate& startDate, const QDate& endDate) const;

    /**
     * @brief Truy vấn thống kê số lượng bệnh nhân khám theo chuyên khoa bác sĩ
     *        trong khoảng ngày [startDate, endDate].
     */
    SpecialityDistDTO getSpecialityDistribution(const QDate& startDate, const QDate& endDate) const;

    /**
     * @brief Truy vấn thống kê số lượt bệnh nhân khám theo từng ngày trong khoảng [startDate,
     * endDate].
     */
    PatientStatsDTO getPatientStats(const QDate& startDate, const QDate& endDate) const;

    /**
     * @brief Truy vấn thống kê tổng doanh thu phòng khám theo từng ngày trong khoảng [startDate,
     * endDate].
     */
    IncomeStatsDTO getIncomeStats(const QDate& startDate, const QDate& endDate) const;

    /**
     * @brief Truy vấn thống kê thời gian chờ trung bình của bệnh nhân theo từng ngày trong khoảng
     * [startDate, endDate].
     */
    WaitTimeStatsDTO getWaitTimeStats(const QDate& startDate, const QDate& endDate) const;
};
