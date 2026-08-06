#include "PatientRecordHistoryWidget.h"
#include "../../service/PharmacyService.h"
#include "model/CommonEnums.h"
#include <QDebug>

PatientRecordHistoryWidget::PatientRecordHistoryWidget(
    std::shared_ptr<PharmacyService> pharmacyService,
    std::shared_ptr<MedicalRecordService> medicalRecordService,
    QWidget *parent)
    : QWidget(parent), m_pharmacyService(pharmacyService), m_medicalRecordService(medicalRecordService) {
    setStyleSheet("QWidget { background-color: #F8FAFC; font-family: 'Segoe UI'; }");
    setupUI();
}

void PatientRecordHistoryWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(14);

    // Workspace Cột Trái (Danh sách ca khám / đơn) + Cột Phải (Chi tiết)
    QHBoxLayout *workspaceLayout = new QHBoxLayout();
    workspaceLayout->setSpacing(14);

    // Cột Trái: Bảng ca khám
    QFrame *leftFrame = new QFrame(this);
    leftFrame->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; }");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    leftLayout->setContentsMargins(12, 12, 12, 12);

    QLabel *lblListTitle = new QLabel("Danh sách ca khám quá khứ:", leftFrame);
    lblListTitle->setStyleSheet("font-weight: bold; color: #334155; border: none;");
    leftLayout->addWidget(lblListTitle);

    tblRecordList = new QTableWidget(leftFrame);
    tblRecordList->setColumnCount(3);
    tblRecordList->setHorizontalHeaderLabels({"Mã Bệnh Án", "Thời Gian Khám", "Lý Do Khám"});
    tblRecordList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblRecordList->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblRecordList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblRecordList->setFocusPolicy(Qt::NoFocus);
    tblRecordList->setStyleSheet("QTableWidget { border: 1px solid #CBD5E1; border-radius: 6px; outline: none; } QTableWidget::item { outline: none; border: none; } QTableWidget::item:focus { outline: none; border: none; } QHeaderView::section { background-color: #F1F5F9; font-weight: bold; border: none; padding: 6px; }");
    leftLayout->addWidget(tblRecordList);
    workspaceLayout->addWidget(leftFrame, 4);

    connect(tblRecordList, &QTableWidget::cellClicked, this, &PatientRecordHistoryWidget::onRecordSelected);

    // Cột Phải: Details (Sinh hiệu, chẩn đoán, chi tiết đơn)
    QFrame *rightFrame = new QFrame(this);
    rightFrame->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 8px; }"
                              "QLabel { font-size: 12px; font-weight: bold; color: #334155; border: none; }"
                              "QLineEdit { border: none; padding: 4px 8px; font-size: 12px; background-color: transparent; }"
                              "QTextEdit { border: none; padding: 6px; font-size: 12px; background-color: transparent; }");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    rightLayout->setContentsMargins(14, 14, 14, 14);
    rightLayout->setSpacing(8);

    QHBoxLayout *r1 = new QHBoxLayout();
    r1->addWidget(new QLabel("Ngày khám:", rightFrame));
    txtRecordDate = new QLineEdit(rightFrame); txtRecordDate->setReadOnly(true);
    r1->addWidget(txtRecordDate);
    r1->addWidget(new QLabel("Bác sĩ phụ trách:", rightFrame));
    txtDoctorName = new QLineEdit(rightFrame); txtDoctorName->setReadOnly(true);
    r1->addWidget(txtDoctorName);
    rightLayout->addLayout(r1);

    QHBoxLayout *r2 = new QHBoxLayout();
    r2->addWidget(new QLabel("Chẩn đoán ICD:", rightFrame));
    txtDiagnosis = new QLineEdit(rightFrame); txtDiagnosis->setReadOnly(true);
    txtDiagnosis->setStyleSheet("QLineEdit { background-color: transparent; border: none; padding: 4px; font-weight: 500; }");
    r2->addWidget(txtDiagnosis);
    rightLayout->addLayout(r2);

    rightLayout->addWidget(new QLabel("Diễn biến & Hướng điều trị:", rightFrame));
    txtTreatmentAdvice = new QTextEdit(rightFrame);
    txtTreatmentAdvice->setReadOnly(true);
    txtTreatmentAdvice->setMinimumHeight(150);
    txtTreatmentAdvice->setStyleSheet("QTextEdit { background-color: transparent; border: none; padding: 4px; font-weight: 500; }");
    rightLayout->addWidget(txtTreatmentAdvice);

    rightLayout->addWidget(new QLabel("Chi tiết đơn thuốc đã kê:", rightFrame));
    tblOldPrescription = new QTableWidget(rightFrame);
    tblOldPrescription->setColumnCount(6);
    tblOldPrescription->setHorizontalHeaderLabels({"Tên Thuốc", "Đơn Giá", "Số Lượng", "Liều Lượng", "Tần Suất", "Số Ngày"});
    tblOldPrescription->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tblOldPrescription->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblOldPrescription->setFocusPolicy(Qt::NoFocus);
    tblOldPrescription->setStyleSheet("QTableWidget { border: 1px solid #CBD5E1; border-radius: 6px; outline: none; } QTableWidget::item { outline: none; border: none; } QTableWidget::item:focus { outline: none; border: none; } QHeaderView::section { background-color: #F1F5F9; font-weight: bold; border: none; padding: 6px; }");
    rightLayout->addWidget(tblOldPrescription);

    workspaceLayout->addWidget(rightFrame, 6);
    mainLayout->addLayout(workspaceLayout, 1);
}

void PatientRecordHistoryWidget::loadPatientHistory(int patientId, const QString &patientName, const QString &patientCode) {
    m_currentPatientId = patientId;

    m_records.clear();
    m_prescriptions.clear();

    if (m_medicalRecordService) {
        m_records = m_medicalRecordService->getMedicalHistory(patientId);
    }
    if (m_pharmacyService) {
        m_prescriptions = m_pharmacyService->getPrescriptionsByPatient(patientId);
    }

    tblRecordList->setRowCount(0);

    if (!m_records.isEmpty()) {
        for (int i = 0; i < m_records.size(); ++i) {
            const auto &rec = m_records[i];
            tblRecordList->insertRow(i);

            QTableWidgetItem *itemCode = new QTableWidgetItem(QString("BA%1").arg(rec.recordId, 6, 10, QChar('0')));
            QTableWidgetItem *itemDate = new QTableWidgetItem(rec.visitDateTime.toString("yyyy-MM-dd HH:mm"));
            QTableWidgetItem *itemReason = new QTableWidgetItem(rec.chiefComplaint);

            tblRecordList->setItem(i, 0, itemCode);
            tblRecordList->setItem(i, 1, itemDate);
            tblRecordList->setItem(i, 2, itemReason);
        }
        tblRecordList->selectRow(0);
        onRecordSelected(0, 0);
    } else if (!m_prescriptions.isEmpty()) {
        for (int i = 0; i < m_prescriptions.size(); ++i) {
            const auto &p = m_prescriptions[i];
            tblRecordList->insertRow(i);

            QTableWidgetItem *itemCode = new QTableWidgetItem(QString("DT%1").arg(p.prescriptionId, 6, 10, QChar('0')));
            QTableWidgetItem *itemDate = new QTableWidgetItem(p.prescribedAt.toString("yyyy-MM-dd HH:mm"));
            QTableWidgetItem *itemReason = new QTableWidgetItem(p.diagnosis);

            tblRecordList->setItem(i, 0, itemCode);
            tblRecordList->setItem(i, 1, itemDate);
            tblRecordList->setItem(i, 2, itemReason);
        }
        tblRecordList->selectRow(0);
        onRecordSelected(0, 0);
    }
}

void PatientRecordHistoryWidget::onRecordSelected(int row, int /*column*/) {
    if (!m_records.isEmpty() && row >= 0 && row < m_records.size()) {
        const auto &rec = m_records[row];
        txtRecordDate->setText(rec.visitDateTime.toString("dd/MM/yyyy HH:mm"));
        txtDoctorName->setText(QString("%1 (%2)").arg(rec.doctorName, rec.doctorCode));

        QStringList diagList;
        for (const auto &d : rec.diagnoses) {
            diagList.append(QString("%1: %2").arg(d.icdCode, d.description));
        }
        txtDiagnosis->setText(diagList.isEmpty() ? "---" : diagList.join("; "));
        txtTreatmentAdvice->setText(QString("Diễn biến: %1 | Hướng điều trị: %2").arg(rec.clinicalNotes, rec.treatment));

        tblOldPrescription->setRowCount(0);
        for (const auto &p : m_prescriptions) {
            if (p.recordId == rec.recordId) {
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
                break;
            }
        }
    } else if (!m_prescriptions.isEmpty() && row >= 0 && row < m_prescriptions.size()) {
        const auto &p = m_prescriptions[row];
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
}
