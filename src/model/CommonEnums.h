#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

template <typename T> struct EnumInfo {
  T value;
  QString viText;
  QString enText;
};

// =====================================================================
// SECTION 1: REMOVED ENUMS - MAPPED TO QHASH TEXT (PLAN A)
// =====================================================================

namespace GenderText {
    // Sử dụng QList<QPair> kết hợp inline (C++17) để giữ nguyên thứ tự hiển thị và
    // tối ưu bộ nhớ
    inline const QList<QPair<QString, QString>> gender = {
        {"MALE", "Nam"}, {"FEMALE", "Nữ"}, {"OTHER", "Khác"}};

    // Hàm lấy danh sách hỗ trợ nạp thẳng QComboBox tại UI
    inline const QList<QPair<QString, QString>> &getList() { return gender; }

    inline bool isValid(const QString &inputText) {
    QString cleanInput = inputText.toUpper().trimmed();
    for (const auto &pair : gender) {
        if (pair.first == cleanInput || pair.second.toUpper() == cleanInput) {
        return true;
        }
    }
    return false;
    }

    inline QString toVi(const QString &en) {
    QString cleanEn = en.toUpper().trimmed();
    for (const auto &pair : gender) {
        if (pair.first == cleanEn) {
        return pair.second;
        }
    }
    return "Khác";
    }

    inline QString toEn(const QString &vi) {
    QString cleanVi = vi.trimmed();
    for (const auto &pair : gender) {
        if (pair.second.compare(cleanVi, Qt::CaseInsensitive) == 0) {
        return pair.first;
        }
    }
    return "OTHER"; // Giá trị mặc định an toàn
    }
} // namespace GenderText

namespace SeverityText {
    inline const QList<QPair<QString, QString>> severities = {
        {"MILD", "Nhẹ"}, {"MODERATE", "Trung bình"}, {"SEVERE", "Nặng"}};

    inline const QList<QPair<QString, QString>> &getList() { return severities; }

    inline bool isValid(const QString &inputText) {
    QString cleanInput = inputText.toUpper().trimmed();
    for (const auto &pair : severities) {
        if (pair.first == cleanInput || pair.second.toUpper() == cleanInput) {
        return true;
        }
    }
    return false;
    }

    inline QString toVi(const QString &en) {
    QString cleanEn = en.toUpper().trimmed();
    for (const auto &pair : severities) {
        if (pair.first == cleanEn) {
        return pair.second;
        }
    }
    return "Khác";
    }

    inline QString toEn(const QString &vi) {
    QString cleanVi = vi.trimmed();
    for (const auto &pair : severities) {
        if (pair.second.compare(cleanVi, Qt::CaseInsensitive) == 0) {
        return pair.first;
        }
    }
    return "MILD"; // Giá trị mặc định an toàn
    }

} // namespace SeverityText
namespace InsuraceTypeText {
    inline const QList<QPair<QString, QString>> insuraceTypes = {
        {"NATIONAL", "Bảo hiểm y tế xã hội"},
        {"COMMERCIAL", "Bảo hiểm y tế tư nhân"},
        {"OTHER", "Khác"}};

    inline const QList<QPair<QString, QString>> &getList() { return insuraceTypes; }

    inline bool isValid(const QString &inputText) {
    QString cleanInput = inputText.toUpper().trimmed();
    for (const auto &pair : insuraceTypes) {
        if (pair.first == cleanInput || pair.second.toUpper() == cleanInput) {
        return true;
        }
    }
    return false;
    }

    inline QString toVi(const QString &en) {
    QString cleanEn = en.toUpper().trimmed();
    for (const auto &pair : insuraceTypes) {
        if (pair.first == cleanEn) {
        return pair.second;
        }
    }
    return "Khác";
    }

    inline QString toEn(const QString &vi) {
    QString cleanVi = vi.trimmed();
    for (const auto &pair : insuraceTypes) {
        if (pair.second.compare(cleanVi, Qt::CaseInsensitive) == 0) {
        return pair.first;
        }
    }
    return "Bảo hiểm y tế xã hội"; // Giá trị mặc định an toàn
    }
} // namespace InsuraceTypeText

namespace MedicationCategoryText {
    inline const QList<QPair<QString, QString>> medicationCategories = {
        {"ANTIBIOTICS", "Kháng sinh"},
        {"ANALGESICS", "Giảm đau"},
        {"ANTIPYRETICS", "Hạ sốt"},
        {"CARDIOVASCULAR", "Tim mạch"},
        {"ANTIDIABETICS", "Trị tiểu đường"},
        {"VITAMINS", "Vitamin & thực phẩm chức năng"},
        {"RESPIRATORY", "Hô hấp"},
        {"GASTROINTESTINAL", "Tiêu hóa"},
        {"DERMATOLOGICAL", "Thuốc bôi da liễu"}};

    inline const QList<QPair<QString, QString>> &getList() {
    return medicationCategories;
    }

    inline bool isValid(const QString &inputText) {
    QString cleanInput = inputText.toUpper().trimmed();
    for (const auto &pair : medicationCategories) {
        if (pair.first == cleanInput || pair.second.toUpper() == cleanInput) {
        return true;
        }
    }
    return false;
    }

    inline QString toVi(const QString &en) {
    QString cleanEn = en.toUpper().trimmed();
    for (const auto &pair : medicationCategories) {
        if (pair.first == cleanEn)
        return pair.second;
    }
    return "Khác";
    }

    inline QString toEn(const QString &vi) {
    QString cleanVi = vi.trimmed();
    for (const auto &pair : medicationCategories) {
        if (pair.second.compare(cleanVi, Qt::CaseInsensitive) == 0)
        return pair.first;
    }
    return "VITAMINS";
    }
} // namespace MedicationCategoryText

namespace MedicationUnitText {
    // Sử dụng QList<QPair> kết hợp inline (C++17) giúp giữ nguyên thứ tự hiển thị
    // và tối ưu bộ nhớ
    inline const QList<QPair<QString, QString>> medicationUnits = {
        {"TABLET", "Viên"}, {"PACKET", "Gói"}, {"BOTTLE", "Chai"}, {"VIAL", "Lọ"},
        {"AMPOULE", "Ống"}, {"TUBE", "Tuýp"},  {"BLISTER", "Vỉ"},  {"BOX", "Hộp"}};

    // Hàm lấy danh sách hỗ trợ nạp thẳng QComboBox tại UI
    inline const QList<QPair<QString, QString>> &getList() {
    return medicationUnits;
    }

    inline bool isValid(const QString &inputText) {
    QString cleanInput = inputText.toUpper().trimmed();
    for (const auto &pair : medicationUnits) {
        if (pair.first == cleanInput || pair.second.toUpper() == cleanInput) {
        return true;
        }
    }
    return false;
    }

    inline QString toVi(const QString &en) {
    QString cleanEn = en.toUpper().trimmed();
    for (const auto &pair : medicationUnits) {
        if (pair.first == cleanEn) {
        return pair.second;
        }
    }
    return "Khác";
    }

    inline QString toEn(const QString &vi) {
    QString cleanVi = vi.trimmed();
    for (const auto &pair : medicationUnits) {
        if (pair.second.compare(cleanVi, Qt::CaseInsensitive) == 0) {
        return pair.first;
        }
    }
    return "TABLET"; // Giá trị mặc định an toàn
    }

} // namespace MedicationUnitText

namespace InvoiceStatusText {
    inline const QString UNPAID = "UNPAID";
    inline const QString PAID = "PAID";
    inline const QString CANCELLED = "CANCELLED";

    inline const QList<QPair<QString, QString>> statuses = {
        {UNPAID, "Chưa thanh toán"},
        {PAID, "Đã thanh toán"},
        {CANCELLED, "Đã hủy"}};

    inline const QList<QPair<QString, QString>> &getList() { return statuses; }

    inline bool isValid(const QString &inputText) {
    QString cleanInput = inputText.toUpper().trimmed();
    for (const auto &pair : statuses) {
        if (pair.first == cleanInput || pair.second.toUpper() == cleanInput) {
        return true;
        }
    }
    return false;
    }

    inline QString toVi(const QString &en) {
    QString cleanEn = en.toUpper().trimmed();
    for (const auto &pair : statuses) {
        if (pair.first == cleanEn)
        return pair.second;
    }
    return "Chưa thanh toán";
    }

    inline QString toEn(const QString &vi) {
    QString cleanVi = vi.trimmed();
    for (const auto &pair : statuses) {
        if (pair.second.compare(cleanVi, Qt::CaseInsensitive) == 0)
        return pair.first;
    }
    return UNPAID;
    }
} // namespace InvoiceStatusText

namespace AppointmentStatusText {
    inline const QString SCHEDULED  = "SCHEDULED";
    inline const QString CONFIRMED  = "CONFIRMED";
    inline const QString CHECKED_IN = "CHECKED_IN";
    inline const QString COMPLETED  = "COMPLETED";
    inline const QString CANCELLED  = "CANCELLED";
    inline const QString NO_SHOW    = "NO_SHOW";

    inline const QList<QPair<QString, QString>> statuses = {
        {SCHEDULED,  "Đã hẹn"},
        {CONFIRMED,  "Đã xác nhận"},
        {CHECKED_IN, "Đã check-in"},
        {COMPLETED,  "Đã khám"},
        {CANCELLED,  "Đã hủy"},
        {NO_SHOW,    "Vắng mặt"}};

    inline const QList<QPair<QString, QString>> &getList() { return statuses; }

    inline bool isValid(const QString &inputText) {
    QString cleanInput = inputText.toUpper().trimmed();
    for (const auto &pair : statuses) {
        if (pair.first == cleanInput || pair.second.toUpper() == cleanInput) {
        return true;
        }
    }
    return false;
    }

    inline QString toVi(const QString &en) {
    QString cleanEn = en.toUpper().trimmed();
    for (const auto &pair : statuses) {
        if (pair.first == cleanEn)
        return pair.second;
    }
    return "Đã hẹn";
    }

    inline QString toEn(const QString &vi) {
    QString cleanVi = vi.trimmed();
    for (const auto &pair : statuses) {
        if (pair.first == cleanVi.toUpper() || pair.second.compare(cleanVi, Qt::CaseInsensitive) == 0)
        return pair.first;
    }
    return SCHEDULED;
    }
} // namespace AppointmentStatusText

// =====================================================================
// SECTION 2: KEPT ENUMS (PATTERN CŨ)
// =====================================================================

// PatientType
enum class PatientType { Outpatient, Inpatient, Emergency };

static const QList<EnumInfo<PatientType>> patientTypeList = {
    {PatientType::Outpatient, "Ngoại trú", "OUTPATIENT"},
    {PatientType::Inpatient, "Nội trú", "INPATIENT"},
    {PatientType::Emergency, "Cấp cứu", "EMERGENCY"},
};

inline QString patientTypeToVi(PatientType value) {
  for (const auto &item : patientTypeList) {
    if (item.value == value)
      return item.viText;
  }
  return "Ngoại trú";
}

inline PatientType patientTypeFromVi(const QString &text) {
  for (const auto &item : patientTypeList) {
    if (item.viText == text)
      return item.value;
  }
  return PatientType::Outpatient;
}

inline QString patientTypeToEn(PatientType value) {
  for (const auto &item : patientTypeList) {
    if (item.value == value)
      return item.enText;
  }
  return "OUTPATIENT";
}

inline PatientType patientTypeFromEn(const QString &text) {
  for (const auto &item : patientTypeList) {
    if (item.enText == text)
      return item.value;
  }
  return PatientType::Outpatient;
}

// PatientPriority
enum class PatientPriority { Low, Normal, High, Critical };

static const QList<EnumInfo<PatientPriority>> patientPriorityList = {
    {PatientPriority::Low, "Thấp", "LOW"},
    {PatientPriority::Normal, "Bình thường", "NORMAL"},
    {PatientPriority::High, "Cao", "HIGH"},
    {PatientPriority::Critical, "Khẩn cấp", "CRITICAL"},
};

inline QString patientPriorityToVi(PatientPriority value) {
  for (const auto &item : patientPriorityList) {
    if (item.value == value)
      return item.viText;
  }
  return "Bình thường";
}

inline PatientPriority patientPriorityFromVi(const QString &text) {
  for (const auto &item : patientPriorityList) {
    if (item.viText == text)
      return item.value;
  }
  return PatientPriority::Normal;
}

inline QString patientPriorityToEn(PatientPriority value) {
  for (const auto &item : patientPriorityList) {
    if (item.value == value)
      return item.enText;
  }
  return "NORMAL";
}

inline PatientPriority patientPriorityFromEn(const QString &text) {
  for (const auto &item : patientPriorityList) {
    if (item.enText == text)
      return item.value;
  }
  return PatientPriority::Normal;
}

// OutPatientState
enum class OutPatientState {
  Registered,
  WaitingForTreatment,
  Treatment,
  Discharged
};

static const QList<EnumInfo<OutPatientState>> outPatientStateList = {
    {OutPatientState::Registered, "Đã đăng ký", "REGISTERED"},
    {OutPatientState::WaitingForTreatment, "Chờ điều trị",
     "WAITING FOR TREATMENT"},
    {OutPatientState::Treatment, "Đang điều trị", "TREATMENT"},
    {OutPatientState::Discharged, "Đã xuất viện", "DISCHARGED"},
};

inline QString outPatientStateToVi(OutPatientState value) {
  for (const auto &item : outPatientStateList) {
    if (item.value == value)
      return item.viText;
  }
  return "Đã đăng ký";
}

inline OutPatientState outPatientStateFromVi(const QString &text) {
  for (const auto &item : outPatientStateList) {
    if (item.viText == text)
      return item.value;
  }
  return OutPatientState::Registered;
}

inline QString outPatientStateToEn(OutPatientState value) {
  for (const auto &item : outPatientStateList) {
    if (item.value == value)
      return item.enText;
  }
  return "REGISTERED";
}

inline OutPatientState outPatientStateFromEn(const QString &text) {
  for (const auto &item : outPatientStateList) {
    if (item.enText == text)
      return item.value;
  }
  return OutPatientState::Registered;
}

// InPatientState
enum class InPatientState { Admitted, Discharged, Transferred };

static const QList<EnumInfo<InPatientState>> inPatientStateList = {
    {InPatientState::Admitted, "Đã nhập viện", "ADMITTED"},
    {InPatientState::Discharged, "Đã xuất viện", "DISCHARGED"},
    {InPatientState::Transferred, "Chuyển viện", "TRANSFERRED"},
};

inline QString inPatientStateToVi(InPatientState value) {
  for (const auto &item : inPatientStateList) {
    if (item.value == value)
      return item.viText;
  }
  return "Đã nhập viện";
}

inline InPatientState inPatientStateFromVi(const QString &text) {
  for (const auto &item : inPatientStateList) {
    if (item.viText == text)
      return item.value;
  }
  return InPatientState::Admitted;
}

inline QString inPatientStateToEn(InPatientState value) {
  for (const auto &item : inPatientStateList) {
    if (item.value == value)
      return item.enText;
  }
  return "ADMITTED";
}

inline InPatientState inPatientStateFromEn(const QString &text) {
  for (const auto &item : inPatientStateList) {
    if (item.enText == text)
      return item.value;
  }
  return InPatientState::Admitted;
}

// EmergencyPatientState
enum class EmergencyPatientState { Emergency, Discharged, Transferred };

static const QList<EnumInfo<EmergencyPatientState>> emergencyPatientStateList =
    {
        {EmergencyPatientState::Emergency, "Cấp cứu", "EMERGENCY"},
        {EmergencyPatientState::Discharged, "Đã xuất viện", "DISCHARGED"},
        {EmergencyPatientState::Transferred, "Chuyển viện", "TRANSFERRED"},
};

inline QString emergencyPatientStateToVi(EmergencyPatientState value) {
  for (const auto &item : emergencyPatientStateList) {
    if (item.value == value)
      return item.viText;
  }
  return "Cấp cứu";
}

inline EmergencyPatientState emergencyPatientStateFromVi(const QString &text) {
  for (const auto &item : emergencyPatientStateList) {
    if (item.viText == text)
      return item.value;
  }
  return EmergencyPatientState::Emergency;
}

inline QString emergencyPatientStateToEn(EmergencyPatientState value) {
  for (const auto &item : emergencyPatientStateList) {
    if (item.value == value)
      return item.enText;
  }
  return "EMERGENCY";
}

inline EmergencyPatientState emergencyPatientStateFromEn(const QString &text) {
  for (const auto &item : emergencyPatientStateList) {
    if (item.enText == text)
      return item.value;
  }
  return EmergencyPatientState::Emergency;
}



// UserRole
enum class UserRole { Admin, Doctor, Nurse, Receptionist, Pharmacist };

static const QList<EnumInfo<UserRole>> userRoleList = {
    {UserRole::Admin, "Quản trị viên", "ADMIN"},
    {UserRole::Doctor, "Bác sĩ", "DOCTOR"},
    {UserRole::Nurse, "Y tá", "NURSE"},
    {UserRole::Receptionist, "Lễ tân", "RECEPTIONIST"},
    {UserRole::Pharmacist, "Dược sĩ", "PHARMACIST"},

};

inline QString userRoleToVi(UserRole value) {
  for (const auto &item : userRoleList) {
    if (item.value == value)
      return item.viText;
  }
  return "Quản trị viên";
}

inline UserRole userRoleFromVi(const QString &text) {
  for (const auto &item : userRoleList) {
    if (item.viText == text)
      return item.value;
  }
  return UserRole::Admin;
}

inline QString userRoleToEn(UserRole value) {
  for (const auto &item : userRoleList) {
    if (item.value == value)
      return item.enText;
  }
  return "ADMIN";
}

inline UserRole userRoleFromEn(const QString &text) {
  for (const auto &item : userRoleList) {
    if (item.enText == text)
      return item.value;
  }
  return UserRole::Admin;
}

// Legacy compatibility for Staff
inline QString roleToString(UserRole role) { return userRoleToEn(role); }
inline UserRole roleFromString(const QString &roleStr) {
  return userRoleFromEn(roleStr);
}