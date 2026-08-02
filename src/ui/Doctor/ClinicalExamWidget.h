#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

#include <memory>
#include "dto/PatientDTOs.h"
#include "service/MedicalRecordService.h"
#include "service/PharmacyService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"

class ClinicalExamWidget : public QWidget {
    Q_OBJECT

public:
    explicit ClinicalExamWidget(std::shared_ptr<MedicalRecordService> medicalRecordService = nullptr, QWidget* parent = nullptr);
    virtual ~ClinicalExamWidget() override = default;

    void setServices(std::shared_ptr<PharmacyService> pharmacyService, std::shared_ptr<PatientService> patientService, std::shared_ptr<AppointmentService> appointmentService);

    // Hàm load thông tin bệnh nhân khi bác sĩ chọn khám
    void loadPatientInfo(int patientId, int appointmentId, const QString& name, const QString& id, const QString& time, const QString& specialty);
    void loadPatientInfo(const PatientDetailDTO& patient, int appointmentId, const QString& time = "", const QString& specialty = "");
    void clearExamForm();
    int currentAppointmentId() const { return m_currentAppointmentId; }

signals:
    // Tín hiệu yêu cầu quay lại màn hình Dashboard chính
    void backToDashboardRequested();
    void finishExamRequested();
    void viewAppointmentsListRequested();
    void callPatientRequested(int appointmentId = 0);

private slots:
    void onSaveClicked();
    void onFinishExamClicked();
    void validateTemperatureInput();
    void validateHeartRateInput();
    void validateWeightInput();
    void validateHeightInput();
    void validateDiagnosisInput();
    void validateChiefComplaintInput();

private:
    std::shared_ptr<PharmacyService> m_pharmacyService;
    std::shared_ptr<PatientService> m_patientService;
    std::shared_ptr<AppointmentService> m_appointmentService;
    std::shared_ptr<MedicalRecordService> m_medicalRecordService;
    int m_currentPatientId = 0;
    int m_currentAppointmentId = 0;
    int m_currentMedicalRecordId = 0;

    QList<Diagnosis> getDiagnosesFromUi() const;

    // --- TOP TABS BAR ---
    QPushButton* m_tabAppointmentsList;
    QPushButton* m_tabClinicalExam;
    QPushButton* m_tabRegistration;
    QPushButton* m_tabBilling;

    // --- PATIENT INFO AREA ---
    QLabel* m_lblPatientCodeVal;
    QLabel* m_lblPatientNameVal;
    QLabel* m_lblPatientDobVal;
    QLabel* m_lblPatientGenderVal;
    QLabel* m_lblPatientAgeVal;

    // --- QUICK ACTION BUTTONS ---
    QPushButton* m_btnSave;            // Viết Hồ Sơ Bệnh Án
    QPushButton* m_btnPrescription;    // Kê Đơn Thuốc
    QPushButton* m_btnServiceOrder;    // Yêu Cầu Xét Nghiệm
    QPushButton* m_btnFinish;          // Hoàn Tất Khám
    QPushButton* m_btnCancel;          // Hủy Khám
    QPushButton* m_btnHistory;
    QPushButton* m_btnCallPatient = nullptr;
    QPushButton* m_btnPrint;

    // --- SUB-SIDEBAR (KHÁM BỆNH MENU) ---
    QPushButton* m_subClinicalExam;
    QPushButton* m_subServiceOrder;
    QPushButton* m_subPrescription;
    QPushButton* m_subSummaryResults;
    QPushButton* m_subSocialInsurance;
    QPushButton* m_subHospitalTransfer;

    // --- MAIN FORM INPUTS ---
    QComboBox* m_cbTemplate;
    QLineEdit* m_txtWeight;
    QLineEdit* m_txtHeight;
    QLineEdit* m_txtTemp;
    QLineEdit* m_txtBreath;
    QLineEdit* m_txtPulse;
    QLineEdit* m_txtBp;
    QLineEdit* m_txtSpo2;
    QLabel*    m_lblBmiVal;

    QTextEdit* m_txtReason;
    QComboBox* m_cbDiagnosis;
    QComboBox* m_cbSeverity;
    QLineEdit* m_txtMainDisease;
    QLineEdit* m_txtSubDisease = nullptr;
    QComboBox* m_cbDirection;
    QComboBox* m_cbAction;
    QComboBox* m_cbService;
    QComboBox* m_cbRoom;
    QLineEdit* m_txtAdvice;

    // --- RIGHT PANEL TEXT EDITS ---
    QTextEdit* m_txtHistoryIllness;
    QTextEdit* m_txtHistoryPersonal;
    QTextEdit* m_txtExamGeneral;
    QTextEdit* m_txtClsSummary;

    // --- UI Setup & Helpers ---
    void setupUi();
    QHBoxLayout* setupTopTabBar();
    QFrame* setupPatientInfoCard();
    QFrame* setupSubSidebar();
    QWidget* setupMainExamForm();
    QFrame* setupMedicalHistoryPanel();
    QFrame* createSeparator();
    void updateBmi();
};