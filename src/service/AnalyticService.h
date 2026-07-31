#pragma once

#include <QDate>
#include <QList>
#include <memory>

#include "dto/AnalyticDTOs.h"
#include "repository/AnalyticRepository.h"

class AnalyticService {
   private:
    std::shared_ptr<AnalyticRepository> m_analyticRepo;

    /**
     * @brief Đếm số ngày làm việc (Thứ 2 đến Thứ 6) trong khoảng [start, end].
     */
    static int countWorkingDays(const QDate& start, const QDate& end);

   public:
    explicit AnalyticService(std::shared_ptr<AnalyticRepository> analyticRepo)
        : m_analyticRepo(std::move(analyticRepo)) {}

    /**
     * @brief Tính toán chỉ số KPI cho danh sách bác sĩ dựa trên số lượng bệnh
     * nhân khám (tiêu chuẩn 8 bệnh nhân/ngày làm việc) và doanh thu tiền khám
     * (tiêu chuẩn 800.000 VNĐ/ngày làm việc).
     *
     * @param start Ngày bắt đầu khoảng thống kê.
     * @param end Ngày kết thúc khoảng thống kê.
     * @return QList<DoctorKPI> Danh sách kết quả KPI của bác sĩ.
     */
    QList<DoctorKPI> getDoctorsKPI(const QDate& start, const QDate& end) const;

    /**
     * @brief Thống kê phân bố bệnh nhân khám theo chuyên khoa bác sĩ
     * trong khoảng [start, end].
     *
     * @param start Ngày bắt đầu khoảng thống kê.
     * @param end Ngày kết thúc khoảng thống kê.
     * @return SpecialityDistDTO Thống kê chuyên khoa và số lượt khám tương ứng.
     */
    SpecialityDistDTO getSpecialityDistribution(const QDate& start, const QDate& end) const;

    /**
     * @brief Thống kê số lượt bệnh nhân khám theo từng ngày trong khoảng [start, end].
     *
     * @param start Ngày bắt đầu khoảng thống kê.
     * @param end Ngày kết thúc khoảng thống kê.
     * @return PatientStatsDTO Thống kê lượt khám tổng và chi tiết từng ngày.
     */
    PatientStatsDTO getPatientStats(const QDate& start, const QDate& end) const;

    /**
     * @brief Thống kê tổng doanh thu phòng khám theo từng ngày trong khoảng [start, end].
     *
     * @param start Ngày bắt đầu khoảng thống kê.
     * @param end Ngày kết thúc khoảng thống kê.
     * @return IncomeStatsDTO Thống kê doanh thu tổng và chi tiết từng ngày.
     */
    IncomeStatsDTO getIncomeStats(const QDate& start, const QDate& end) const;

    /**
     * @brief Thống kê thời gian chờ trung bình của bệnh nhân theo từng ngày trong khoảng [start,
     * end].
     *
     * @param start Ngày bắt đầu khoảng thống kê.
     * @param end Ngày kết thúc khoảng thống kê.
     * @return WaitTimeStatsDTO Thống kê thời gian chờ trung bình tổng và chi tiết từng ngày (tính
     * bằng phút).
     */
    WaitTimeStatsDTO getWaitTimeStats(const QDate& start, const QDate& end) const;
};
