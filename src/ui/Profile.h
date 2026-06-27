#ifndef PROFILE_H
#define PROFILE_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

class ProfileWidget : public QDialog {
    Q_OBJECT

public:
    explicit ProfileWidget(QWidget *parent = nullptr);
    void loadDoctorProfile(const QString &staffCode);

private slots:
    void onEditClicked();

private:
    QWidget* createTopBar();
    QWidget* createLeftPanel();
    QWidget* createRightPanel();

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

    int currentStaffId;
    QString currentStaffCode;
};

#endif