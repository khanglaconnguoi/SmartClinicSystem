#ifndef PROFILE_H
#define PROFILE_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <memory>
#include "../../service/StaffService.h"
#include "../../dto/StaffDTOs.h"
#include "../../model/CommonEnums.h"
#include "../../service/Validation.h"
#include <QTextEdit>
#include "AvatarPickerWidget.h"

class ProfileWidget : public QDialog {
    Q_OBJECT

public:
    ProfileWidget(std::shared_ptr<StaffService> staffService, QWidget *parent = nullptr);
    void loadProfile(int staffId);
    void setReadOnlyMode();

private slots:
    void onEditClicked();
    void validatePhoneNumber();
    void validateEmail();

private:
    QWidget* createLeftPanel();
    QWidget* createRightPanel();

    std::shared_ptr<StaffService> m_staffService;

    AvatarPickerWidget *lblAvatar;
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
    QWidget *cardBio; 
    QLabel *lblTitleRoleSpecific;
    
    QWidget *widgetDoctorFields;
    QLineEdit *txtSpecialty;
    QLineEdit *txtLicenseNumber;
    QLineEdit *txtConsultationFee;
    QLineEdit *txtDocRoom;
    QTextEdit *txtBio;

    QWidget *widgetNurseFields;
    QLineEdit *txtNurseLevel;
    QLineEdit *txtCertification;
    QLineEdit *txtNurseRoom;


    int currentStaffId;
    int currentDepartmentId = -1;
    int currentExperienceYears = 0;  
    QString currentStaffCode;
    UserRole currentRole;
    QPixmap currentAvatar;
};

#endif