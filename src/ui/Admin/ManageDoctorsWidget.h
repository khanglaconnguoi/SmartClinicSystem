#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QFrame>
#include <memory>
#include "../../service/StaffService.h"
#include "../../service/AppointmentService.h"
#include "../../model/Doctor.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class ManageDoctorsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManageDoctorsWidget(std::shared_ptr<StaffService> staffService,
                                std::shared_ptr<AppointmentService> appointmentService = nullptr,
                                QWidget* parent = nullptr);
    ~ManageDoctorsWidget() override = default;

    void loadDoctorsList();

private slots:
    void showAddDoctorDialog();
    void showEditDoctorDialog(std::shared_ptr<Doctor> doc);
    void handleFilterChanged();
    void handleResetFilters();
    void handlePrevPage();
    void handleNextPage();

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    std::shared_ptr<AppointmentService> m_appointmentService;
    QTableWidget* m_tblDoctors;

    // Filter controls
    QLineEdit* m_txtSearchKey;
    QComboBox* m_cbSpecialtyFilter;
    QComboBox* m_cbDepartmentFilter;
    QComboBox* m_cbShiftFilter;
    QComboBox* m_cbStatusFilter;
    QPushButton* m_btnResetFilters;

    // Pagination controls
    int m_currentPage = 1;
    int m_totalPages = 1;
    QPushButton* m_btnPrevPage;
    QPushButton* m_btnNextPage;
    QLabel* m_lblPageInfo;
};

