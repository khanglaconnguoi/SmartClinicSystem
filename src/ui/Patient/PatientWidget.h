#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QTabBar>
#include <QHeaderView>
#include <QStackedWidget>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QButtonGroup>
#include <QScrollArea>
#include <QGraphicsDropShadowEffect>
#include <QDateEdit>

class PatientWidget : public QWidget {
    Q_OBJECT

public:
    explicit PatientWidget(QWidget *parent = nullptr);
    ~PatientWidget() override;

private slots:
    void handlePatientSelected(int row, int column);
    void showPatientList();
    //void onAddMedicationClicked();

private:
    void setupUi();
    void createListView();
    void createDetailView();

    QVBoxLayout* m_mainLayout;
    QStackedWidget* m_mainStackedWidget;

    QWidget* m_listViewWidget;
    QTabBar* m_patientTabBar;
    QTableWidget* m_patientTable;

    QWidget* m_detailViewWidget;
    QStackedWidget* m_detailStackedWidget;
    QWidget* m_infoSubPage;
    
    QPushButton* m_btnInfo;
    QPushButton* m_btnBack;
    
    QLineEdit* txtPatientID;
    QLineEdit* txtFullName;
    QLineEdit* txtDob;
    QLineEdit* txtPhone;
    QLineEdit* txtAddress;
    QTextEdit* txtNotes;
    QPushButton* m_btnEditInfo;
    QPushButton* m_btnDeleteInfo;
    QLineEdit* txtCitizenID;
    QLineEdit* txtEmail;
    QLineEdit* txtBloodType;
    QLineEdit* txtAllergies;
    QLineEdit* txtInsurance;
    QLineEdit* txtEmergencyName;
    QLineEdit* txtEmergencyPhone;
    QCheckBox* radMale;
    QCheckBox* radFemale;
    QButtonGroup* m_genderGroup;
};