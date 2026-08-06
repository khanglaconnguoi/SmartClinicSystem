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
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
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
    void handlePrevPage();
    void handleNextPage();

private:
    void setupUi();
    void createListView();
    void updatePaginationInfo();

    std::shared_ptr<PatientService> m_patientService;
    std::shared_ptr<PharmacyService> m_pharmacyService;
    std::shared_ptr<MedicalRecordService> m_medicalRecordService;
    std::shared_ptr<AppointmentService> m_appointmentService;
    int m_doctorId = -1;
    QList<PatientSearchResultDTO> m_patientList;

    QVBoxLayout* m_mainLayout;
    QWidget* m_listViewWidget;
    QTableWidget* m_patientTable;
    QLineEdit* m_searchBar;

    // Pagination
    int m_currentPage = 1;
    int m_totalPages = 1;
    QPushButton* m_btnPrevPage;
    QPushButton* m_btnNextPage;
    QLabel* m_lblPageInfo;
};
