#ifndef PROFILE_H
#define PROFILE_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <memory>
#include "../service/StaffService.h"
#include "../dto/StaffDTOs.h"
#include "../model/CommonEnums.h"

class ProfileWidget : public QDialog {
    Q_OBJECT

public:
    ProfileWidget(std::shared_ptr<StaffService> staffService, QWidget *parent = nullptr);
    void loadProfile(int staffId);

private slots:
    void onEditClicked();

private:
    QWidget* createLeftPanel();
    QWidget* createRightPanel();

    std::shared_ptr<StaffService> m_staffService;

    QLabel *lblAvatar;
    QLabel *lblStatus;
    QLabel *lblName;
    QLabel *lblRole;
    QLabel *lblStaffCode;
    QComboBox *cmbShift;
    QPushButton *btnEdit;

    QLineEdit *txtFullName;
    QLineEdit *txtGender;
    QLineEdit *txtDob;
    QLineEdit *txtCitizenId;
    QLineEdit *txtPhone;
    QLineEdit *txtEmail;
    QLineEdit *txtAddress;

    QLabel *lblDepartment;
    QLabel *lblHireDate;

    QWidget *cardRoleSpecific;
    QLabel *lblTitleRoleSpecific;
    
    QWidget *widgetDoctorFields;
    QLineEdit *txtSpecialty;
    QLineEdit *txtLicenseNumber;
    QLineEdit *txtExperienceYears;
    QLineEdit *txtConsultationFee;
    QLineEdit *txtBio;

    QWidget *widgetNurseFields;
    QLineEdit *txtNurseLevel;
    QLineEdit *txtCertification;

    int currentStaffId;
    int currentDepartmentId = -1;
    QString currentStaffCode;
    UserRole currentRole;
};

#endif