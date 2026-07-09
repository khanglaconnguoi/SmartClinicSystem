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

class ClinicalExamWidget : public QWidget {
    Q_OBJECT

public:
    explicit ClinicalExamWidget(QWidget* parent = nullptr);
    virtual ~ClinicalExamWidget() override = default;

    // Hàm load thông tin bệnh nhân khi bác sĩ chọn khám
    void loadPatientInfo(const QString& name, const QString& id, const QString& time, const QString& specialty);

signals:
    // Tín hiệu yêu cầu quay lại màn hình Dashboard chính
    void backToDashboardRequested();
    void finishExamRequested();
    void viewAppointmentsListRequested();

private:
    // --- TOP TABS BAR ---
    QPushButton* m_tabDanhSach;
    QPushButton* m_tabKhamLamSang;
    QPushButton* m_tabDangKyKham;
    QPushButton* m_tabThuTien;

    // --- PATIENT INFO AREA ---
    QLabel* m_lblPatientCodeVal;
    QLabel* m_lblPatientNameVal;
    QLabel* m_lblPatientDobVal;
    QLabel* m_lblPatientGenderVal;
    QLabel* m_lblPatientAgeVal;

    // --- QUICK ACTION BUTTONS ---
    QPushButton* m_btnNew;
    QPushButton* m_btnSave;
    QPushButton* m_btnFinish;
    QPushButton* m_btnCancel;
    QPushButton* m_btnHistory;
    QPushButton* m_btnCallPatient;
    QPushButton* m_btnPrint;

    // --- SUB-SIDEBAR (KHÁM BỆNH MENU) ---
    QPushButton* m_subKhamLamSang;
    QPushButton* m_subChiDinhDichVu;
    QPushButton* m_subDonThuoc;
    QPushButton* m_subKetQuaTongHop;
    QPushButton* m_subBhxh;
    QPushButton* m_subChuyenVien;

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
    QLineEdit* m_txtMainDisease;
    QLineEdit* m_txtSubDisease;
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

    // --- UI Setup ---
    void setupUi();
    QFrame* createSeparator();
    void updateBmi();
};