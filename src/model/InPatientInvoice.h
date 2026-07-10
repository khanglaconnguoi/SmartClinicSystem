#pragma once

#include "Invoice.h"

/**
 * @brief   Lớp hóa đơn dành cho bệnh nhân nội trú.
 */
class InPatientInvoice : public Invoice {
public:
    /**
     * @brief   Khởi tạo hóa đơn nội trú.
     * @param   patientId        ID của bệnh nhân.
     * @param   consultationFee  Phí khám bệnh.
     * @param   medicationFee    Phí thuốc.
     * @param   issuedDate       Ngày phát hành hóa đơn.
     */
    explicit InPatientInvoice(int patientId, double consultationFee, double medicationFee, const QDate& issuedDate);

    /**
     * @brief   Tính tổng tiền hóa đơn.
     * @return  Tổng số tiền (khám + thuốc). Chưa tính phụ phí giường bệnh.
     */
    double calculate() const override;

    /**
     * @brief   Trả về dòng tóm tắt hóa đơn hiển thị UI.
     * @return  Chuỗi tóm tắt hóa đơn.
     */
    QString toSummaryString() const override;
};
