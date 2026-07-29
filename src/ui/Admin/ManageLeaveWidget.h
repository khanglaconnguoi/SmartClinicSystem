#pragma once

#include <QWidget>
#include <memory>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QLabel>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include "../../service/StaffService.h"
#include "../../service/AppointmentService.h"

class ManageLeaveWidget : public QWidget {
    Q_OBJECT

public:
    explicit ManageLeaveWidget(std::shared_ptr<StaffService> staffService, std::shared_ptr<AppointmentService> appointmentService, QWidget *parent = nullptr);
    ~ManageLeaveWidget() override = default;

private slots:
    void onLeaveStaffSelected(int index);
    void onSubmitLeaveRequest();
    void loadPendingLeaves();

private:
    void setupUi();
    class QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    std::shared_ptr<AppointmentService> m_appointmentService;

    QComboBox*      m_comboLeaveStaff;
    QLabel*         m_lblLeaveBalance;
    QDateEdit*      m_leaveStartDate;
    QDateEdit*      m_leaveEndDate;
    QTextEdit*      m_txtLeaveReason;
    
    QTableWidget*   m_tablePendingLeaves;
};
