#include "repository/AnalyticRepository.h"

#include <QSqlQuery>
#include <QVariantList>

#include "repository/DatabaseManager.h"

QList<DoctorMetricRawData> AnalyticRepository::getDoctorRawMetrics(
        const QDate& startDate, const QDate& endDate) const {
    QList<DoctorMetricRawData> resultList;

    // 1. Lấy danh sách bác sĩ
    QString doctorSql = R"(
      SELECT staff_id, full_name
      FROM staff
      WHERE role = 'DOCTOR' AND is_deleted = 0
      ORDER BY staff_id ASC
  )";

    QSqlQuery queryDoctor = DatabaseManager::getInstance().selectQuery(doctorSql, {});
    QMap<int, DoctorMetricRawData> doctorMap;

    while (queryDoctor.next()) {
        DoctorMetricRawData data;
        data.doctorId = queryDoctor.value(0).toInt();
        data.doctorName = queryDoctor.value(1).toString();
        data.patientCount = 0;
        data.consultationIncome = 0.0;
        doctorMap.insert(data.doctorId, data);
    }

    if (doctorMap.isEmpty()) { return resultList; }

    QString startStr = startDate.toString("yyyy-MM-dd") + " 00:00:00";
    QString endStr = endDate.toString("yyyy-MM-dd") + " 23:59:59";

    // 2. Thống kê số lượng bệnh nhân khám theo bác sĩ (medical_records)
    QString visitSql = R"(
      SELECT doctor_id, COUNT(record_id) AS visit_count
      FROM medical_records
      WHERE is_deleted = 0
        AND visit_datetime >= ?
        AND visit_datetime <= ?
      GROUP BY doctor_id
  )";

    QSqlQuery queryVisit = DatabaseManager::getInstance().selectQuery(visitSql, {startStr, endStr});
    while (queryVisit.next()) {
        int docId = queryVisit.value(0).toInt();
        int count = queryVisit.value(1).toInt();
        if (doctorMap.contains(docId)) { doctorMap[docId].patientCount = count; }
    }

    // 3. Thống kê tiền khám từ hóa đơn đã thanh toán (invoices.consultation_fee)
    QString feeSql = R"(
      SELECT mr.doctor_id, SUM(inv.consultation_fee) AS total_consultation_fee
      FROM invoices inv
      JOIN medical_records mr ON inv.record_id = mr.record_id
      WHERE inv.status = 'PAID'
        AND mr.is_deleted = 0
        AND mr.visit_datetime >= ?
        AND mr.visit_datetime <= ?
      GROUP BY mr.doctor_id
  )";

    QSqlQuery queryFee = DatabaseManager::getInstance().selectQuery(feeSql, {startStr, endStr});
    while (queryFee.next()) {
        int docId = queryFee.value(0).toInt();
        double income = queryFee.value(1).toDouble();
        if (doctorMap.contains(docId)) { doctorMap[docId].consultationIncome = income; }
    }

    return doctorMap.values();
}

SpecialityDistDTO AnalyticRepository::getSpecialityDistribution(
        const QDate& startDate, const QDate& endDate) const {
    SpecialityDistDTO dto;
    if (!startDate.isValid() || !endDate.isValid() || startDate > endDate) { return dto; }

    QString sql = R"(
        SELECT dp.specialty, COUNT(mr.record_id) AS visit_count
        FROM doctor_profiles dp
        JOIN staff s ON dp.staff_id = s.staff_id
        LEFT JOIN medical_records mr ON dp.staff_id = mr.doctor_id
            AND mr.is_deleted = 0
            AND mr.visit_datetime >= ?
            AND mr.visit_datetime <= ?
        WHERE s.is_deleted = 0
        GROUP BY dp.specialty
        ORDER BY visit_count DESC, dp.specialty ASC
    )";

    QString startStr = startDate.toString("yyyy-MM-dd") + " 00:00:00";
    QString endStr = endDate.toString("yyyy-MM-dd") + " 23:59:59";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {startStr, endStr});
    while (query.next()) {
        QString specialty = query.value(0).toString();
        int count = query.value(1).toInt();
        dto.distribution.append(qMakePair(specialty, count));
    }

    return dto;
}

PatientStatsDTO AnalyticRepository::getPatientStats(
        const QDate& startDate, const QDate& endDate) const {
    PatientStatsDTO dto;
    if (!startDate.isValid() || !endDate.isValid() || startDate > endDate) { return dto; }

    QString sql = R"(
        SELECT date(visit_datetime) AS visit_date, COUNT(record_id) AS patient_count
        FROM medical_records
        WHERE is_deleted = 0
          AND visit_datetime >= ?
          AND visit_datetime <= ?
        GROUP BY date(visit_datetime)
        ORDER BY visit_date ASC
    )";

    QString startStr = startDate.toString("yyyy-MM-dd") + " 00:00:00";
    QString endStr = endDate.toString("yyyy-MM-dd") + " 23:59:59";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {startStr, endStr});
    QMap<QDate, int> map;
    while (query.next()) {
        QDate dt = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
        int count = query.value(1).toInt();
        if (dt.isValid()) { map.insert(dt, count); }
    }

    for (QDate d = startDate; d <= endDate; d = d.addDays(1)) {
        int cnt = map.value(d, 0);
        dto.patientsPerDate.append(qMakePair(d, cnt));
        dto.total += cnt;
    }

    return dto;
}

IncomeStatsDTO AnalyticRepository::getIncomeStats(
        const QDate& startDate, const QDate& endDate) const {
    IncomeStatsDTO dto;
    if (!startDate.isValid() || !endDate.isValid() || startDate > endDate) { return dto; }

    QString sql = R"(
        SELECT date(COALESCE(paid_date, issued_date)) AS invoice_date, SUM(total_amount) AS daily_income
        FROM invoices
        WHERE status = 'PAID'
          AND COALESCE(paid_date, issued_date) >= ?
          AND COALESCE(paid_date, issued_date) <= ?
        GROUP BY date(COALESCE(paid_date, issued_date))
        ORDER BY invoice_date ASC
    )";

    QString startStr = startDate.toString("yyyy-MM-dd") + " 00:00:00";
    QString endStr = endDate.toString("yyyy-MM-dd") + " 23:59:59";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {startStr, endStr});
    QMap<QDate, double> map;
    while (query.next()) {
        QDate dt = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
        double income = query.value(1).toDouble();
        if (dt.isValid()) { map.insert(dt, income); }
    }

    for (QDate d = startDate; d <= endDate; d = d.addDays(1)) {
        double inc = map.value(d, 0.0);
        dto.incomePerDate.append(qMakePair(d, inc));
        dto.total += inc;
    }

    return dto;
}

WaitTimeStatsDTO AnalyticRepository::getWaitTimeStats(
        const QDate& startDate, const QDate& endDate) const {
    WaitTimeStatsDTO dto;
    if (!startDate.isValid() || !endDate.isValid() || startDate > endDate) { return dto; }

    QString sql = R"(
        SELECT 
            date(mr.visit_datetime) AS visit_date,
            SUM(MAX(0, (julianday(COALESCE(app.started_at, mr.visit_datetime)) - julianday(COALESCE(app.checked_in_at, app.created_at, app.appointment_date || ' ' || app.start_time))) * 1440.0)) AS total_wait_minutes,
            COUNT(CASE WHEN app.started_at IS NOT NULL OR app.checked_in_at IS NOT NULL OR app.created_at IS NOT NULL OR (app.appointment_date IS NOT NULL AND app.start_time IS NOT NULL) THEN mr.record_id END) AS wait_count
        FROM medical_records mr
        JOIN appointments app ON mr.appointment_id = app.appointment_id
        WHERE mr.is_deleted = 0
          AND mr.visit_datetime >= ?
          AND mr.visit_datetime <= ?
        GROUP BY date(mr.visit_datetime)
        ORDER BY visit_date ASC
    )";

    QString startStr = startDate.toString("yyyy-MM-dd") + " 00:00:00";
    QString endStr = endDate.toString("yyyy-MM-dd") + " 23:59:59";

    QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, {startStr, endStr});
    QMap<QDate, double> map;
    double totalWaitMinutes = 0.0;
    int totalVisitsWithWait = 0;

    while (query.next()) {
        QDate dt = QDate::fromString(query.value(0).toString(), "yyyy-MM-dd");
        double dailyTotalWait = query.value(1).toDouble();
        int validCnt = query.value(2).toInt();
        if (dt.isValid()) {
            double dailyAvg = (validCnt > 0) ? (dailyTotalWait / validCnt) : 0.0;
            map.insert(dt, dailyAvg);
            totalWaitMinutes += dailyTotalWait;
            totalVisitsWithWait += validCnt;
        }
    }

    for (QDate d = startDate; d <= endDate; d = d.addDays(1)) {
        double wait = map.value(d, 0.0);
        dto.waitTimePerDate.append(qMakePair(d, wait));
    }

    if (totalVisitsWithWait > 0) {
        dto.avg = totalWaitMinutes / totalVisitsWithWait;
    } else {
        dto.avg = 0.0;
    }

    return dto;
}
