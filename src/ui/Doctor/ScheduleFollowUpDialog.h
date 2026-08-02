#pragma once

#include <QDialog>
#include <QDateEdit>
#include <QTimeEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <memory>

#include "service/AppointmentService.h"
#include "dto/AppointmentDTOs.h"

class ScheduleFollowUpDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScheduleFollowUpDialog(
        int patientId,
        const QString &patientName,
        int doctorId,
        std::shared_ptr<AppointmentService> appointmentService,
        QWidget *parent = nullptr);
    ~ScheduleFollowUpDialog() override = default;

signals:
    void appointmentScheduled();

private slots:
    void handleSave();

private:
    void setupUi();
    void loadRooms();

    int m_patientId;
    QString m_patientName;
    int m_doctorId;
    std::shared_ptr<AppointmentService> m_appointmentService;

    QDateEdit *m_dateEdit = nullptr;
    QTimeEdit *m_timeEdit = nullptr;
    QComboBox *m_cbRooms = nullptr;
    QLineEdit *m_txtReason = nullptr;

    QPushButton *m_btnSave = nullptr;
    QPushButton *m_btnCancel = nullptr;
};
