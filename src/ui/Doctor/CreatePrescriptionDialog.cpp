#include "CreatePrescriptionDialog.h"
#include "service/PharmacyService.h"
#include "service/UserSession.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextDocument>
#include <QPdfWriter>
#include <QPainter>

CreatePrescriptionDialog::CreatePrescriptionDialog(std::shared_ptr<PharmacyService> pharmacyService, QWidget *parent)
    : QDialog(parent), m_pharmacyService(pharmacyService) {
    setWindowTitle(QString::fromUtf8("Tạo Đơn Thuốc Mới"));
    resize(900, 600);

    setupUI();
    setupStyleSheets();
}

void CreatePrescriptionDialog::setPharmacyService(std::shared_ptr<PharmacyService> pharmacyService) {
    m_pharmacyService = pharmacyService;
}

void CreatePrescriptionDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *lblTitle = new QLabel(QString::fromUtf8("TẠO ĐƠN THUỐC MỚI"), this);
    lblTitle->setObjectName("dialogTitle");
    lblTitle->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(lblTitle);

    QHBoxLayout *topInfoLayout = new QHBoxLayout();
    topInfoLayout->setSpacing(20);

    txtRecordId = new QLineEdit(this);
    txtRecordId->setReadOnly(true);

    txtDoctorId = new QLineEdit(this);
    txtDoctorId->setReadOnly(true);

    cboDoctorName = new QComboBox(this);

    QLabel *lblRecord = new QLabel(QString::fromUtf8("Mã Bệnh Án:"), this);
    QLabel *lblDocId = new QLabel(QString::fromUtf8("Mã Bác Sĩ:"), this);
    QLabel *lblDocName = new QLabel(QString::fromUtf8("Bác Sĩ Kê Đơn:"), this);

    topInfoLayout->addWidget(lblRecord);
    topInfoLayout->addWidget(txtRecordId, 1);
    topInfoLayout->addWidget(lblDocId);
    topInfoLayout->addWidget(txtDoctorId, 1);
    topInfoLayout->addWidget(lblDocName);
    topInfoLayout->addWidget(cboDoctorName, 2);

    mainLayout->addLayout(topInfoLayout);

    QHBoxLayout *notesLayout = new QHBoxLayout();
    QLabel *lblNotes = new QLabel(QString::fromUtf8("Ghi Chú Chung:"), this);

    txtGeneralNotes = new QTextEdit(this);
    txtGeneralNotes->setFixedHeight(45);
    txtGeneralNotes->setPlaceholderText(QString::fromUtf8("Nhập ghi chú chung cho đơn thuốc (nếu có)..."));

    notesLayout->addWidget(lblNotes);
    notesLayout->addWidget(txtGeneralNotes);
    mainLayout->addLayout(notesLayout);

    QLabel *lblDetail = new QLabel(QString::fromUtf8("Chi Tiết Đơn Thuốc"), this);
    lblDetail->setObjectName("lblDetail");
    mainLayout->addWidget(lblDetail);

    tblPrescription = new QTableWidget(0, 8, this);
    tblPrescription->setHorizontalHeaderLabels(QStringList{
        QString::fromUtf8("Tên Thuốc"),
        QString::fromUtf8("Đơn Giá"),
        QString::fromUtf8("Số Lượng"),
        QString::fromUtf8("Liều Lượng"),
        QString::fromUtf8("Tần Suất"),
        QString::fromUtf8("Số Ngày"),
        QString::fromUtf8("Ghi Chú"),
        ""
    });

    tblPrescription->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tblPrescription->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tblPrescription->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tblPrescription->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tblPrescription->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tblPrescription->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    tblPrescription->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    tblPrescription->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    tblPrescription->setColumnWidth(7, 40);
    tblPrescription->verticalHeader()->setVisible(false);

    mainLayout->addWidget(tblPrescription);

    btnAddMedicine = new QPushButton(QString::fromUtf8("+ Thêm Thuốc"), this);
    btnAddMedicine->setObjectName("btnAddMedicine");
    btnAddMedicine->setFixedWidth(130);

    QHBoxLayout *addBtnLayout = new QHBoxLayout();
    addBtnLayout->addWidget(btnAddMedicine);
    addBtnLayout->addStretch();
    mainLayout->addLayout(addBtnLayout);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    btnSave = new QPushButton(QString::fromUtf8("Lưu Đơn Thuốc"), this);
    btnSave->setObjectName("btnSave");
    btnSave->setFixedHeight(38);

    btnExportPdf = new QPushButton(QString::fromUtf8("📄 Xuất PDF"), this);
    btnExportPdf->setObjectName("btnExportPdf");
    btnExportPdf->setFixedHeight(38);

    btnCancel = new QPushButton(QString::fromUtf8("Hủy Bỏ"), this);
    btnCancel->setObjectName("btnCancel");
    btnCancel->setFixedHeight(38);

    bottomLayout->addWidget(btnSave);
    bottomLayout->addWidget(btnExportPdf);
    bottomLayout->addWidget(btnCancel);

    mainLayout->addLayout(bottomLayout);

    connect(btnAddMedicine, &QPushButton::clicked, this, &CreatePrescriptionDialog::onAddMedicineClicked);
    connect(btnSave, &QPushButton::clicked, this, &CreatePrescriptionDialog::onSaveClicked);
    connect(btnExportPdf, &QPushButton::clicked, this, &CreatePrescriptionDialog::onExportPdfClicked);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void CreatePrescriptionDialog::addMedicineRow(const QString &name, double price, int qty, const QString &dosage, const QString &freq, int days, const QString &note) {
    int row = tblPrescription->rowCount();
    tblPrescription->insertRow(row);

    // 0. Tên thuốc
    QTableWidgetItem *itemDrug = new QTableWidgetItem("🔵 " + name);
    itemDrug->setForeground(QBrush(QColor(0, 0, 0)));
    tblPrescription->setItem(row, 0, itemDrug);

    // 1. Đơn giá
    QTableWidgetItem *itemPrice = new QTableWidgetItem(QString("%1 đ").arg(price, 0, 'f', 0));
    itemPrice->setForeground(QBrush(QColor(0, 0, 0)));
    itemPrice->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    tblPrescription->setItem(row, 1, itemPrice);

    // 2. Số lượng
    QSpinBox *spnQty = new QSpinBox(this);
    spnQty->setRange(1, 1000);
    spnQty->setValue(qty);
    tblPrescription->setCellWidget(row, 2, spnQty);

    // 3. Liều Lượng (QLineEdit - không có đơn vị)
    QLineEdit *txtDosage = new QLineEdit(dosage, this);
    txtDosage->setPlaceholderText(QString::fromUtf8("Ví dụ: 1"));
    tblPrescription->setCellWidget(row, 3, txtDosage);

    // 4. Tần Suất (QLineEdit - không có đơn vị)
    QLineEdit *txtFreq = new QLineEdit(freq, this);
    txtFreq->setPlaceholderText(QString::fromUtf8("Ví dụ: 2"));
    tblPrescription->setCellWidget(row, 4, txtFreq);

    // 5. Số Ngày
    QSpinBox *spnDays = new QSpinBox(this);
    spnDays->setRange(1, 365);
    spnDays->setValue(days);
    tblPrescription->setCellWidget(row, 5, spnDays);

    // 6. Ghi Chú
    QLineEdit *txtNote = new QLineEdit(note, this);
    txtNote->setPlaceholderText(QString::fromUtf8("Ghi chú..."));
    tblPrescription->setCellWidget(row, 6, txtNote);

    // 7. Nút Xóa
    QPushButton *btnDelete = new QPushButton("🗑️", this);
    btnDelete->setObjectName("btnDeleteRow");
    connect(btnDelete, &QPushButton::clicked, this, &CreatePrescriptionDialog::onRemoveMedicineClicked);
    tblPrescription->setCellWidget(row, 7, btnDelete);
}

// ── BỔ SUNG 2 HÀM SLOT XỬ LÝ NÚT BẤM CÒN THIẾU ────────────────────
void CreatePrescriptionDialog::onAddMedicineClicked() {
    addMedicineRow("", 0, 1, "", "", 1, "");
}

void CreatePrescriptionDialog::onRemoveMedicineClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    for (int i = 0; i < tblPrescription->rowCount(); ++i) {
        if (tblPrescription->cellWidget(i, 7) == btn) {
            tblPrescription->removeRow(i);
            break;
        }
    }
}
// ──────────────────────────────────────────────────────────────────

void CreatePrescriptionDialog::onSaveClicked() {
    PrescriptionInputDTO dto = getPrescriptionInput();
    if (dto.items.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Vui lòng thêm ít nhất một loại thuốc vào đơn."));
        return;
    }

    if (m_pharmacyService) {
        QString errorMsg = m_pharmacyService->createPrescription(dto);
        if (errorMsg.isEmpty()) {
            QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã tạo đơn thuốc và trừ tồn kho thành công."));
            accept();
        } else {
            QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString("Không thể tạo đơn thuốc: %1").arg(errorMsg));
        }
    } else {
        // QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Dịch vụ Dược chưa được khởi tạo. Không thể tạo đơn thuốc."));
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Chưa kết nối dịch vụ nhà thuốc (PharmacyService)."));
    }

}

void CreatePrescriptionDialog::onExportPdfClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, QString::fromUtf8("Xuất PDF Đơn Thuốc"), QString::fromUtf8("DonThuoc_%1.pdf").arg(txtRecordId->text()), QString::fromUtf8("PDF Files (*.pdf)"));
    if (fileName.isEmpty()) return;

    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15));

    QPainter painter(&writer);
    QTextDocument doc;

    QString html = QString(R"(
        <h2 style='text-align: center; color: #000000;'>ĐƠN THUỐC CỦA BỆNH NHÂN</h2>
        <p style='color: #000000;'><b>Mã bệnh án:</b> %1 &nbsp;&nbsp;&nbsp;&nbsp; <b>Mã bác sĩ:</b> %2 &nbsp;&nbsp;&nbsp;&nbsp; <b>Bác sĩ kê đơn:</b> %3</p>
        <p style='color: #000000;'><b>Ghi chú chung:</b> %4</p>
        <hr/>
        <br/>
        <table border='1' cellspacing='0' cellpadding='6' style='width: 100%; border-collapse: collapse; color: #000000;'>
            <tr style='background-color: #f1f5f9;'>
                <th>STT</th>
                <th>Tên Thuốc</th>
                <th>Đơn Giá</th>
                <th>Số Lượng</th>
                <th>Liều Dùng</th>
                <th>Tần Suất</th>
                <th>Số Ngày</th>
                <th>Ghi Chú</th>
            </tr>
    )").arg(txtRecordId->text(), txtDoctorId->text(), cboDoctorName->currentText(), txtGeneralNotes->toPlainText());

    PrescriptionInputDTO dto = getPrescriptionInput();
    int stt = 1;
    for (const auto &item : dto.items) {
        html += QString(R"(
            <tr>
                <td style='text-align:center;'>%1</td>
                <td><b>%2</b></td>
                <td style='text-align:right;'>%3 đ</td>
                <td style='text-align:center;'>%4</td>
                <td>%5</td>
                <td>%6</td>
                <td style='text-align:center;'>%7</td>
                <td>%8</td>
            </tr>
        )").arg(stt++)
           .arg(item.brandName)
           .arg(QString::number(item.unitPrice, 'f', 0))
           .arg(item.quantity)
           .arg(item.dosage)
           .arg(item.frequency)
           .arg(item.durationDays)
           .arg(item.note);
    }

    html += R"(</table>)";
    doc.setHtml(html);
    doc.drawContents(&painter);

    QMessageBox::information(this, QString::fromUtf8("Thành Công"), QString::fromUtf8("Đã xuất đơn thuốc ra file PDF thành công:\n%1").arg(fileName));
}

void CreatePrescriptionDialog::setRecordId(const QString &recordId) {
    if (txtRecordId) txtRecordId->setText(recordId);
}

void CreatePrescriptionDialog::setDoctorId(const QString &doctorId) {
    if (txtDoctorId) txtDoctorId->setText(doctorId);
}

void CreatePrescriptionDialog::setDoctorName(const QString &doctorName) {
    if (cboDoctorName) {
        int index = cboDoctorName->findText(doctorName);
        if (index >= 0) {
            cboDoctorName->setCurrentIndex(index);
        } else {
            cboDoctorName->addItem(doctorName);
            cboDoctorName->setCurrentText(doctorName);
        }
    }
}

PrescriptionInputDTO CreatePrescriptionDialog::getPrescriptionInput() const {
    PrescriptionInputDTO dto;
    dto.recordId = txtRecordId->text().toInt();
    if (UserSession::getInstance().isLoggedIn() && UserSession::getInstance().getCurrentAccount()) {
        dto.doctorId = UserSession::getInstance().getCurrentAccount()->getAccountId();
    } else {
        dto.doctorId = txtDoctorId->text().toInt();
    }
    dto.notes = txtGeneralNotes->toPlainText().trimmed();

    for (int i = 0; i < tblPrescription->rowCount(); ++i) {
        PrescriptionItemDTO item;
        if (QTableWidgetItem *itemDrug = tblPrescription->item(i, 0)) {
            item.brandName = itemDrug->text().remove("🔵 ").remove("🟢 ").trimmed();
        }
        if (QTableWidgetItem *itemPrice = tblPrescription->item(i, 1)) {
            QString priceStr = itemPrice->text();
            priceStr.remove("đ").remove(" ").remove(",");
            item.unitPrice = priceStr.toDouble();
        }
        
        QSpinBox  *spnQty    = qobject_cast<QSpinBox*>(tblPrescription->cellWidget(i, 2));
        QLineEdit *txtDosage = qobject_cast<QLineEdit*>(tblPrescription->cellWidget(i, 3));
        QLineEdit *txtFreq   = qobject_cast<QLineEdit*>(tblPrescription->cellWidget(i, 4));
        QSpinBox  *spnDays   = qobject_cast<QSpinBox*>(tblPrescription->cellWidget(i, 5));
        QLineEdit *txtNote   = qobject_cast<QLineEdit*>(tblPrescription->cellWidget(i, 6));

        if (spnQty)    item.quantity     = spnQty->value();
        if (txtDosage) item.dosage       = txtDosage->text().trimmed();
        if (txtFreq)   item.frequency    = txtFreq->text().trimmed();
        if (spnDays)   item.durationDays = spnDays->value();
        if (txtNote)   item.note         = txtNote->text().trimmed();

        dto.items.append(item);
    }

    return dto;
}

void CreatePrescriptionDialog::setupStyleSheets() {
    QString style = R"(
        QDialog {
            background-color: #f8fafc;
        }
        #dialogTitle {
            font-size: 20px;
            font-weight: bold;
            color: #000000;
            padding-bottom: 5px;
        }
        #lblDetail {
            font-size: 15px;
            font-weight: bold;
            color: #000000;
            margin-top: 10px;
        }
        QLabel {
            font-weight: bold;
            color: #000000;
        }
        QLineEdit, QComboBox, QTextEdit, QSpinBox {
            border: 1px solid #cbd5e1;
            border-radius: 4px;
            padding: 4px 8px;
            background-color: #ffffff;
            color: #000000;
        }
        QComboBox QAbstractItemView {
            color: #000000;
            background-color: #ffffff;
            selection-background-color: #e2e8f0;
            selection-color: #000000;
        }
        QLineEdit:focus, QComboBox:focus, QTextEdit:focus, QSpinBox:focus {
            border: 1px solid #2563eb;
            color: #000000;
        }
        QTableWidget {
            background-color: #ffffff;
            border: 1px solid #cbd5e1;
            gridline-color: #e2e8f0;
            color: #000000;
        }
        QTableWidget::item {
            color: #000000;
        }
        QHeaderView::section {
            background-color: #f1f5f9;
            color: #000000;
            font-weight: bold;
            padding: 6px;
            border: 1px solid #cbd5e1;
        }
        #btnAddMedicine {
            background-color: #ffffff;
            border: 1px dashed #2563eb;
            color: #000000;
            font-weight: bold;
            border-radius: 4px;
            padding: 6px;
        }
        #btnAddMedicine:hover {
            background-color: #eff6ff;
            color: #000000;
        }
        #btnSave {
            background-color: #22c55e;
            color: #ffffff;
            font-weight: bold;
            border: none;
            border-radius: 4px;
            padding: 0 20px;
        }
        #btnSave:hover {
            background-color: #16a34a;
        }
        #btnExportPdf {
            background-color: #2563eb;
            color: #ffffff;
            font-weight: bold;
            border: none;
            border-radius: 4px;
            padding: 0 20px;
        }
        #btnExportPdf:hover {
            background-color: #1d4ed8;
        }
        #btnCancel {
            background-color: #ef4444;
            color: #ffffff;
            font-weight: bold;
            border: none;
            border-radius: 4px;
            padding: 0 20px;
        }
        #btnCancel:hover {
            background-color: #dc2626;
        }
        #btnDeleteRow {
            border: none;
            background: transparent;
        }
        #btnDeleteRow:hover {
            background-color: #fee2e2;
            border-radius: 3px;
        }
    )";
    this->setStyleSheet(style);
}