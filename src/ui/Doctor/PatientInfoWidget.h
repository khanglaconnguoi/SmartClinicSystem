#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QLineEdit>
#include <QComboBox>

#include "dto/PatientDTOs.h"
#include "model/CommonEnums.h"

class PatientInfoWidget : public QWidget {
    Q_OBJECT

public:
    explicit PatientInfoWidget(QWidget *parent = nullptr);
    ~PatientInfoWidget() override = default;

    void loadPatientData(const PatientDetailDTO& detail);

private:
    void setupUi();

    QLineEdit* txtPatientID;
    QLineEdit* txtFullName;
    QLineEdit* txtAge;
    QComboBox* cbGender;
    QComboBox* cbBloodType;
    QLineEdit* txtAllergies;
};
