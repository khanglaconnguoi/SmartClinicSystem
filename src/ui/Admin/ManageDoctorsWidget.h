#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QFrame>
#include <memory>
#include "../../service/StaffService.h"
#include "../../service/AppointmentService.h"
#include "../../model/Doctor.h"

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

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    std::shared_ptr<AppointmentService> m_appointmentService;
    QTableWidget* m_tblDoctors;
};

