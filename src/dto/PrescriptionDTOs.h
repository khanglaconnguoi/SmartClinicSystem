#pragma once

#include <QString>

// dto/PrescriptionDTOs.h — bạn của bạn tạo file, nhưng CẢ HAI cùng chốt nội
// dung
struct PrescriptionItemDTO {
  int medicationId;
  QString medicationName; // snapshot tên thuốc tại thời điểm kê
  int quantity;
  double unitPrice; // snapshot giá tại thời điểm kê — KHÔNG lấy giá live từ kho
  QString dosage;   // "1 viên/lần"
  QString frequency; // "2 lần/ngày"
  int durationDays;
  QString note;
};