#include "ManagePatientsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include "AdminPatientRegistrationDialog.h"

ManagePatientsWidget::ManagePatientsWidget(std::shared_ptr<PatientService> patientService, QWidget* parent)
    : QWidget(parent), m_patientService(patientService), m_tblPatients(nullptr) {
    buildUI();
}

QFrame* ManagePatientsWidget::makeCard(QWidget* parent) {
    QFrame* card = new QFrame(parent);
    card->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: 12px; border: 1px solid #E5E7EB; }");
    return card;
}

void ManagePatientsWidget::buildUI() {
    QVBoxLayout* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(30, 30, 30, 30);
    pageLayout->setSpacing(20);

    // Header
    QHBoxLayout* headerLayout = new QHBoxLayout();
    QLabel* lblTitle = new QLabel("Quản lý Bệnh nhân");
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #111827;");
    headerLayout->addWidget(lblTitle);
    
    headerLayout->addStretch();
    
    QPushButton* btnAddNew = new QPushButton("+ Thêm Bệnh nhân");
    btnAddNew->setCursor(Qt::PointingHandCursor);
    btnAddNew->setStyleSheet("QPushButton { background-color: #2563EB; color: white; border-radius: 6px; padding: 8px 16px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: #1D4ED8; }");
    headerLayout->addWidget(btnAddNew);
    
    pageLayout->addLayout(headerLayout);

    // Table Card
    QFrame* tableCard = makeCard(this);
    QVBoxLayout* cardLayout = new QVBoxLayout(tableCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    m_tblPatients = new QTableWidget(tableCard);
    m_tblPatients->setColumnCount(6);
    m_tblPatients->setHorizontalHeaderLabels({"Mã BN", "Họ Tên", "Tuổi", "SĐT", "Loại", "Thao tác"});
    m_tblPatients->horizontalHeader()->setStretchLastSection(true);
    m_tblPatients->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tblPatients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tblPatients->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblPatients->setAlternatingRowColors(true);
    m_tblPatients->setStyleSheet("QTableWidget { border: none; background-color: white; alternate-background-color: #F9FAFB; color: #111827; } QHeaderView::section { background-color: #F3F4F6; padding: 12px; font-weight: bold; border: none; border-bottom: 1px solid #E5E7EB; color: #111827; } QTableWidget::item { padding: 12px; border-bottom: 1px solid #E5E7EB; color: #111827; }");

    cardLayout->addWidget(m_tblPatients);
    pageLayout->addWidget(tableCard);

    connect(btnAddNew, &QPushButton::clicked, this, &ManagePatientsWidget::showAddPatientDialog);

    loadPatientsList();
}

void ManagePatientsWidget::showAddPatientDialog() {
    AdminPatientRegistrationDialog dialog(m_patientService, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadPatientsList();
    }
}

void ManagePatientsWidget::showEditPatientDialog() {
    QMessageBox::information(this, "Sửa Bệnh nhân", "Tính năng sửa bệnh nhân đang được phát triển.");
}

void ManagePatientsWidget::loadPatientsList() {
    if (!m_tblPatients || !m_patientService) return;
    m_tblPatients->setRowCount(0);

    PatientSearchCriteria criteria;
    auto patients = m_patientService->searchPatients(criteria);
    
    for (int i = 0; i < patients.size(); ++i) {
        auto patient = patients[i];
        m_tblPatients->insertRow(i);

        QTableWidgetItem* itemCode = new QTableWidgetItem(patient.patientCode);
        itemCode->setData(Qt::UserRole, patient.patientId);
        itemCode->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemName = new QTableWidgetItem(patient.fullName);
        itemName->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemGender = new QTableWidgetItem(genderToString(patient.gender));
        itemGender->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemDob = new QTableWidgetItem(patient.dateOfBirth.toString("yyyy-MM-dd"));
        itemDob->setForeground(QBrush(QColor("#111827")));
        
        QTableWidgetItem* itemPhone = new QTableWidgetItem(patient.phone);
        itemPhone->setForeground(QBrush(QColor("#111827")));

        m_tblPatients->setItem(i, 0, itemCode);
        m_tblPatients->setItem(i, 1, itemName);
        m_tblPatients->setItem(i, 2, itemGender);
        m_tblPatients->setItem(i, 3, itemDob);
        m_tblPatients->setItem(i, 4, itemPhone);

        QWidget *actionWidget = new QWidget();
        QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 4, 4, 4);
        actionLayout->setSpacing(8);
        QPushButton *btnEdit = new QPushButton("Sửa");
        btnEdit->setCursor(Qt::PointingHandCursor);
        btnEdit->setStyleSheet(
            "QPushButton { color: #4B94F2; border: 1px solid #4B94F2; padding: 4px "
            "8px; border-radius: 4px; background-color: white; } QPushButton:hover "
            "{ background-color: #EBF5FF; }");
        QPushButton *btnDelete = new QPushButton("Xóa");
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setStyleSheet(
            "QPushButton { color: #F43F5E; border: 1px solid #F43F5E; padding: 4px "
            "8px; border-radius: 4px; background-color: white; } QPushButton:hover "
            "{ background-color: #FFE4E6; }");
        actionLayout->addWidget(btnEdit);
        actionLayout->addWidget(btnDelete);
        actionLayout->setAlignment(Qt::AlignCenter);
        m_tblPatients->setCellWidget(i, 5, actionWidget);
    }
}
