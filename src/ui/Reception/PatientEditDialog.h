#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <memory>
#include "service/PatientService.h"
#include "dto/PatientDTOs.h"

class PatientEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit PatientEditDialog(int patientId, std::shared_ptr<PatientService> patientService, QWidget *parent = nullptr);
    ~PatientEditDialog() override = default;

signals:
    void patientUpdated();

private slots:
    void handleSave();

private:
    void setupUi();
    void loadPatientData();

    int m_patientId;
    std::shared_ptr<PatientService> m_patientService;
    std::optional<PatientDetailDTO> m_patientDetail;

    QLineEdit *m_txtFullName = nullptr;
    QLineEdit *m_txtPhone = nullptr;
    QLineEdit *m_txtEmail = nullptr;
    QLineEdit *m_txtAddress = nullptr;
    QLineEdit *m_txtEmergencyName = nullptr;
    QLineEdit *m_txtEmergencyPhone = nullptr;

    QPushButton *m_btnSave = nullptr;
    QPushButton *m_btnCancel = nullptr;
};
