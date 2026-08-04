#pragma once
#include <QWidget>
#include <QFrame>
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
#include "service/PatientService.h"
#include "service/PharmacyService.h"
#include "service/MedicalRecordService.h"
#include "service/AppointmentService.h"
#include <memory>

class PatientWidget : public QWidget {
    Q_OBJECT

public:
    explicit PatientWidget(QWidget *parent = nullptr);
    ~PatientWidget() override;

    void setPatientService(std::shared_ptr<PatientService> patientService);
    void setServices(std::shared_ptr<PharmacyService> pharmacyService, 
                     std::shared_ptr<MedicalRecordService> medicalRecordService,
                     std::shared_ptr<AppointmentService> appointmentService = nullptr,
                     int doctorId = -1) {
        m_pharmacyService = pharmacyService;
        m_medicalRecordService = medicalRecordService;
        m_appointmentService = appointmentService;
        m_doctorId = doctorId;
    }
    void loadPatientsData();

private slots:
    void handlePatientSelected(int row, int column);
    void showPatientList();

private:
    void setupUi();
    void createListView();
    void createDetailView();

    std::shared_ptr<PatientService> m_patientService;
    std::shared_ptr<PharmacyService> m_pharmacyService;
    std::shared_ptr<MedicalRecordService> m_medicalRecordService;
    std::shared_ptr<AppointmentService> m_appointmentService;
    int m_doctorId = -1;
    QList<PatientSearchResultDTO> m_patientList;

    QVBoxLayout* m_mainLayout;
    QStackedWidget* m_mainStackedWidget;

    QWidget* m_listViewWidget;
    QTabBar* m_patientTabBar;
    QTableWidget* m_patientTable;

    QWidget* m_detailViewWidget;
    QStackedWidget* m_detailStackedWidget;
    QWidget* m_infoSubPage;
    
    QPushButton* m_btnInfo;
    QPushButton* m_btnHistoryInfo;
    QPushButton* m_btnBack;
    
    QLineEdit* txtPatientID;
    QLineEdit* txtFullName;
    QLineEdit* txtDob;
    QLineEdit* txtPhone;
    QLineEdit* txtAddress;
    QTextEdit* txtNotes;
    QLineEdit* txtCitizenID;
    QLineEdit* txtEmail;
    QComboBox* cbBloodType;
    QLineEdit* txtAllergies;
    QLineEdit* txtInsurance;
    QLineEdit* txtEmergencyName;
    QLineEdit* txtEmergencyPhone;
    QComboBox* cbGender;

    QLineEdit* m_searchBar;
};
