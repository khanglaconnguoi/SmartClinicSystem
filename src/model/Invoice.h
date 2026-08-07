#pragma once

#include "CommonEnums.h"
#include <QDate>
#include <QString>
#include <optional>

/**
 * @brief   Lớp cơ sở trừu tượng cho Hóa đơn.
 */
class Invoice {
protected:
    int m_invoiceId;
    QString m_invoiceCode;
    int m_patientId;
    std::optional<int> m_recordId;
    double m_consultationFee;
    double m_medicationFee;
    QDate m_issuedDate;
    QString m_status; // UNPAID / PAID / CANCELLED

public:
    /**
     * @brief   Khởi tạo một hóa đơn mới.
     * @param   patientId        ID của bệnh nhân.
     * @param   consultationFee  Phí khám bệnh.
     * @param   medicationFee    Phí thuốc.
     * @param   issuedDate       Ngày phát hành hóa đơn.
     */
    explicit Invoice(int patientId, double consultationFee, double medicationFee, const QDate& issuedDate);

    virtual ~Invoice() = default;

    // Getters
    int getInvoiceId() const { return m_invoiceId; }
    QString getInvoiceCode() const { return m_invoiceCode; }
    int getPatientId() const { return m_patientId; }
    std::optional<int> getRecordId() const { return m_recordId; }
    double getConsultationFee() const { return m_consultationFee; }
    double getMedicationFee() const { return m_medicationFee; }
    QDate getIssuedDate() const { return m_issuedDate; }
    QString getStatus() const { return m_status; }

    /**
     * @brief   Trả về loại hóa đơn (OUTPATIENT, INPATIENT, EMERGENCY).
     * @return  Chuỗi loại hóa đơn.
     */
    virtual QString getInvoiceType() const = 0;

    /**
     * @brief   Tính tổng tiền hóa đơn (khám + thuốc + phụ phí nếu có).
     * @return  Tổng số tiền.
     */
    virtual double calculate() const = 0;

    /**
     * @brief   Trả về 1 dòng tóm tắt hiển thị trên UI.
     * @return  Chuỗi tóm tắt hóa đơn.
     */
    virtual QString toSummaryString() const = 0;
};
