#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QTabBar>
#include <QHeaderView>
#include <QStackedWidget>
#include <QLabel>
#include <QComboBox>
#include<QLineEdit>
#include<QTextEdit>
class Patient : public QWidget {
    Q_OBJECT

public:
    explicit Patient(QWidget *parent = nullptr);
    ~Patient() override;

private slots:
    void handlePatientSelected(int row, int column);
    void showPatientList();

private:
    void setupUi();
    void createListView();
    void createDetailView();

    QVBoxLayout* m_mainLayout;
    QStackedWidget* m_mainStackedWidget;

    // --- Thành phần của Màn hình 1: Danh sách bệnh nhân ---
    QWidget* m_listViewWidget;
    QTabBar* m_patientTabBar;
    QTableWidget* m_patientTable;
    QPushButton* m_addPatientButton;

    // --- Thành phần của Màn hình 2: Chi tiết bệnh nhân ---
    QWidget* m_detailViewWidget;
    QStackedWidget* m_detailStackedWidget;
    QWidget* m_infoSubPage;
    QWidget* m_recordSubPage;
    
    QPushButton* m_btnInfo;
    QPushButton* m_btnMedicalRecord;
    QPushButton* m_btnBack;
    // Giao diện thông tin bệnh nhân
    QLineEdit* txtPatientID;
    QLineEdit* txtFullName;
    QLineEdit*  txtDob;
    QLineEdit* txtPhone;
    QLineEdit* txtAddress;
    QComboBox *cmbGender;
    QPushButton* m_btnEditInfo;
    QPushButton* m_btnDeleteInfo;



};