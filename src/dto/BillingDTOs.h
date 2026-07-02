#pragma once

#include "model/CommonEnums.h"
#include <QDate>
#include <QString>
#include <QList>
#include <optional>

struct InvoiceItemDTO {
    QString itemType;    // "CONSULTATION" / "MEDICATION" / "SERVICE"
    QString description;
    int quantity;
    double unitPrice;
    double subtotal;
};

struct InvoiceInsertDTO {
    int patientId;
    std::optional<int> recordId;
    PatientType patientType;
    double consultationFee;
    double medicationFee;
    double totalAmount;
    QDate issuedDate;
    QList<InvoiceItemDTO> items;
};

struct InvoiceResultDTO {
    int invoiceId;
    QString invoiceCode;
    int patientId;
    std::optional<int> recordId;
    PatientType patientType;
    double consultationFee;
    double medicationFee;
    double totalAmount;
    QString status;
    QDate issuedDate;
    std::optional<QDate> paidDate;
    QList<InvoiceItemDTO> items;
};

struct InvoiceUpdateDTO {
    int invoiceId;
    double consultationFee;
    double medicationFee;
    double totalAmount;
    QList<InvoiceItemDTO> items;
};
