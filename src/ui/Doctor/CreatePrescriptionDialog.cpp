#include "CreatePrescriptionDialog.h"
#include "service/PharmacyService.h"
#include "service/PatientService.h"
#include "service/UserSession.h"
#include "../utils/UIValidationUtils.h"
#include "../../service/Validation.h"

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
#include <QRegularExpressionValidator>

CreatePrescriptionDialog::CreatePrescriptionDialog(std::shared_ptr<PharmacyService> pharmacyService,
                                                   std::shared_ptr<PatientService> patientService,
                                                   int patientId,
                                                   QWidget *parent)
    : QDialog(parent), m_pharmacyService(pharmacyService), m_patientService(patientService), m_patientId(patientId) {
    setWindowTitle(QString::fromUtf8("Tạo Đơn Thuốc Mới"));
    resize(1200, 700);

    setupUI();
    setupStyleSheets();
    updateSearchResults();
}

void CreatePrescriptionDialog::setPharmacyService(std::shared_ptr<PharmacyService> pharmacyService) {
    m_pharmacyService = pharmacyService;
}

void CreatePrescriptionDialog::setPatientService(std::shared_ptr<PatientService> patientService, int patientId) {
    m_patientService = patientService;
    m_patientId = patientId;
}

void CreatePrescriptionDialog::setupUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // =========================================================================
    // LEFT PANEL: MEDICATION SEARCH
    // =========================================================================
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);

    QLabel *lblSearchTitle = new QLabel(QString::fromUtf8("TRA CỨU BIỆT DƯỢC / HOẠT CHẤT"), this);
    lblSearchTitle->setObjectName("lblDetail");
    leftLayout->addWidget(lblSearchTitle);

    txtSearchKeyword = new QLineEdit(this);
    txtSearchKeyword->setPlaceholderText(QString::fromUtf8("Nhập tên thuốc hoặc hoạt chất để tìm kiếm..."));
    leftLayout->addWidget(txtSearchKeyword);

    tblSearchResults = new QTableWidget(0, 5, this);
    tblSearchResults->setHorizontalHeaderLabels(QStringList{
        QString::fromUtf8("Tên Thuốc"),
        QString::fromUtf8("Hoạt Chất"),
        QString::fromUtf8("Đơn vị"),
        QString::fromUtf8("Đơn Giá"),
        QString::fromUtf8("Tồn Kho")
    });
    tblSearchResults->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tblSearchResults->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tblSearchResults->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tblSearchResults->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tblSearchResults->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tblSearchResults->setSelectionBehavior(QAbstractItemView::SelectRows);
    tblSearchResults->setSelectionMode(QAbstractItemView::SingleSelection);
    tblSearchResults->verticalHeader()->setVisible(false);
    tblSearchResults->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblSearchResults->setFocusPolicy(Qt::NoFocus);
    tblSearchResults->setStyleSheet("QTableWidget { outline: none; } QTableWidget::item { outline: none; border: none; } QTableWidget::item:focus { outline: none; border: none; }");
    leftLayout->addWidget(tblSearchResults);

    QHBoxLayout *paginationLayout = new QHBoxLayout();
    btnPrevPage = new QPushButton(QString::fromUtf8("◀ Trang trước"), this);
    btnPrevPage->setObjectName("btnPagination");
    btnPrevPage->setFixedWidth(110);
    lblPageIndicator = new QLabel(QString::fromUtf8("Trang 1/1"), this);
    lblPageIndicator->setAlignment(Qt::AlignCenter);
    btnNextPage = new QPushButton(QString::fromUtf8("Trang sau ▶"), this);
    btnNextPage->setObjectName("btnPagination");
    btnNextPage->setFixedWidth(110);
    paginationLayout->addWidget(btnPrevPage);
    paginationLayout->addWidget(lblPageIndicator, 1);
    paginationLayout->addWidget(btnNextPage);
    leftLayout->addLayout(paginationLayout);

    // Selected Info Area
    QFrame *detailFrame = new QFrame(this);
    detailFrame->setObjectName("detailFrame");
    detailFrame->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout *detailLayout = new QVBoxLayout(detailFrame);
    detailLayout->setContentsMargins(10, 10, 10, 10);
    lblMedicationDetails = new QLabel(this);
    lblMedicationDetails->setWordWrap(true);
    lblMedicationDetails->setText(QString::fromUtf8("<i>Chọn một loại thuốc từ bảng tìm kiếm để xem chi tiết.</i>"));
    detailLayout->addWidget(lblMedicationDetails);
    leftLayout->addWidget(detailFrame);

    btnAddSelected = new QPushButton(QString::fromUtf8("Thêm Vào Đơn Thuốc"), this);
    btnAddSelected->setObjectName("btnSave"); 
    btnAddSelected->setFixedHeight(38);
    btnAddSelected->setEnabled(false);
    leftLayout->addWidget(btnAddSelected);

    // =========================================================================
    // RIGHT PANEL: PRESCRIPTION DETAILS
    // =========================================================================
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    QLabel *lblTitle = new QLabel(QString::fromUtf8("ĐƠN THUỐC CỦA BỆNH NHÂN"), this);
    lblTitle->setObjectName("dialogTitle");
    lblTitle->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(lblTitle);

    QHBoxLayout *topInfoLayout = new QHBoxLayout();
    topInfoLayout->setSpacing(10);

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
    rightLayout->addLayout(topInfoLayout);

    QHBoxLayout *notesLayout = new QHBoxLayout();
    QLabel *lblNotes = new QLabel(QString::fromUtf8("Ghi Chú Chung:"), this);
    txtGeneralNotes = new QTextEdit(this);
    txtGeneralNotes->setFixedHeight(45);
    txtGeneralNotes->setPlaceholderText(QString::fromUtf8("Nhập ghi chú chung cho đơn thuốc (nếu có)..."));
    notesLayout->addWidget(lblNotes);
    notesLayout->addWidget(txtGeneralNotes);
    rightLayout->addLayout(notesLayout);

    QLabel *lblDetail = new QLabel(QString::fromUtf8("Chi Tiết Toa Thuốc"), this);
    lblDetail->setObjectName("lblDetail");
    rightLayout->addWidget(lblDetail);

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
    tblPrescription->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    tblPrescription->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    tblPrescription->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
    tblPrescription->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive);
    tblPrescription->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
    tblPrescription->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
    tblPrescription->setColumnWidth(2, 90);   // Số Lượng
    tblPrescription->setColumnWidth(3, 110);  // Liều Lượng
    tblPrescription->setColumnWidth(4, 110);  // Tần Suất
    tblPrescription->setColumnWidth(5, 90);   // Số Ngày
    tblPrescription->setColumnWidth(7, 60);   // Nút Xóa
    tblPrescription->verticalHeader()->setVisible(false);
    tblPrescription->verticalHeader()->setDefaultSectionSize(38); // Đặt chiều cao dòng phù hợp để chứa widget (QSpinBox, QLineEdit)
    tblPrescription->setFocusPolicy(Qt::NoFocus);
    tblPrescription->setStyleSheet("QTableWidget { outline: none; } QTableWidget::item { outline: none; border: none; } QTableWidget::item:focus { outline: none; border: none; }");
    rightLayout->addWidget(tblPrescription);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();

    btnSave = new QPushButton(QString::fromUtf8("Lưu Đơn Thuốc"), this);
    btnSave->setObjectName("btnSave");
    btnSave->setFixedHeight(38);

    btnExportPdf = new QPushButton(QString::fromUtf8("Xuất PDF"), this);
    btnExportPdf->setObjectName("btnExportPdf");
    btnExportPdf->setFixedHeight(38);

    btnCancel = new QPushButton(QString::fromUtf8("Hủy Bỏ"), this);
    btnCancel->setObjectName("btnCancel");
    btnCancel->setFixedHeight(38);

    bottomLayout->addWidget(btnSave);
    bottomLayout->addWidget(btnExportPdf);
    bottomLayout->addWidget(btnCancel);
    rightLayout->addLayout(bottomLayout);

    // Add Left and Right Panels to main layout
    mainLayout->addWidget(leftPanel, 4); 
    mainLayout->addWidget(rightPanel, 6); 

    // Connections
    connect(txtSearchKeyword, &QLineEdit::textChanged, this, &CreatePrescriptionDialog::onSearchClicked);
    connect(btnPrevPage, &QPushButton::clicked, this, &CreatePrescriptionDialog::onPrevPageClicked);
    connect(btnNextPage, &QPushButton::clicked, this, &CreatePrescriptionDialog::onNextPageClicked);
    connect(tblSearchResults, &QTableWidget::itemSelectionChanged, this, &CreatePrescriptionDialog::onSearchTableSelectionChanged);
    connect(btnAddSelected, &QPushButton::clicked, this, &CreatePrescriptionDialog::onAddSelectedClicked);
    connect(btnSave, &QPushButton::clicked, this, &CreatePrescriptionDialog::onSaveClicked);
    connect(btnExportPdf, &QPushButton::clicked, this, &CreatePrescriptionDialog::onExportPdfClicked);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void CreatePrescriptionDialog::updateSearchResults() {
    if (!m_pharmacyService) return;

    MedicationSearchCriteria criteria;
    criteria.keyword = txtSearchKeyword->text().trimmed();
    criteria.page = m_searchPage;
    criteria.pageSize = m_searchPageSize;
    criteria.inStockOnly = false; 
    criteria.excludeExpired = false; 

    PagedResult<MedicationSummaryDTO> result = m_pharmacyService->searchMedicationsPaged(criteria);
    m_currentSearchResults = result.items;

    m_searchTotalPages = qMax(1, (result.totalCount + m_searchPageSize - 1) / m_searchPageSize);

    btnPrevPage->setEnabled(m_searchPage > 1);
    btnNextPage->setEnabled(m_searchPage < m_searchTotalPages);
    lblPageIndicator->setText(QString("Trang %1/%2").arg(m_searchPage).arg(m_searchTotalPages));

    tblSearchResults->setRowCount(0);
    for (int i = 0; i < m_currentSearchResults.size(); ++i) {
        const auto &med = m_currentSearchResults[i];
        tblSearchResults->insertRow(i);

        // 0. Tên thuốc
        QTableWidgetItem *itemBrand = new QTableWidgetItem(med.brandName);
        itemBrand->setData(Qt::UserRole, i); 
        tblSearchResults->setItem(i, 0, itemBrand);

        // 1. Hoạt chất
        QStringList ingList;
        for (const auto &ing : med.ingredients) {
            ingList.append(QString("%1 (%2)").arg(ing.ingredientName, ing.strength));
        }
        QTableWidgetItem *itemIngs = new QTableWidgetItem(ingList.join(", "));
        tblSearchResults->setItem(i, 1, itemIngs);

        // 2. Đơn vị
        QTableWidgetItem *itemUnit = new QTableWidgetItem(med.unit);
        tblSearchResults->setItem(i, 2, itemUnit);

        // 3. Đơn giá
        QTableWidgetItem *itemPrice = new QTableWidgetItem(QString("%1 đ").arg(med.unitPrice, 0, 'f', 0));
        itemPrice->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tblSearchResults->setItem(i, 3, itemPrice);

        // 4. Tồn kho
        QTableWidgetItem *itemStock = new QTableWidgetItem(QString::number(med.stockQuantity));
        itemStock->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tblSearchResults->setItem(i, 4, itemStock);

        // Color coding based on status
        QColor textColor(0, 0, 0);
        if (!med.isActive) {
            textColor = QColor(156, 163, 175); // gray
        } else if (med.expiryDate.isValid() && med.expiryDate <= QDate::currentDate()) {
            textColor = QColor(239, 68, 68); // red
        } else if (med.stockQuantity <= 0) {
            textColor = QColor(249, 115, 22); // orange
        }

        for (int col = 0; col < 5; ++col) {
            if (auto item = tblSearchResults->item(i, col)) {
                item->setForeground(QBrush(textColor));
            }
        }
    }

    tblSearchResults->clearSelection();
    m_selectedMedication = std::nullopt;
    updateSelectedDetails();
}

void CreatePrescriptionDialog::updateSelectedDetails() {
    int selectedRow = tblSearchResults->currentRow();
    if (selectedRow < 0 || selectedRow >= m_currentSearchResults.size()) {
        m_selectedMedication = std::nullopt;
        lblMedicationDetails->setText(QString::fromUtf8("<i>Chọn một loại thuốc từ bảng tìm kiếm để xem chi tiết.</i>"));
        btnAddSelected->setEnabled(false);
        return;
    }

    const auto &med = m_currentSearchResults[selectedRow];
    m_selectedMedication = med;

    QString statusHtml;
    bool canPrescribe = true;

    if (!med.isActive) {
        statusHtml = QString::fromUtf8("<span style='color: #ef4444; font-weight: bold;'>[Ngừng Kinh Doanh]</span>");
        canPrescribe = false;
    } else if (med.expiryDate.isValid() && med.expiryDate <= QDate::currentDate()) {
        statusHtml = QString::fromUtf8("<span style='color: #ef4444; font-weight: bold;'>[Đã Hết Hạn: %1]</span>").arg(med.expiryDate.toString("dd/MM/yyyy"));
        canPrescribe = false;
    } else if (med.stockQuantity <= 0) {
        statusHtml = QString::fromUtf8("<span style='color: #f97316; font-weight: bold;'>[Hết Hàng Trong Kho]</span>");
        canPrescribe = false;
    } else {
        statusHtml = QString::fromUtf8("<span style='color: #22c55e; font-weight: bold;'>[Có thể kê đơn]</span>");
    }

    QString details = QString(R"(
        <p style='margin: 0; padding: 0;'>
            <b>Tên thuốc:</b> %1 &nbsp;&nbsp;|&nbsp;&nbsp; <b>Nhà SX:</b> %2<br/>
            <b>Đơn giá:</b> %3 đ / %4 &nbsp;&nbsp;|&nbsp;&nbsp; <b>Tồn kho:</b> %5 %6<br/>
            <b>Hoạt chất:</b> %7<br/>
            <b>Mô tả:</b> %8<br/>
            <b>Trạng thái:</b> %9
        </p>
    )").arg(med.brandName)
       .arg(med.manufacturer.isEmpty() ? "---" : med.manufacturer)
       .arg(QString::number(med.unitPrice, 'f', 0))
       .arg(med.unit)
       .arg(med.stockQuantity)
       .arg(med.unit)
       .arg(tblSearchResults->item(selectedRow, 1)->text())
       .arg(med.description.isEmpty() ? "---" : med.description)
       .arg(statusHtml);

    lblMedicationDetails->setText(details);
    btnAddSelected->setEnabled(canPrescribe);
}

void CreatePrescriptionDialog::onSearchClicked() {
    m_searchPage = 1;
    updateSearchResults();
}

void CreatePrescriptionDialog::onPrevPageClicked() {
    if (m_searchPage > 1) {
        m_searchPage--;
        updateSearchResults();
    }
}

void CreatePrescriptionDialog::onNextPageClicked() {
    if (m_searchPage < m_searchTotalPages) {
        m_searchPage++;
        updateSearchResults();
    }
}

void CreatePrescriptionDialog::onSearchTableSelectionChanged() {
    updateSelectedDetails();
}

void CreatePrescriptionDialog::onAddSelectedClicked() {
    if (!m_selectedMedication.has_value()) return;
    const auto &med = m_selectedMedication.value();

    for (int i = 0; i < tblPrescription->rowCount(); ++i) {
        if (QTableWidgetItem *itemDrug = tblPrescription->item(i, 0)) {
            int medId = itemDrug->data(Qt::UserRole).toInt();
            if (medId == med.medicationId) {
                QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), 
                                     QString::fromUtf8("Thuốc \"%1\" đã có trong đơn thuốc. Bạn có thể thay đổi số lượng trực tiếp tại bảng đơn thuốc.").arg(med.brandName));
                return;
            }
        }
    }

    addMedicineRow(med.medicationId, med.brandName, med.unitPrice, 1, "", "", 1, "", med.stockQuantity);
}

void CreatePrescriptionDialog::addMedicineRow(int medicationId, const QString &name, double price, int qty, const QString &dosage, const QString &freq, int days, const QString &note, int maxStock) {
    int row = tblPrescription->rowCount();
    tblPrescription->insertRow(row);
    tblPrescription->setRowHeight(row, 38); // Đảm bảo dòng mới có chiều cao 38px để không bị cắt xén nội dung

    // 0. Tên thuốc
    QTableWidgetItem *itemDrug = new QTableWidgetItem(name);
    itemDrug->setForeground(QBrush(QColor(0, 0, 0)));
    itemDrug->setData(Qt::UserRole, medicationId); 
    tblPrescription->setItem(row, 0, itemDrug);

    // 1. Đơn giá
    QTableWidgetItem *itemPrice = new QTableWidgetItem(QString("%1 đ").arg(price, 0, 'f', 0));
    itemPrice->setForeground(QBrush(QColor(0, 0, 0)));
    itemPrice->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    tblPrescription->setItem(row, 1, itemPrice);

    // 2. Số lượng
    QSpinBox *spnQty = new QSpinBox(this);
    spnQty->setRange(1, maxStock);
    spnQty->setValue(qty);
    tblPrescription->setCellWidget(row, 2, spnQty);

    // 3. Liều Lượng
    QLineEdit *txtDosage = new QLineEdit(dosage, this);
    txtDosage->setPlaceholderText(QString::fromUtf8("Ví dụ: 1"));
    txtDosage->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9]+(?:\\.[0-9]+)?$"), txtDosage));
    txtDosage->setStyleSheet("QLineEdit { border: 1px solid #cbd5e1; border-radius: 4px; padding: 2px 6px; background-color: #ffffff; color: #0f172a; min-height: 24px; }");
    connect(txtDosage, &QLineEdit::editingFinished, this, [txtDosage]() {
        QString err = txtDosage->text().trimmed().isEmpty() ? "Vui lòng nhập liều lượng" : "";
        UIValidationUtils::applyTableFieldValidationStyle(txtDosage, err);
    });
    tblPrescription->setCellWidget(row, 3, txtDosage);

    // 4. Tần Suất
    QLineEdit *txtFreq = new QLineEdit(freq, this);
    txtFreq->setPlaceholderText(QString::fromUtf8("Ví dụ: 2"));
    txtFreq->setValidator(new QRegularExpressionValidator(QRegularExpression("^[0-9]+$"), txtFreq));
    txtFreq->setStyleSheet("QLineEdit { border: 1px solid #cbd5e1; border-radius: 4px; padding: 2px 6px; background-color: #ffffff; color: #0f172a; min-height: 24px; }");
    connect(txtFreq, &QLineEdit::editingFinished, this, [txtFreq]() {
        QString err = txtFreq->text().trimmed().isEmpty() ? "Vui lòng nhập tần suất" : "";
        UIValidationUtils::applyTableFieldValidationStyle(txtFreq, err);
    });
    tblPrescription->setCellWidget(row, 4, txtFreq);

    // 5. Số Ngày
    QSpinBox *spnDays = new QSpinBox(this);
    spnDays->setRange(1, 365);
    spnDays->setValue(days);
    tblPrescription->setCellWidget(row, 5, spnDays);

    // 6. Ghi Chú
    QLineEdit *txtNote = new QLineEdit(note, this);
    txtNote->setPlaceholderText(QString::fromUtf8("Ghi chú..."));
    txtNote->setStyleSheet("QLineEdit { border: 1px solid #cbd5e1; border-radius: 4px; padding: 2px 6px; background-color: #ffffff; color: #0f172a; min-height: 24px; }");
    tblPrescription->setCellWidget(row, 6, txtNote);

    // 7. Nút Xóa
    QPushButton *btnDelete = new QPushButton("Xóa", this);
    btnDelete->setObjectName("btnDeleteRow");
    connect(btnDelete, &QPushButton::clicked, this, &CreatePrescriptionDialog::onRemoveMedicineClicked);
    tblPrescription->setCellWidget(row, 7, btnDelete);
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

void CreatePrescriptionDialog::onSaveClicked() {
    PrescriptionInputDTO dto = getPrescriptionInput();
    if (dto.items.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Vui lòng thêm ít nhất một loại thuốc vào đơn."));
        return;
    }

    if (!m_pharmacyService) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Chưa kết nối dịch vụ nhà thuốc (PharmacyService)."));
        return;
    }

    // ────────────────────────────────────────────────────────────────
    // CLINICAL DRUG SAFETY CHECKS (Duplications & Allergies)
    // ────────────────────────────────────────────────────────────────
    QList<AllergyResultDTO> allergies;
    if (m_patientService && m_patientId > 0) {
        allergies = m_patientService->getAllergies(m_patientId);
    }

    PrescriptionSafetyReport report = m_pharmacyService->checkPrescriptionSafety(dto.items, allergies);

    if (!report.isSafe()) {
        QString warningMsg;

        // 1. Handle Severe Allergy Conflicts (BLOCK SAVING)
        if (report.hasSevereConflict()) {
            warningMsg = QString::fromUtf8("<b>CẢNH BÁO AN TOÀN ĐƠN THUỐC: Phát hiện dị ứng nghiêm trọng!</b><br/>"
                                           "Thuốc được kê chứa hoạt chất mà bệnh nhân có tiền sử dị ứng <b>NẶNG</b>:<br/><br/>");
            for (const auto &conflict : report.allergyConflicts) {
                if (conflict.severity == Severity::Severe) {
                    QStringList meds;
                    for (const auto &m : conflict.conflictingMedications) meds.append(m.second);
                    warningMsg += QString::fromUtf8("- Hoạt chất: <b>%1</b><br/>  Thuốc kê xung đột: <i>%2</i><br/><br/>")
                                  .arg(conflict.ingredientName, 
                                       meds.join(", "));
                }
            }
            warningMsg += QString::fromUtf8("<b>Vì lý do an toàn, hệ thống không thể lưu đơn thuốc này. Vui lòng điều chỉnh lại thuốc gây dị ứng.</b>");
            
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(QString::fromUtf8("Cảnh Báo Dị Ứng Nghiêm Trọng"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(warningMsg);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
            return;
        }

        // 2. Handle Mild/Moderate Allergy Conflicts & Duplications (WARN & CONFIRM BYPASS)
        warningMsg = QString::fromUtf8("<b>CẢNH BÁO AN TOÀN ĐƠN THUỐC</b><br/>"
                                       "Phát hiện một số cảnh báo khi kê đơn:<br/><br/>");

        if (report.hasAllergyConflict()) {
            warningMsg += QString::fromUtf8("<b>[Dị ứng thuốc Nhẹ/Vừa]:</b><br/>");
            for (const auto &conflict : report.allergyConflicts) {
                if (conflict.severity != Severity::Severe) {
                    QStringList meds;
                    for (const auto &m : conflict.conflictingMedications) meds.append(m.second);
                    QString severityText = (conflict.severity == Severity::Mild) ? QString::fromUtf8("Nhẹ") : QString::fromUtf8("Vừa");
                    warningMsg += QString::fromUtf8("- Bệnh nhân dị ứng mức <b>%1</b> với hoạt chất <b>%2</b>. Thuốc kê: <i>%3</i><br/>")
                                  .arg(severityText, conflict.ingredientName, 
                                       meds.join(", "));
                }
            }
            warningMsg += "<br/>";
        }

        if (report.hasDuplication()) {
            warningMsg += QString::fromUtf8("<b>[Trùng lặp hoạt chất]:</b><br/>");
            for (const auto &dup : report.duplications) {
                QStringList meds;
                for (const auto &m : dup.conflictingMedications) meds.append(m.second);
                warningMsg += QString::fromUtf8("- Hoạt chất <b>%1</b> được kê trùng lặp trong các thuốc: <i>%2</i><br/>")
                              .arg(dup.ingredientName, meds.join(", "));
            }
            warningMsg += "<br/>";
        }

        warningMsg += QString::fromUtf8("Bạn có chắc chắn muốn bỏ qua và tiếp tục lưu đơn thuốc không?");

        QMessageBox msgBox(this);
        msgBox.setWindowTitle(QString::fromUtf8("Xác Nhận Kê Đơn"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(warningMsg);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (msgBox.exec() != QMessageBox::Yes) {
            return; 
        }
    }

    QString errorMsg = m_pharmacyService->createPrescription(dto);
    if (errorMsg.isEmpty()) {
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã tạo đơn thuốc thành công và tự động chuyển đơn sang Hàng đợi Dược sĩ để cấp phát!"));
        accept();
    } else {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString("Không thể tạo đơn thuốc: %1").arg(errorMsg));
    }
}

void CreatePrescriptionDialog::onExportPdfClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, QString::fromUtf8("Xuất PDF Đơn Thuốc"), QString::fromUtf8("DonThuoc_%1.pdf").arg(txtRecordId->text()), QString::fromUtf8("PDF Files (*.pdf)"));
    if (fileName.isEmpty()) return;

    QPdfWriter writer(fileName);
    writer.setResolution(96);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument doc;
    doc.setPageSize(QSizeF(writer.width(), writer.height()));

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
    doc.print(&writer);

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

void CreatePrescriptionDialog::onPrescriptionCellChanged(int row, int col) {
    Q_UNUSED(row);
    Q_UNUSED(col);
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
            item.medicationId = itemDrug->data(Qt::UserRole).toInt();
            item.brandName = itemDrug->text().trimmed();
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
            color: #0f172a;
            padding-bottom: 5px;
        }
        #lblDetail {
            font-size: 15px;
            font-weight: bold;
            color: #1e293b;
            margin-top: 10px;
        }
        QLabel {
            font-weight: bold;
            color: #334155;
        }
        QLineEdit, QComboBox, QTextEdit, QSpinBox {
            border: 1px solid #cbd5e1;
            border-radius: 4px;
            padding: 4px 8px;
            background-color: #ffffff;
            color: #0f172a;
        }
        QComboBox QAbstractItemView {
            color: #0f172a;
            background-color: #ffffff;
            selection-background-color: #e2e8f0;
            selection-color: #0f172a;
        }
        QLineEdit:focus, QComboBox:focus, QTextEdit:focus, QSpinBox:focus {
            border: 1px solid #2563eb;
            color: #0f172a;
        }
        QTableWidget {
            background-color: #ffffff;
            border: 1px solid #cbd5e1;
            gridline-color: #e2e8f0;
            color: #0f172a;
            selection-background-color: #eff6ff;
            selection-color: #1e40af;
        }
        QHeaderView::section {
            background-color: #eff6ff;
            color: #1e40af;
            font-weight: bold;
            padding: 6px;
            border: none;
            border-bottom: 2px solid #bfdbfe;
        }
        #detailFrame {
            background-color: #ffffff;
            border: 1px solid #cbd5e1;
            border-radius: 4px;
        }
        #btnSearch {
            background-color: #2563eb;
            color: #ffffff;
            font-weight: bold;
            border: none;
            border-radius: 4px;
        }
        #btnSearch:hover {
            background-color: #1d4ed8;
        }
        #btnPagination {
            background-color: #ffffff;
            color: #0f172a;
            border: 1px solid #cbd5e1;
            border-radius: 4px;
            font-weight: bold;
            padding: 4px 8px;
        }
        #btnPagination:hover {
            background-color: #f1f5f9;
        }
        #btnPagination:disabled {
            background-color: #f8fafc;
            color: #94a3b8;
            border: 1px solid #e2e8f0;
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