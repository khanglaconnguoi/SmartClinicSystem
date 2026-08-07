#pragma once

#include "Invoice.h"

/**
 * @brief   Lớp hóa đơn dành cho bệnh nhân ngoại trú.
 */
class OutPatientInvoice : public Invoice {
public:
    /**
     * @brief   Khởi tạo hóa đơn ngoại trú.
     * @param   patientId        ID của bệnh nhân.
     * @param   consultationFee  Phí khám bệnh.
     * @param   medicationFee    Phí thuốc.
     * @param   issuedDate       Ngày phát hành hóa đơn.
     */
    explicit OutPatientInvoice(int patientId, double consultationFee, double medicationFee, const QDate& issuedDate);

    /**
     * @brief   Trả về loại hóa đơn ("OUTPATIENT").
     * @return  Chuỗi loại hóa đơn.
     */
    QString getInvoiceType() const override;

    /**
     * @brief   Tính tổng tiền hóa đơn.
     * @return  Tổng số tiền (khám + thuốc, không phụ phí).
     */
    double calculate() const override;

    /**
     * @brief   Trả về dòng tóm tắt hóa đơn hiển thị UI.
     * @return  Chuỗi tóm tắt hóa đơn.
     */
    QString toSummaryString() const override;
};
