/**
 * @file    PatientTableModel.h
 * @brief   Model cho QTableView hiển thị danh sách bệnh nhân.
 */
#pragma once

#include <QAbstractTableModel>
#include <QList>
#include "model/Patient.h"

/**
 * @brief Qt Table Model hiển thị danh sách bệnh nhân.
 *
 * Kế thừa QAbstractTableModel để cung cấp dữ liệu cho QTableView.
 * Tuân thủ Convention §3.1 về thứ tự khai báo trong class.
 */
class PatientTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    /**
     * @brief Các cột hiển thị trong bảng.
     */
    enum Column {
        ColId = 0,
        ColFullName,
        ColBirthDate,
        ColGender,
        ColPhoneNumber,
        ColCitizenId,
        ColEmail,
        ColInsurance,
        ColAddress,
        ColCount  // Tổng số cột
    };

    explicit PatientTableModel(QObject* parent = nullptr);
    ~PatientTableModel() override = default;

    /**
     * @brief Cập nhật toàn bộ dữ liệu trong model.
     * @param patients Danh sách Patient mới.
     */
    void setPatients(const QList<Patient>& patients);

    /**
     * @brief Lấy Patient tại dòng chỉ định.
     * @param row Chỉ số dòng (0-indexed).
     * @return Patient nếu row hợp lệ, std::nullopt nếu không.
     */
    std::optional<Patient> patientAt(int row) const;

    // --- QAbstractTableModel overrides ---
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    /**
     * @brief Chuyển enum Gender thành chuỗi hiển thị.
     */
    QString genderToString(Gender gender) const;

    QList<Patient> m_patients;
};
