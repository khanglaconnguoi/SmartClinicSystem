/**
 * @file    PatientFormDialog.h
 * @brief   Dialog form để thêm/sửa thông tin bệnh nhân.
 */
#pragma once

#include <QDialog>
#include "model/Patient.h"
#include "state/IPatientState.h"

class QLineEdit;
class QDateEdit;
class QComboBox;
class QPushButton;

/**
 * @brief Dialog form cho thêm mới / chỉnh sửa bệnh nhân.
 *
 * Chứa các widget input: QLineEdit, QDateEdit, QComboBox.
 * Trả về Patient đã điền thông tin qua getPatient().
 */
class PatientFormDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Khởi tạo form rỗng (chế độ Thêm mới).
     */
    explicit PatientFormDialog(QWidget* parent = nullptr);

    /**
     * @brief Khởi tạo form với dữ liệu có sẵn (chế độ Sửa).
     * @param patient Dữ liệu bệnh nhân cần hiển thị.
     */
    explicit PatientFormDialog(const Patient& patient,
                               QWidget* parent = nullptr);

    ~PatientFormDialog() override = default;

    /**
     * @brief Lấy đối tượng Patient từ dữ liệu trên form.
     * @return Patient chứa dữ liệu người dùng đã nhập.
     */
    Patient getPatient() const;

private slots:
    void handleSaveClicked();

private:
    void setupUi();
    void populateForm(const Patient& patient);

    QLineEdit*   m_editFullName;
    QDateEdit*   m_editBirthDate;
    QComboBox*   m_comboGender;
    QLineEdit*   m_editPhoneNumber;
    QLineEdit*   m_editAddress;
    QLineEdit*   m_editCitizenId;
    QLineEdit*   m_editEmail;
    QLineEdit*   m_editInsurance;
    QPushButton* m_btnSave;
    QPushButton* m_btnCancel;
    QComboBox*   m_comboState;

    int  m_patientId = -1;
    bool m_isActive = true;
    PatientStateType m_stateType = PatientStateType::Registered;
};
