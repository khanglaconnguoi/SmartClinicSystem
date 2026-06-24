/**
 * @file    PatientTableModel.cpp
 * @brief   Implementation cho PatientTableModel.
 */
#include "PatientTableModel.h"

PatientTableModel::PatientTableModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

void PatientTableModel::setPatients(const QList<Patient>& patients) {
    // Qt yêu cầu model reset để view cập nhật đúng khi thay đổi cấu trúc
    beginResetModel();
    m_patients = patients;
    endResetModel();
}

std::optional<Patient> PatientTableModel::patientAt(int row) const {
    if (row < 0 || row >= m_patients.size()) {
        return std::nullopt;
    }
    return m_patients.at(row);
}

int PatientTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_patients.size();
}

int PatientTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return ColCount;
}

QVariant PatientTableModel::data(const QModelIndex& index,
                                  int role) const {
    if (!index.isValid() || index.row() >= m_patients.size()) {
        return {};
    }

    const auto& patient = m_patients.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case ColId:          return patient.id();
            case ColFullName:    return patient.fullName();
            case ColBirthDate:
                return patient.birthDate().toString("dd/MM/yyyy");
            case ColGender:      return genderToString(patient.gender());
            case ColPhoneNumber: return patient.phoneNumber();
            case ColCitizenId:   return patient.citizenId();
            case ColEmail:       return patient.email();
            case ColInsurance:   return patient.insurance();
            case ColAddress:     return patient.address();
            case ColState:       return patient.stateName();
            default:             return {};
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColId) {
            return Qt::AlignCenter;
        }
    }

    return {};
}

QVariant PatientTableModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }

    switch (section) {
        case ColId:          return tr("ID");
        case ColFullName:    return tr("Họ và Tên");
        case ColBirthDate:   return tr("Ngày Sinh");
        case ColGender:      return tr("Giới Tính");
        case ColPhoneNumber: return tr("Số ĐT");
        case ColCitizenId:   return tr("CCCD");
        case ColEmail:       return tr("Email");
        case ColInsurance:   return tr("Bảo Hiểm");
        case ColAddress:     return tr("Địa Chỉ");
        case ColState:       return tr("Trạng Thái");
        default:             return {};
    }
}

QString PatientTableModel::genderToString(Gender gender) const {
    switch (gender) {
        case Gender::Male:   return tr("Nam");
        case Gender::Female: return tr("Nữ");
        case Gender::Other:  return tr("Khác");
        default:             return tr("Khác");
    }
}
