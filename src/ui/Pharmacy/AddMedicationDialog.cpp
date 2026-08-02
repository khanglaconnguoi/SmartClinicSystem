#include "AddMedicationDialog.h"
#include "service/PharmacyService.h"
#include "dto/MedicationDTOs.h"
#include "model/CommonEnums.h"
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QCheckBox>
#include <QDateTime>

// ============================================================================
// IMPLEMENTATION: ACTIVE INGREDIENT SEARCH DIALOG
// ============================================================================
ActiveIngredientSearchDialog::ActiveIngredientSearchDialog(std::shared_ptr<PharmacyService> pharmacyService, QWidget* parent)
    : QDialog(parent), m_pharmacyService(pharmacyService), m_currentPage(1), m_pageSize(10), m_totalPages(1) {
    setWindowTitle("Tra cứu hoạt chất");
    resize(550, 450);
    setupUI();
    performSearch();
}

void ActiveIngredientSearchDialog::performSearch() {
    m_currentPage = 1; 
    loadPageData();
}

void ActiveIngredientSearchDialog::goToPreviousPage() {
    if (m_currentPage > 1) {
        m_currentPage--;
        loadPageData();
    }
}

void ActiveIngredientSearchDialog::goToNextPage() {
    if (m_currentPage < m_totalPages) {
        m_currentPage++;
        loadPageData();
    }
}

void ActiveIngredientSearchDialog::handleRowClicked(int row) {
    if (row < 0) return;

    int ingredientId = m_resultsTable->item(row, 0)->text().toInt();
    QString ingredientName = m_resultsTable->item(row, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Xác nhận", 
        QString("Bạn có muốn chọn hoạt chất \"%1\" không?").arg(ingredientName),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        emit ingredientSelected(ingredientId, ingredientName);
        accept();
    }
}

void ActiveIngredientSearchDialog::loadPageData() {
    m_resultsTable->setRowCount(0);
    
    IngredientSearchCriteria criteria;
    criteria.keyword = m_searchBar->text().simplified();
    criteria.page = m_currentPage;
    criteria.pageSize = m_pageSize;

    auto result = m_pharmacyService->searchIngredientsPaged(criteria);
    
    m_totalPages = (result.totalCount + m_pageSize - 1) / m_pageSize;
    if (m_totalPages < 1) m_totalPages = 1;
    m_currentPage = result.page;

    int row = 0;
    for (const auto& item : result.items) {
        m_resultsTable->insertRow(row);
        
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(item.ingredientId));
        QTableWidgetItem* nameItem = new QTableWidgetItem(item.ingredientName);
        QTableWidgetItem* descItem = new QTableWidgetItem(item.description);

        idItem->setTextAlignment(Qt::AlignCenter);

        m_resultsTable->setItem(row, 0, idItem);
        m_resultsTable->setItem(row, 1, nameItem);
        m_resultsTable->setItem(row, 2, descItem);
        row++;
    }

    m_lblPageInfo->setText(QString("Trang %1 / %2").arg(m_currentPage).arg(m_totalPages));
    updatePaginationButtons();
}

void ActiveIngredientSearchDialog::updatePaginationButtons() {
    m_btnPrev->setEnabled(m_currentPage > 1);
    m_btnNext->setEnabled(m_currentPage < m_totalPages);
}

void ActiveIngredientSearchDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText("Nhập tên hoạt chất để tìm kiếm...");
    QPushButton* btnSearch = new QPushButton("Tìm kiếm", this);
    btnSearch->setStyleSheet("background-color: #4B94F2; color: white; padding: 6px 12px; border-radius: 4px; border: none; font-weight: bold;");
    searchLayout->addWidget(m_searchBar);
    searchLayout->addWidget(btnSearch);
    mainLayout->addLayout(searchLayout);

    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(3);
    m_resultsTable->setHorizontalHeaderLabels({"ID", "Tên hoạt chất", "Mô tả"});
    m_resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(m_resultsTable);

    QHBoxLayout* pagingLayout = new QHBoxLayout();
    m_btnPrev = new QPushButton("Trước", this);
    m_btnNext = new QPushButton("Sau", this);
    m_lblPageInfo = new QLabel("Trang 1 / 1", this);
    pagingLayout->addWidget(m_btnPrev);
    pagingLayout->addWidget(m_lblPageInfo);
    pagingLayout->addWidget(m_btnNext);
    mainLayout->addLayout(pagingLayout);

    m_btnClose = new QPushButton("Đóng", this);
    m_btnClose->setStyleSheet("background-color: #F3F4F6; color: #374151; padding: 6px 12px; border-radius: 4px; border: 1px solid #D1D5DB;");
    mainLayout->addWidget(m_btnClose);

    connect(btnSearch, &QPushButton::clicked, this, &ActiveIngredientSearchDialog::performSearch);
    connect(m_searchBar, &QLineEdit::returnPressed, this, &ActiveIngredientSearchDialog::performSearch);
    connect(m_btnPrev, &QPushButton::clicked, this, &ActiveIngredientSearchDialog::goToPreviousPage);
    connect(m_btnNext, &QPushButton::clicked, this, &ActiveIngredientSearchDialog::goToNextPage);
    connect(m_resultsTable, &QTableWidget::cellDoubleClicked, this, &ActiveIngredientSearchDialog::handleRowClicked);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::reject);
}


// ============================================================================
// IMPLEMENTATION: ADD / EDIT MEDICATION DIALOG
// ============================================================================
AddMedicationDialog::AddMedicationDialog(std::shared_ptr<PharmacyService> pharmacyService, int medicationId, QWidget* parent)
    : QDialog(parent), m_pharmacyService(pharmacyService), m_medicationId(medicationId) {
    setWindowTitle(m_medicationId == -1 ? "Thêm thuốc mới vào kho" : "Chỉnh sửa thông tin thuốc");
    resize(700, 650);
    setupUI();
    loadCommonEnums();
    if (m_medicationId != -1) {
        loadMedicationData();
    }
}

void AddMedicationDialog::loadCommonEnums() {
    // Thêm các đơn vị thuốc cơ bản
    m_cbUnit->addItems({"Viên", "Chai", "Vỉ", "Hộp", "Tuýp", "Gói", "Ống"});
}

void AddMedicationDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 28);
    mainLayout->setSpacing(16);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget* formContainer = new QWidget(scrollArea);
    QVBoxLayout* formLayout = new QVBoxLayout(formContainer);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(16);

    // 1. Group Box: Thông tin chung
    QGroupBox* grpGeneral = new QGroupBox("Thông tin chung", formContainer);
    QGridLayout* gridGeneral = new QGridLayout(grpGeneral);
    gridGeneral->setSpacing(12);

    gridGeneral->addWidget(new QLabel("Tên thương hiệu (Brand Name):*"), 0, 0);
    m_txtBrandName = new QLineEdit(grpGeneral);
    m_txtBrandName->setPlaceholderText("Ví dụ: Paracetamol 500mg");
    gridGeneral->addWidget(m_txtBrandName, 0, 1, 1, 3);

    gridGeneral->addWidget(new QLabel("Đơn vị tính:*"), 1, 0);
    m_cbUnit = new QComboBox(grpGeneral);
    gridGeneral->addWidget(m_cbUnit, 1, 1);

    gridGeneral->addWidget(new QLabel("Đơn giá (VND):*"), 1, 2);
    m_spinUnitPrice = new QDoubleSpinBox(grpGeneral);
    m_spinUnitPrice->setRange(0, 100000000);
    m_spinUnitPrice->setSingleStep(500);
    m_spinUnitPrice->setDecimals(1);
    m_spinUnitPrice->setValue(1000);
    gridGeneral->addWidget(m_spinUnitPrice, 1, 3);

    gridGeneral->addWidget(new QLabel("Nhà sản xuất:"), 2, 0);
    m_txtManufacturer = new QLineEdit(grpGeneral);
    gridGeneral->addWidget(m_txtManufacturer, 2, 1, 1, 3);

    gridGeneral->addWidget(new QLabel("Hạn sử dụng:*"), 3, 0);
    m_dateExpiry = new QDateEdit(grpGeneral);
    m_dateExpiry->setCalendarPopup(true);
    m_dateExpiry->setDate(QDate::currentDate().addYears(2));
    gridGeneral->addWidget(m_dateExpiry, 3, 1);

    formLayout->addWidget(grpGeneral);

    // 2. Group Box: Cấu hình tồn kho
    QGroupBox* grpStock = new QGroupBox("Cấu hình tồn kho", formContainer);
    QGridLayout* gridStock = new QGridLayout(grpStock);
    gridStock->setSpacing(12);

    gridStock->addWidget(new QLabel("Số lượng tồn kho ban đầu:*"), 0, 0);
    m_spinStockQuantity = new QSpinBox(grpStock);
    m_spinStockQuantity->setRange(0, 1000000);
    m_spinStockQuantity->setValue(100);
    gridStock->addWidget(m_spinStockQuantity, 0, 1);

    gridStock->addWidget(new QLabel("Tồn kho tối thiểu (Min):*"), 0, 2);
    m_spinMinimumStock = new QSpinBox(grpStock);
    m_spinMinimumStock->setRange(0, 100000);
    m_spinMinimumStock->setValue(10);
    gridStock->addWidget(m_spinMinimumStock, 0, 3);

    gridStock->addWidget(new QLabel("Ngưỡng cảnh báo mua thêm:*"), 1, 0);
    m_spinReorderThreshold = new QSpinBox(grpStock);
    m_spinReorderThreshold->setRange(0, 100000);
    m_spinReorderThreshold->setValue(20);
    gridStock->addWidget(m_spinReorderThreshold, 1, 1);

    formLayout->addWidget(grpStock);

    // 3. Group Box: Danh mục thuốc (Sử dụng QGridLayout để wrap các checkbox)
    QGroupBox* grpCategories = new QGroupBox("Danh mục thuốc", formContainer);
    m_categoriesLayout = new QGridLayout(grpCategories);
    m_categoriesLayout->setSpacing(10);
    
    QList<QString> defaultCategories = {"Kháng sinh", "Giảm đau", "Kháng viêm", "Hạ sốt", "Tim mạch", "Tiêu hóa", "Khác"};
    int cRow = 0, cCol = 0;
    for (const auto& catName : defaultCategories) {
        QCheckBox* chk = new QCheckBox(catName, grpCategories);
        m_categoryCheckBoxes.append(chk);
        m_categoriesLayout->addWidget(chk, cRow, cCol);
        cCol++;
        if (cCol >= 4) { cCol = 0; cRow++; }
    }
    formLayout->addWidget(grpCategories);

    // 4. Group Box: Thành phần hoạt chất (Ingredients)
    QGroupBox* grpIngredients = new QGroupBox("Thành phần hoạt chất", formContainer);
    QVBoxLayout* layIngredients = new QVBoxLayout(grpIngredients);
    
    m_ingredientsTable = new QTableWidget(grpIngredients);
    m_ingredientsTable->setColumnCount(3);
    m_ingredientsTable->setHorizontalHeaderLabels({"ID hoạt chất", "Tên hoạt chất", "Hàm lượng (Ví dụ: 500mg)"});
    m_ingredientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_ingredientsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_ingredientsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_ingredientsTable->verticalHeader()->setDefaultSectionSize(40);
    m_ingredientsTable->setMinimumHeight(180); // Đảm bảo bảng hoạt chất có chiều cao cố định để cuộn mượt mà
    layIngredients->addWidget(m_ingredientsTable);

    QHBoxLayout* btnIngredientsLayout = new QHBoxLayout();
    m_btnAddIngredient = new QPushButton("Thêm hoạt chất", grpIngredients);
    m_btnAddIngredient->setStyleSheet("background-color: #EFF6FF; color: #2563EB; border: 1px solid #2563EB; font-weight: bold; padding: 6px 12px; border-radius: 6px;");
    m_btnRemoveIngredient = new QPushButton("Xóa hoạt chất", grpIngredients);
    m_btnRemoveIngredient->setStyleSheet("background-color: #FEE2E2; color: #DC2626; border: 1px solid #DC2626; font-weight: bold; padding: 6px 12px; border-radius: 6px;");
    btnIngredientsLayout->addWidget(m_btnAddIngredient);
    btnIngredientsLayout->addWidget(m_btnRemoveIngredient);
    btnIngredientsLayout->addStretch();
    layIngredients->addLayout(btnIngredientsLayout);

    formLayout->addWidget(grpIngredients);

    // 5. Group Box: Mô tả khác
    QGroupBox* grpDescription = new QGroupBox("Mô tả / Chỉ định khác", formContainer);
    QVBoxLayout* layDescription = new QVBoxLayout(grpDescription);
    m_txtDescription = new QTextEdit(grpDescription);
    m_txtDescription->setPlaceholderText("Nhập thông tin chỉ định, chống chỉ định, hoặc ghi chú khác...");
    m_txtDescription->setMaximumHeight(80);
    layDescription->addWidget(m_txtDescription);
    formLayout->addWidget(grpDescription);

    scrollArea->setWidget(formContainer);
    mainLayout->addWidget(scrollArea);

    // 6. Nút điều khiển lưu / hủy
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_btnCancel = new QPushButton("Hủy bỏ", this);
    m_btnCancel->setFixedWidth(110);
    m_btnCancel->setFixedHeight(38);
    m_btnCancel->setStyleSheet("background-color: #F3F4F6; color: #374151; border: 1px solid #D1D5DB; border-radius: 6px; padding: 8px 16px; font-weight: bold;");
    
    m_btnSave = new QPushButton("Lưu lại", this);
    m_btnSave->setFixedWidth(110);
    m_btnSave->setFixedHeight(38);
    m_btnSave->setStyleSheet("background-color: #2563EB; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold;");

    buttonLayout->addWidget(m_btnCancel);
    buttonLayout->addWidget(m_btnSave);
    mainLayout->addLayout(buttonLayout);

    // Connect signals
    connect(m_btnAddIngredient, &QPushButton::clicked, this, &AddMedicationDialog::openIngredientSearch);
    connect(m_btnRemoveIngredient, &QPushButton::clicked, this, &AddMedicationDialog::removeSelectedIngredient);
    connect(m_btnSave, &QPushButton::clicked, this, &AddMedicationDialog::handleSave);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void AddMedicationDialog::loadMedicationData() {
    auto optMed = m_pharmacyService->getMedicationById(m_medicationId);
    if (!optMed.has_value()) {
        QMessageBox::critical(this, "Lỗi", "Không tìm thấy thông tin thuốc trong cơ sở dữ liệu.");
        reject();
        return;
    }

    auto med = optMed.value();
    m_txtBrandName->setText(med.brandName);
    m_cbUnit->setCurrentText(med.unit);
    m_spinUnitPrice->setValue(med.unitPrice);
    m_spinStockQuantity->setValue(med.stockQuantity);
    m_spinMinimumStock->setValue(med.minimumStock);
    m_spinReorderThreshold->setValue(med.reorderThreshold);
    m_dateExpiry->setDate(med.expiryDate);
    m_txtManufacturer->setText(med.manufacturer);
    m_txtDescription->setHtml(med.description);

    // Lọc danh mục thuốc
    for (auto* chk : m_categoryCheckBoxes) {
        chk->setChecked(med.categories.contains(chk->text()));
    }

    // Load hoạt chất
    m_ingredientsTable->setRowCount(0);
    int row = 0;
    for (const auto& ing : med.ingredients) {
        m_ingredientsTable->insertRow(row);
        
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(ing.ingredientId));
        QTableWidgetItem* nameItem = new QTableWidgetItem(ing.ingredientName);
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        
        QLineEdit* strengthInput = new QLineEdit(this);
        strengthInput->setText(ing.strength);
        
        m_ingredientsTable->setItem(row, 0, idItem);
        m_ingredientsTable->setItem(row, 1, nameItem);
        m_ingredientsTable->setCellWidget(row, 2, strengthInput);
        
        row++;
    }
}

void AddMedicationDialog::openIngredientSearch() {
    ActiveIngredientSearchDialog searchDlg(m_pharmacyService, this);
    connect(&searchDlg, &ActiveIngredientSearchDialog::ingredientSelected, this, &AddMedicationDialog::addIngredientToTable);
    searchDlg.exec();
}

void AddMedicationDialog::addIngredientToTable(int id, const QString& name) {
    // Tránh trùng lặp hoạt chất trong bảng UI
    for (int r = 0; r < m_ingredientsTable->rowCount(); ++r) {
        if (m_ingredientsTable->item(r, 0)->text().toInt() == id) {
            QMessageBox::warning(this, "Cảnh báo", QString("Hoạt chất \"%1\" đã có trong bảng.").arg(name));
            return;
        }
    }

    int row = m_ingredientsTable->rowCount();
    m_ingredientsTable->insertRow(row);
    
    QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(id));
    QTableWidgetItem* nameItem = new QTableWidgetItem(name);
    idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    
    QLineEdit* strengthInput = new QLineEdit(this);
    strengthInput->setPlaceholderText("Ví dụ: 500mg, 10ml, v.v.");

    m_ingredientsTable->setItem(row, 0, idItem);
    m_ingredientsTable->setItem(row, 1, nameItem);
    m_ingredientsTable->setCellWidget(row, 2, strengthInput);
}

void AddMedicationDialog::removeSelectedIngredient() {
    int curRow = m_ingredientsTable->currentRow();
    if (curRow >= 0) {
        m_ingredientsTable->removeRow(curRow);
    } else {
        QMessageBox::information(this, "Thông báo", "Vui lòng chọn hoạt chất trong bảng để xóa.");
    }
}

void AddMedicationDialog::handleSave() {
    // 1. Thu thập dữ liệu sang DTO
    MedicationInputDTO input;
    input.brandName = m_txtBrandName->text().trimmed();
    input.unit = m_cbUnit->currentText().trimmed();
    input.unitPrice = m_spinUnitPrice->value();
    input.stockQuantity = m_spinStockQuantity->value();
    input.minimumStock = m_spinMinimumStock->value();
    input.reorderThreshold = m_spinReorderThreshold->value();
    input.expiryDate = m_dateExpiry->date();
    input.manufacturer = m_txtManufacturer->text().trimmed();
    input.description = m_txtDescription->toPlainText().trimmed();

    // Thu thập categories
    for (const auto* chk : m_categoryCheckBoxes) {
        if (chk->isChecked()) {
            input.categories.append(chk->text());
        }
    }

    // Thu thập hoạt chất
    for (int r = 0; r < m_ingredientsTable->rowCount(); ++r) {
        MedicationInputDTO::IngredientInput ing;
        ing.ingredientId = m_ingredientsTable->item(r, 0)->text().toInt();
        
        QLineEdit* strengthInput = qobject_cast<QLineEdit*>(m_ingredientsTable->cellWidget(r, 2));
        if (strengthInput) {
            ing.strength = strengthInput->text().trimmed();
        }
        input.ingredients.append(ing);
    }

    // 2. Validate dữ liệu bằng các Validator tĩnh ở tầng Service
    QString err;
    if (input.brandName.isEmpty()) {
        QMessageBox::warning(this, "Lỗi kiểm tra", "Tên thương hiệu thuốc không được bỏ trống.");
        return;
    }
    if (!(err = PharmacyService::validateUnitPrice(input.unitPrice)).isEmpty() ||
        !(err = PharmacyService::validateStockQuantity(input.stockQuantity)).isEmpty() ||
        !(err = PharmacyService::validateMinimumStock(input.minimumStock)).isEmpty() ||
        !(err = PharmacyService::validateReorderThreshold(input.reorderThreshold, input.minimumStock)).isEmpty() ||
        !(err = PharmacyService::validateExpiryDate(input.expiryDate)).isEmpty() ||
        !(err = PharmacyService::validateCategories(input.categories)).isEmpty() ||
        !(err = PharmacyService::validateIngredients(input.ingredients)).isEmpty()) {
        
        QMessageBox::warning(this, "Lỗi kiểm tra", err);
        return;
    }

    // 3. Thực hiện lưu thông qua Service
    QString resultMessage;
    if (m_medicationId == -1) {
        resultMessage = m_pharmacyService->addMedication(input);
    } else {
        resultMessage = m_pharmacyService->updateMedication(m_medicationId, input);
    }

    if (resultMessage.isEmpty()) {
        QMessageBox::information(this, "Thành công", m_medicationId == -1 ? "Thêm thuốc vào kho thành công!" : "Cập nhật thông tin thuốc thành công!");
        accept();
    } else {
        QMessageBox::critical(this, "Lỗi hệ thống", resultMessage);
    }
}