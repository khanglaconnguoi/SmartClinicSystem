#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QFrame>
#include <memory>

#include "service/PatientService.h"

class ManagePatientsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManagePatientsWidget(std::shared_ptr<PatientService> patientService, QWidget* parent = nullptr);
    ~ManagePatientsWidget() override = default;

    void loadPatientsList();

private slots:
    void showAddPatientDialog();
    void showEditPatientDialog();

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<PatientService> m_patientService;
    QTableWidget* m_tblPatients;
};
