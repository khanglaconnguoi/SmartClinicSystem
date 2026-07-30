#include "PatientRecordHistoryDialog.h"
#include "../../service/PharmacyService.h"
#include "model/CommonEnums.h"
#include <QDebug>

PatientRecordHistoryDialog::PatientRecordHistoryDialog(
    std::shared_ptr<PharmacyService> pharmacyService,
    QWidget *parent)
    : QDialog(parent), m_pharmacyService(pharmacyService) {
    setWindowTitle("Lịch Sử Bệnh Án & Đơn Thuốc Bệnh Nhân");
    setMinimumSize(950, 650);
    setStyleSheet("QDialog { background-color: #F8FAFC; font-family: 'Segoe UI'; }");

    setupUI();
}

void PatientRecordHistoryDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    // 1. Header Bệnh nhân
    lblPatientHeader = new QLabel("LỊCH SỬ KHÁM BỆNH", this);
    lblPatientHeader->setStyleSheet("font-size: 18px; font-weight: bold; color: #0F172A;");
    mainLayout->addWidget(lblPatientHeader);

    // 2. Workspace Cột Trái (Danh sách đơn/ca khám) + Cột Phải (Chi tiết)
    QHBoxLayout *workspaceLayout = new QHBoxLayout();
    workspaceLayout->setSpacing(14);

    // Cột Trái: Bảng đơn thuốc / ca khám
    QFrame *leftFrame = new QFrame(this);
    leftFrame->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; }");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    leftLayout->setContentsMargins(12, 12, 12, 12);

    QLabel *lblListTitle = new QLabel("Danh sách ca khám / Đơn thuốc:", leftFrame);
    lblListTitle->setStyleSheet("font-weight: bold; color: #334155; border: none;");
    leftLayout->addWidget(lblListTitle);

    tblRecordList = new QTableWidget(leftFrame);
    tblRecordList->setColumnCount(4);
    tblRecordList->setHorizontalHeaderLabels({"Mã Đơn", "Ngày Kê", "Bác Sĩ", "Trạng Thái"});
    tblRecordList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblRecordList->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblRecordList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblRecordList->verticalHeader()->setVisible(false);
    tblRecordList->setStyleSheet("QTableWidget { border: 1px solid #CBD5E1; border-radius: 6px; } QHeaderView::section { background-color: #F1F5F9; font-weight: bold; border: none; padding: 6px; }");
    leftLayout->addWidget(tblRecordList);
    workspaceLayout->addWidget(leftFrame, 4);

    connect(tblRecordList, &QTableWidget::cellClicked, this, &PatientRecordHistoryDialog::onRecordSelected);

    // Cột Phải: Details (Sinh hiệu, chẩn đoán, chi tiết đơn)
    QFrame *rightFrame = new QFrame(this);
    rightFrame->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; }"
                              "QLabel { font-size: 12px; font-weight: bold; color: #334155; border: none; }"
                              "QLineEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 4px 8px; font-size: 12px; background-color: #F8FAFC; }"
                              "QTextEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 6px; font-size: 12px; background-color: #F8FAFC; }");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    rightLayout->setContentsMargins(14, 14, 14, 14);
    rightLayout->setSpacing(8);

    QHBoxLayout *r1 = new QHBoxLayout();
    r1->addWidget(new QLabel("Ngày kê:", rightFrame));
    txtRecordDate = new QLineEdit(rightFrame); txtRecordDate->setReadOnly(true);
    r1->addWidget(txtRecordDate);
    r1->addWidget(new QLabel("Bác sĩ:", rightFrame));
    txtDoctorName = new QLineEdit(rightFrame); txtDoctorName->setReadOnly(true);
    r1->addWidget(txtDoctorName);
    rightLayout->addLayout(r1);

    QHBoxLayout *r2 = new QHBoxLayout();
    r2->addWidget(new QLabel("Chẩn đoán:", rightFrame));
    txtDiagnosis = new QLineEdit(rightFrame); txtDiagnosis->setReadOnly(true);
    r2->addWidget(txtDiagnosis);
    rightLayout->addLayout(r2);

    rightLayout->addWidget(new QLabel("Ghi chú / Lời dặn:", rightFrame));
    txtTreatmentAdvice = new QTextEdit(rightFrame); txtTreatmentAdvice->setReadOnly(true);
    txtTreatmentAdvice->setFixedHeight(50);
    rightLayout->addWidget(txtTreatmentAdvice);

    rightLayout->addWidget(new QLabel("Chi tiết đơn thuốc cũ:", rightFrame));
    tblOldPrescription = new QTableWidget(rightFrame);
    tblOldPrescription->setColumnCount(6);
    tblOldPrescription->setHorizontalHeaderLabels({"Tên Thuốc", "Đơn Giá", "Số Lượng", "Liều Lượng", "Tần Suất", "Số Ngày"});
    tblOldPrescription->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblOldPrescription->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblOldPrescription->verticalHeader()->setVisible(false);
    tblOldPrescription->setStyleSheet("QTableWidget { border: 1px solid #CBD5E1; border-radius: 6px; } QHeaderView::section { background-color: #F1F5F9; font-weight: bold; border: none; padding: 6px; }");
    rightLayout->addWidget(tblOldPrescription);

    workspaceLayout->addWidget(rightFrame, 6);
    mainLayout->addLayout(workspaceLayout, 1);

    // Nút Đóng
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    btnClose = new QPushButton("Đóng", this);
    btnClose->setFixedWidth(100);
    btnClose->setCursor(Qt::PointingHandCursor);
    btnClose->setStyleSheet("QPushButton { background-color: #64748B; color: white; border: none; font-weight: bold; border-radius: 6px; padding: 8px 16px; }"
                            "QPushButton:hover { background-color: #475569; }");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(btnClose);
    mainLayout->addLayout(bottomLayout);
}

void PatientRecordHistoryDialog::loadPatientHistory(int patientId, const QString &patientName, const QString &patientCode) {
    m_currentPatientId = patientId;
    lblPatientHeader->setText(QString("LỊCH SỬ KHÁM BỆNH - %1 (%2)").arg(patientName, patientCode));

    if (!m_pharmacyService) return;

    auto prescriptions = m_pharmacyService->getPrescriptionsByPatient(patientId);
    tblRecordList->setRowCount(0);

    for (int i = 0; i < prescriptions.size(); ++i) {
        const auto &p = prescriptions[i];
        tblRecordList->insertRow(i);

        QTableWidgetItem *itemCode = new QTableWidgetItem(QString("DT%1").arg(p.prescriptionId, 6, 10, QChar('0')));
        itemCode->setData(Qt::UserRole, p.prescriptionId);
        itemCode->setData(Qt::UserRole + 1, p.recordId);
        
        QTableWidgetItem *itemDate = new QTableWidgetItem(p.prescribedAt.toString("yyyy-MM-dd HH:mm"));
        QTableWidgetItem *itemDoc = new QTableWidgetItem(p.doctorName);
        QTableWidgetItem *itemStatus = new QTableWidgetItem(prescriptionStatusToVi(p.status));

        tblRecordList->setItem(i, 0, itemCode);
        tblRecordList->setItem(i, 1, itemDate);
        tblRecordList->setItem(i, 2, itemDoc);
        tblRecordList->setItem(i, 3, itemStatus);
    }

    if (!prescriptions.isEmpty()) {
        tblRecordList->selectRow(0);
        onRecordSelected(0, 0);
    }
}

void PatientRecordHistoryDialog::onRecordSelected(int row, int /*column*/) {
    if (row < 0 || !tblRecordList->item(row, 0)) return;

    int recordId = tblRecordList->item(row, 0)->data(Qt::UserRole + 1).toInt();
    if (!m_pharmacyService) return;

    PrescriptionSearchCriteria criteria;
    criteria.recordId = recordId;
    auto list = m_pharmacyService->searchPrescriptionsPaged(criteria).items;

    if (list.isEmpty()) return;

    const auto &p = list.first();
    txtRecordDate->setText(p.prescribedAt.toString("dd/MM/yyyy HH:mm"));
    txtDoctorName->setText(p.doctorName);
    txtDiagnosis->setText(p.diagnosis.isEmpty() ? "---" : p.diagnosis);
    txtTreatmentAdvice->setText(p.notes.isEmpty() ? "Không có ghi chú" : p.notes);

    tblOldPrescription->setRowCount(0);
    for (int i = 0; i < p.items.size(); ++i) {
        const auto &item = p.items[i];
        tblOldPrescription->insertRow(i);
        tblOldPrescription->setItem(i, 0, new QTableWidgetItem(item.brandName));
        tblOldPrescription->setItem(i, 1, new QTableWidgetItem(QString("%1 đ").arg(item.unitPrice, 0, 'f', 0)));
        tblOldPrescription->setItem(i, 2, new QTableWidgetItem(QString::number(item.quantity)));
        tblOldPrescription->setItem(i, 3, new QTableWidgetItem(item.dosage));
        tblOldPrescription->setItem(i, 4, new QTableWidgetItem(item.frequency));
        tblOldPrescription->setItem(i, 5, new QTableWidgetItem(QString("%1 ngày").arg(item.durationDays)));
    }
}
