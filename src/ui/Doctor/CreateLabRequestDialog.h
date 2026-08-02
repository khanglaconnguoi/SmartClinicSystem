#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QMessageBox>
#include <memory>

#include "service/ServiceRequestService.h"
#include "service/AppointmentService.h"
#include "dto/ServiceRequestDTOs.h"

class CreateLabRequestDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateLabRequestDialog(
        std::shared_ptr<ServiceRequestService> serviceRequestService,
        std::shared_ptr<AppointmentService> appointmentService,
        int recordId,
        int doctorId,
        const QString& doctorName,
        const QString& patientName,
        const QString& patientCode,
        QWidget* parent = nullptr
    );

    ~CreateLabRequestDialog() override = default;

private slots:
    void handleSubmit();

private:
    void setupUI();
    void loadLabRooms();

    std::shared_ptr<ServiceRequestService> m_serviceRequestService;
    std::shared_ptr<AppointmentService> m_appointmentService;

    int m_recordId;
    int m_doctorId;
    QString m_doctorName;
    QString m_patientName;
    QString m_patientCode;

    QLabel* m_lblPatientInfo;
    QLabel* m_lblDoctorInfo;
    QComboBox* m_cbLabRoom;
    QComboBox* m_cbServiceName;
    QLineEdit* m_txtCustomService;

    QPushButton* m_btnSubmit;
    QPushButton* m_btnCancel;
};
