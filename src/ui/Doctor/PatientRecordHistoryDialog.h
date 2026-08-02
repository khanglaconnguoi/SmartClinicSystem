#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <memory>

#include "dto/PatientDTOs.h"
#include "dto/PrescriptionDTOs.h"

#include "service/MedicalRecordService.h"
#include "service/PharmacyService.h"

/**
 * @class PatientRecordHistoryDialog
 * @brief Hộp thoại cho phép Bác sĩ xem toàn bộ lịch sử ca khám, chẩn đoán, sinh hiệu và đơn thuốc cũ của Bệnh nhân.
 */
class PatientRecordHistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit PatientRecordHistoryDialog(
        std::shared_ptr<PharmacyService> pharmacyService = nullptr,
        std::shared_ptr<MedicalRecordService> medicalRecordService = nullptr,
        QWidget *parent = nullptr
    );
    virtual ~PatientRecordHistoryDialog() override = default;

    /**
     * @brief Nạp toàn bộ lịch sử ca khám & đơn thuốc cũ của Bệnh nhân theo patientId / patientCode
     */
    void loadPatientHistory(int patientId, const QString &patientName, const QString &patientCode);

private slots:
    void onRecordSelected(int row, int column);

private:
    void setupUI();

    std::shared_ptr<PharmacyService> m_pharmacyService;
    std::shared_ptr<MedicalRecordService> m_medicalRecordService;
    int m_currentPatientId = 0;
    QList<MedicalRecordResultDTO> m_records;
    QList<PrescriptionResultDTO> m_prescriptions;

    // Header info labels
    QLabel       *lblPatientHeader;

    // Bảng danh sách ca khám quá khứ (Trái)
    QTableWidget *tblRecordList;

    // Chi tiết ca khám được chọn (Phải)
    QLineEdit    *txtRecordDate;
    QLineEdit    *txtDoctorName;
    QLineEdit    *txtDiagnosis;
    QTextEdit    *txtReason;
    QTextEdit    *txtClinicalExam;
    QTextEdit    *txtTreatmentAdvice;

    // Sinh hiệu ca khám cũ
    QLineEdit    *txtPulse;
    QLineEdit    *txtBloodPressure;
    QLineEdit    *txtTemperature;
    QLineEdit    *txtBMI;

    // Bảng đơn thuốc đã kê trong ca khám đó
    QTableWidget *tblOldPrescription;

    QPushButton  *btnClose;
};
