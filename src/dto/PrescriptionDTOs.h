#pragma once

#include <QString>
#include <QDateTime>
#include <optional>

struct PrescriptionActionInfoDTO {
    int       staffId = 0;
    QString   staffCode;
    QString   staffName;
    QDateTime actionAt;
    QString   reason;
};

struct PrescriptionItemDTO {
  int     medicationId;
  QString brandName; // snapshot tên thuốc tại thời điểm kê
  double  unitPrice; // snapshot giá tại thời điểm kê — KHÔNG lấy giá live từ kho
  int     quantity;
  QString dosage;   // "1 viên/lần"
  QString frequency; // "2 lần/ngày"
  int     durationDays;
  QString note;
};

// ── INPUT DTO — bác sĩ gửi toàn bộ đơn xuống Service ────────────────
// UI chỉ truyền medicationId, quantity, dosage, frequency, durationDays, note
// PharmacyService tự điền brandNamevà unitPrice (snapshot) từ Medication
struct PrescriptionInputDTO {
    int     recordId;   // Lấy từ kết quả MedicalRecordService::createMedicalRecord()
    int     doctorId;
    QString notes;      // Ghi chú cho dược sĩ
    QList<PrescriptionItemDTO> items;
};


struct PrescriptionSearchCriteria {
    QString patientName;  // Tìm kiếm gần đúng (LIKE) tên bệnh nhân
    QString doctorName;   // Tìm kiếm gần đúng (LIKE) tên bác sĩ kê đơn
    QString keyword;// Ô tìm kiếm nhanh chung (Mã đơn, chẩn đoán, ghi chú...)

    int recordId = 0;     // Lọc chính xác theo Mã đơn thuốc (hoặc Mã bệnh án)
    int doctorId = 0;     // Lọc tất cả đơn thuốc do 1 bác sĩ cụ thể kê


    QDateTime fromDate;   // Lọc đơn từ ngày...
    QDateTime toDate;     // ...đến ngày

    QString status;       
};

// ── RESULT DTO — đọc từ DB để hiển thị ──────────────────────────────
struct PrescriptionResultDTO {
    int       prescriptionId;
    int       recordId;

    QString   patientName;      
    int       patientAge;       
    QString   patientGender;    
    QString   diagnosis;

    int       doctorId;
    QString   doctorCode;
    QString   doctorName;
    
    QString   status;
    QString   notes;
    QDateTime prescribedAt;
    
    QList<PrescriptionItemDTO> items;
    double    totalAmount;

    std::optional<PrescriptionActionInfoDTO> dispensedInfo; // Có giá trị nếu status == "DISPENSED"
    std::optional<PrescriptionActionInfoDTO> cancelledInfo; // Có giá trị nếu status == "CANCELLED" (đã gồm lý do hủy bên trong)
};




