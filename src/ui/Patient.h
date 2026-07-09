// Patient.h
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QTabBar>
#include <QHeaderView>
#include <QStackedWidget>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QButtonGroup>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QDateEdit>

class Patient : public QWidget {
    Q_OBJECT

public:
    explicit Patient(QWidget *parent = nullptr);
    ~Patient() override;

private slots:
    void handlePatientSelected(int row, int column);
    void showPatientList();

private:
    void setupUi();
    void createListView();
    void createDetailView();
    void createMedicalRecordPage();

    QVBoxLayout* m_mainLayout;
    QStackedWidget* m_mainStackedWidget;

    QWidget* m_listViewWidget;
    QTabBar* m_patientTabBar;
    QTableWidget* m_patientTable;

    QWidget* m_detailViewWidget;
    QStackedWidget* m_detailStackedWidget;
    QWidget* m_infoSubPage;
    QWidget* m_recordSubPage;
    
    QPushButton* m_btnInfo;
    QPushButton* m_btnMedicalRecord;
    QPushButton* m_btnBack;
    
    QLineEdit* txtPatientID;
    QLineEdit* txtFullName;
    QLineEdit* txtDob;
    QLineEdit* txtPhone;
    QLineEdit* txtAddress;
    QTextEdit* txtNotes;
    QPushButton* m_btnEditInfo;
    QPushButton* m_btnDeleteInfo;
    QLineEdit* txtCitizenID;
    QLineEdit* txtEmail;
    QLineEdit* txtBloodType;
    QLineEdit* txtAllergies;
    QLineEdit* txtInsurance;
    QLineEdit* txtEmergencyName;
    QLineEdit* txtEmergencyPhone;
    QCheckBox* radMale;
    QCheckBox* radFemale;
    QButtonGroup* m_genderGroup;

    QScrollArea* m_recordScrollArea;
    QLineEdit* txtRecordID;
    QLineEdit* txtRecordDate;
    QLineEdit* txtRecordDoctor;
    QLineEdit* txtRecordDept;
    QLineEdit* txtRecordType;
    QLineEdit* txtRecordStatus;
    QLineEdit* txtRecordCount;

    QTextEdit* txtReason;
    QTextEdit* txtHistory;
    QTextEdit* txtPastHistory;
    QTextEdit* txtAllergyRecord;
    QTextEdit* txtClinicalExam;
    
    QTableWidget* m_diagnosticTable;
    QPushButton* m_btnAddDiagnostic;
    
    QTextEdit* txtTreatment;
    
    QTableWidget* m_prescriptionTable;
    QPushButton* m_btnAddPrescription;
    
    QCheckBox* chkBloodCount;
    QCheckBox* chkGlucose;
    QCheckBox* chkLiver;
    QCheckBox* chkEcg;
    QTextEdit* txtSubClinicalNotes;

    QLineEdit* txtPulse;
    QLineEdit* txtBloodPressure;
    QLineEdit* txtRespiratoryRate;
    QLineEdit* txtSpO2;
    QLineEdit* txtWeight;
    QLineEdit* txtHeight;
    QLineEdit* txtBMI;
    QLineEdit* txtTemperature;

    QTextEdit* txtSubClinicalResult;
    QTextEdit* txtAdvice;
    QDateEdit* dateRecall;
};