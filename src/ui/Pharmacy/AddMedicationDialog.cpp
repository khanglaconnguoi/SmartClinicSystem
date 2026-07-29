// #include "AddMedicationDialog.h"
// #include "service/PharmacyService.h"
// #include "dto/MedicationDTOs.h"
// #include "../../model/CommonEnums.h"

// // ============================================================================
// // IMPLEMENTATION: ACTIVE INGREDIENT SEARCH DIALOG
// // ============================================================================
// ActiveIngredientSearchDialog::ActiveIngredientSearchDialog(std::shared_ptr<PharmacyService> pharmacyService, QWidget* parent)
//     : QDialog(parent), m_pharmacyService(pharmacyService), m_currentPage(1), m_pageSize(5), m_totalPages(1) {
//     setWindowTitle("Tra cứu hoạt chất");
//     resize(550, 450);
//     setupUI();
//     updatePaginationButtons();
// }

// void ActiveIngredientSearchDialog::performSearch() {
//     m_currentPage = 1; 
//     loadPageData();
// }

// void ActiveIngredientSearchDialog::goToPreviousPage() {
//     if (m_currentPage > 1) {
//         m_currentPage--;
//         loadPageData();
//     }
// }

// void ActiveIngredientSearchDialog::goToNextPage() {
//     if (m_currentPage < m_totalPages) {
//         m_currentPage++;
//         loadPageData();
//     }
// }

// void ActiveIngredientSearchDialog::handleRowClicked(int row) {
//     if (row < 0) return;

//     int ingredientId = m_resultsTable->item(row, 0)->text().toInt();
//     QString ingredientName = m_resultsTable->item(row, 1)->text();

//     QMessageBox::StandardButton reply = QMessageBox::question(
//         this, 
//         "Xác nhận", 
//         QString("Bạn có muốn thêm hoạt chất \"%1\" vào danh sách không?").arg(ingredientName),
//         QMessageBox::Yes | QMessageBox::No
//     );

//     if (reply == QMessageBox::Yes) {
//         emit ingredientSelected(ingredientId, ingredientName);
//         accept();
//     }
// }

// void ActiveIngredientSearchDialog::loadPageData() {
//     m_resultsTable->setRowCount(0);
//     QString keyword = m_searchBar->text().simplified();
//     if (keyword.isEmpty()) {
//         m_lblPageInfo->setText("Trang 1 / 1");
//         m_totalPages = 1;
//         updatePaginationButtons();
//         return;
//     }

//     // Gọi hàm tìm kiếm phân trang hoạt chất thuốc trong Medication Service
//     auto result = m_pharmacyService->searchIngredientsPaged(keyword, m_currentPage, m_pageSize);
    
//     m_totalPages = result.totalPages;
//     m_currentPage = result.currentPage;

//     int row = 0;
//     for (const auto& item : result.items) {
//         m_resultsTable->insertRow(row);
        
//         QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(item.ingredientId));
//         QTableWidgetItem* nameItem = new QTableWidgetItem(item.ingredientName);
//         QTableWidgetItem* descItem = new QTableWidgetItem(item.description);

//         idItem->setTextAlignment(Qt::AlignCenter);

//         m_resultsTable->setItem(row, 0, idItem);
//         m_resultsTable->setItem(row, 1, nameItem);
//         m_resultsTable->setItem(row, 2, descItem);
//         row++;
//     }

//     m_lblPageInfo->setText(QString("Trang %1 / %2").arg(m_currentPage).arg(m_totalPages == 0 ? 1 : m_totalPages));
//     updatePaginationButtons();
// }

// void ActiveIngredientSearchDialog::updatePaginationButtons() {
//     m_btnPrev->setEnabled(m_currentPage > 1);
//     m_btnNext->setEnabled(m_currentPage < m_totalPages);
// }

// void ActiveIngredientSearchDialog::setupUI() {
//     QVBoxLayout* mainLayout = new QVBoxLayout(this);
//     mainLayout->setContentsMargins(16, 16, 16, 16);
//     mainLayout->setSpacing(12);

//     QHBoxLayout* searchLayout = new QHBoxLayout();
//     m_searchBar = new QLineEdit(this);
//     m_searchBar->setPlaceholderText("Nhập tên hoạt chất và ấn Enter để tìm kiếm...");
//     m_searchBar->setStyleSheet(
//         "QLineEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 12px; font-size: 13px; }"
//         "QLineEdit:focus { border: 1px solid #0284C7; }"
//     );
//     searchLayout->addWidget(m_searchBar);
//     mainLayout->addLayout(searchLayout);

//     m_resultsTable = new QTableWidget(this);
//     m_resultsTable->setColumnCount(3);
//     m_resultsTable->setHorizontalHeaderLabels({"ID", "Tên hoạt chất", "Mô tả"});
//     m_resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//     m_resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//     m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
//     m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
//     m_resultsTable->verticalHeader()->setVisible(false);
//     m_resultsTable->setStyleSheet(
//         "QTableWidget { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 6px; font-size: 13px; }"
//         "QHeaderView::section { background-color: #F8FAFC; color: #475569; font-weight: bold; border: none; border-bottom: 2px solid #E2E8F0; padding: 6px; }"
//     );
//     mainLayout->addWidget(m_resultsTable);

//     QHBoxLayout* paginationLayout = new QHBoxLayout();
//     m_btnPrev = new QPushButton("Trước", this);
//     m_btnPrev->setStyleSheet("QPushButton { border: 1px solid #CBD5E1; border-radius: 4px; padding: 4px 12px; font-size: 12px; background-color: #FFFFFF; }"
//                              "QPushButton:disabled { color: #94A3B8; background-color: #F1F5F9; }");
    
//     m_lblPageInfo = new QLabel("Trang 1 / 1", this);
//     m_lblPageInfo->setAlignment(Qt::AlignCenter);
//     m_lblPageInfo->setStyleSheet("font-size: 13px; font-weight: bold; color: #475569;");
    
//     m_btnNext = new QPushButton("Sau", this);
//     m_btnNext->setStyleSheet("QPushButton { border: 1px solid #CBD5E1; border-radius: 4px; padding: 4px 12px; font-size: 12px; background-color: #FFFFFF; }"
//                              "QPushButton:disabled { color: #94A3B8; background-color: #F1F5F9; }");
    
//     paginationLayout->addWidget(m_btnPrev);
//     paginationLayout->addWidget(m_lblPageInfo);
//     paginationLayout->addWidget(m_btnNext);
//     mainLayout->addLayout(paginationLayout);

//     QHBoxLayout* buttonLayout = new QHBoxLayout();
//     buttonLayout->addStretch();
//     m_btnClose = new QPushButton("Đóng", this);
//     m_btnClose->setStyleSheet(
//         "QPushButton { background-color: #F1F5F9; color: #334155; border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
//         "QPushButton:hover { background-color: #E2E8F0; }"
//     );
//     buttonLayout->addWidget(m_btnClose);
//     mainLayout->addLayout(buttonLayout);

//     connect(m_searchBar, &QLineEdit::returnPressed, this, &ActiveIngredientSearchDialog::performSearch);
//     connect(m_resultsTable, &QTableWidget::cellClicked, this, &ActiveIngredientSearchDialog::handleRowClicked);
//     connect(m_btnPrev, &QPushButton::clicked, this, &ActiveIngredientSearchDialog::goToPreviousPage);
//     connect(m_btnNext, &QPushButton::clicked, this, &ActiveIngredientSearchDialog::goToNextPage);
//     connect(m_btnClose, &QPushButton::clicked, this, &QDialog::reject);
// }


// // ============================================================================
// // IMPLEMENTATION: ADD MEDICATION DIALOG
// // ============================================================================
// AddMedicationDialog::AddMedicationDialog(std::shared_ptr<PharmacyService> pharmacyService, QWidget* parent)
//     : QDialog(parent), m_pharmacyService(pharmacyService) {
//     setWindowTitle("Thêm thuốc mới vào kho");
//     resize(850, 700);
//     setupUI();
//     loadCommonEnums();
// }

// void AddMedicationDialog::openIngredientSearch() {
//     ActiveIngredientSearchDialog dialog(m_pharmacyService, this);
//     connect(&dialog, &ActiveIngredientSearchDialog::ingredientSelected, this, &AddMedicationDialog::addIngredientToTable);
//     dialog.exec();
// }

// void AddMedicationDialog::addIngredientToTable(int id, const QString& name) {
//     for (int i = 0; i < m_ingredientsTable->rowCount(); ++i) {
//         if (m_ingredientsTable->item(i, 0)->text().toInt() == id) {
//             QMessageBox::warning(this, "Cảnh báo", "Hoạt chất này đã được thêm vào danh sách.");
//             return;
//         }
//     }

//     int row = m_ingredientsTable->rowCount();
//     m_ingredientsTable->insertRow(row);

//     QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(id));
//     QTableWidgetItem* nameItem = new QTableWidgetItem(name);
//     idItem->setTextAlignment(Qt::AlignCenter);

//     QLineEdit* strengthInput = new QLineEdit(this);
//     strengthInput->setPlaceholderText("VD: 500mg");
//     strengthInput->setStyleSheet("border: 1px solid #CBD5E1; border-radius: 4px; padding: 4px; font-size: 13px;");

//     QPushButton* btnDelete = new QPushButton("Xóa", this);
//     btnDelete->setStyleSheet("QPushButton { background-color: #EF4444; color: white; border: none; border-radius: 4px; padding: 4px 8px; font-weight: bold; }"
//                              "QPushButton:hover { background-color: #DC2626; }");
    
//     connect(btnDelete, &QPushButton::clicked, this, [this, id]() {
//         for (int i = 0; i < m_ingredientsTable->rowCount(); ++i) {
//             if (m_ingredientsTable->item(i, 0)->text().toInt() == id) {
//                 m_ingredientsTable->removeRow(i);
//                 break;
//             }
//         }
//     });

//     m_ingredientsTable->setItem(row, 0, idItem);
//     m_ingredientsTable->setItem(row, 1, nameItem);
//     m_ingredientsTable->setCellWidget(row, 2, strengthInput);
//     m_ingredientsTable->setCellWidget(row, 3, btnDelete);
// }

// void AddMedicationDialog::handleSave() {
//     MedicationInputDTO dto;
    
//     // Thu thập dữ liệu cơ bản từ UI
//     dto.brandName = m_txtBrandName->text();
//     dto.unitPrice = m_spinUnitPrice->value();
//     dto.stockQuantity = m_spinStockQuantity->value();
//     dto.minimumStock = m_spinMinimumStock->value();
//     dto.reorderThreshold = m_spinReorderThreshold->value();
//     dto.expiryDate = m_dateExpiry->date();
//     dto.manufacturer = m_txtManufacturer->text();
//     dto.description = m_txtDescription->toPlainText();

//     // 1. Xử lý Đơn vị: Chuyển đổi từ chuỗi Tiếng Việt hiển thị sang chuỗi DTO tiếng Anh tương ứng
//     dto.unit = MedicationUnitText::toEn(m_cbUnit->currentText());

//     // 2. Xử lý Danh mục: Duyệt qua các CheckBox được tích, đổi từ chuỗi Tiếng Việt sang định dạng chuỗi Tiếng Anh
//     for (QCheckBox* cb : m_categoryCheckBoxes) {
//         if (cb->isChecked()) {
//             dto.categories.append(MedicationCategoryText::toEn(cb->text()));
//         }
//     }

//     // 3. Thu thập thông tin danh sách Hoạt chất từ bảng
//     for (int i = 0; i < m_ingredientsTable->rowCount(); ++i) {
//         MedicationInputDTO::IngredientInput ing;
//         ing.ingredientId = m_ingredientsTable->item(i, 0)->text().toInt();
        
//         QLineEdit* strengthWidget = qobject_cast<QLineEdit*>(m_ingredientsTable->cellWidget(i, 2));
//         if (strengthWidget) {
//             ing.strength = strengthWidget->text();
//         }
//         dto.ingredients.append(ing);
//     }

//     // Gọi tầng Service để xác thực nghiệp vụ và thêm thuốc mới
//     QString errorMsg = m_pharmacyService->addMedication(dto);
//     if (!errorMsg.isEmpty()) {
//         QMessageBox::critical(this, "Lỗi kiểm tra dữ liệu", errorMsg);
//         return;
//     }

//     QMessageBox::information(this, "Thành công", "Đã thêm thuốc mới vào hệ thống.");
//     accept();
// }

// void AddMedicationDialog::loadCommonEnums() {
//     // 1. Lấy toàn bộ danh sách Đơn vị tiếng Việt từ MedicationUnitText
//     m_cbUnit->clear();
//     for (const auto& pair : MedicationUnitText::getList()) {
//         m_cbUnit->addItem(pair.second);
//     }

//     // 2. Lấy toàn bộ danh mục thuốc tiếng Việt từ MedicationCategoryText
//     QLayoutItem* item;
//     while ((item = m_categoriesLayout->takeAt(0)) != nullptr) {
//         if (item->widget()) delete item->widget();
//         delete item;
//     }
//     m_categoryCheckBoxes.clear();

//     // Đổ động các danh mục tiếng Việt lên giao diện dưới dạng CheckBox
//     for (const auto& pair : MedicationCategoryText::getList()) {
//         QCheckBox* cb = new QCheckBox(pair.second, this);
//         cb->setStyleSheet("QCheckBox { font-size: 13px; color: #334155; }");
//         m_categoriesLayout->addWidget(cb);
//         m_categoryCheckBoxes.append(cb);
//     }
//     m_categoriesLayout->addStretch();
// }

// void AddMedicationDialog::setupUI() {
//     QVBoxLayout* mainLayout = new QVBoxLayout(this);
//     mainLayout->setContentsMargins(20, 20, 20, 20);
//     mainLayout->setSpacing(16);

//     QLabel* titleLabel = new QLabel("THÊM THUỐC MỚI VÀO HỆ THỐNG", this);
//     titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #0F172A;");
//     mainLayout->addWidget(titleLabel);

//     QScrollArea* scrollArea = new QScrollArea(this);
//     scrollArea->setWidgetResizable(true);
//     scrollArea->setFrameShape(QFrame::NoFrame);
    
//     QWidget* scrollContent = new QWidget();
//     QVBoxLayout* formLayout = new QVBoxLayout(scrollContent);
//     formLayout->setContentsMargins(0, 0, 8, 0);
//     formLayout->setSpacing(16);

//     QGroupBox* basicInfoGroup = new QGroupBox("Thông tin cơ bản", scrollContent);
//     basicInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #0284C7; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding-top: 16px; } QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
//     QGridLayout* gridLayout = new QGridLayout(basicInfoGroup);
//     gridLayout->setContentsMargins(16, 16, 16, 16);
//     gridLayout->setHorizontalSpacing(20);
//     gridLayout->setVerticalSpacing(16);

//     QString labelStyle = "QLabel { font-size: 13px; font-weight: bold; color: #334155; }";
//     QString inputStyle = "QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px; font-size: 13px; color: #0F172A; background-color: #FFFFFF; }"
//                          "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus { border: 1px solid #0284C7; }";

//     QLabel* lblBrandName = new QLabel("Tên thuốc *", basicInfoGroup);
//     lblBrandName->setStyleSheet(labelStyle);
//     m_txtBrandName = new QLineEdit(basicInfoGroup);
//     m_txtBrandName->setPlaceholderText("Nhập tên thương mại của thuốc");
//     m_txtBrandName->setStyleSheet(inputStyle);
//     gridLayout->addWidget(lblBrandName, 0, 0);
//     gridLayout->addWidget(m_txtBrandName, 0, 1);

//     QLabel* lblUnit = new QLabel("Đơn vị *", basicInfoGroup);
//     lblUnit->setStyleSheet(labelStyle);
//     m_cbUnit = new QComboBox(basicInfoGroup);
//     m_cbUnit->setStyleSheet(inputStyle);
//     gridLayout->addWidget(lblUnit, 0, 2);
//     gridLayout->addWidget(m_cbUnit, 0, 3);

//     QLabel* lblUnitPrice = new QLabel("Giá / Đơn vị (VND) *", basicInfoGroup);
//     lblUnitPrice->setStyleSheet(labelStyle);
//     m_spinUnitPrice = new QDoubleSpinBox(basicInfoGroup);
//     m_spinUnitPrice->setRange(0, 999999999);
//     m_spinUnitPrice->setSingleStep(1000);
//     m_spinUnitPrice->setStyleSheet(inputStyle);
//     gridLayout->addWidget(lblUnitPrice, 1, 0);
//     gridLayout->addWidget(m_spinUnitPrice, 1, 1);

//     QLabel* lblStockQuantity = new QLabel("Số lượng tồn *", basicInfoGroup);
//     lblStockQuantity->setStyleSheet(labelStyle);
//     m_spinStockQuantity = new QSpinBox(basicInfoGroup);
//     m_spinStockQuantity->setRange(0, 999999);
//     m_spinStockQuantity->setStyleSheet(inputStyle);
//     gridLayout->addWidget(lblStockQuantity, 1, 2);
//     gridLayout->addWidget(m_spinStockQuantity, 1, 3);

//     QLabel* lblMinStock = new QLabel("Ngưỡng tối thiểu *", basicInfoGroup);
//     lblMinStock->setStyleSheet(labelStyle);
//     m_spinMinimumStock = new QSpinBox(basicInfoGroup);
//     m_spinMinimumStock->setRange(0, 9999);
//     m_spinMinimumStock->setValue(10);
//     m_spinMinimumStock->setStyleSheet(inputStyle);
//     gridLayout->addWidget(lblMinStock, 2, 0);
//     gridLayout->addWidget(m_spinMinimumStock, 2, 1);

//     QLabel* lblReorder = new QLabel("Ngưỡng đặt hàng *", basicInfoGroup);
//     lblReorder->setStyleSheet(labelStyle);
//     m_spinReorderThreshold = new QSpinBox(basicInfoGroup);
//     m_spinReorderThreshold->setRange(0, 9999);
//     m_spinReorderThreshold->setValue(20);
//     m_spinReorderThreshold->setStyleSheet(inputStyle);
//     gridLayout->addWidget(lblReorder, 2, 2);
//     gridLayout->addWidget(m_spinReorderThreshold, 2, 3);

//     QLabel* lblExpiry = new QLabel("Hạn sử dụng *", basicInfoGroup);
//     lblExpiry->setStyleSheet(labelStyle);
//     m_dateExpiry = new QDateEdit(QDate::currentDate().addYears(2), basicInfoGroup);
//     m_dateExpiry->setCalendarPopup(true);
//     m_dateExpiry->setStyleSheet(inputStyle);
//     gridLayout->addWidget(lblExpiry, 3, 0);
//     gridLayout->addWidget(m_dateExpiry, 3, 1);

//     QLabel* lblManufacturer = new QLabel("Nhà sản xuất", basicInfoGroup);
//     lblManufacturer->setStyleSheet(labelStyle);
//     m_txtManufacturer = new QLineEdit(basicInfoGroup);
//     m_txtManufacturer->setPlaceholderText("Tên hãng sản xuất");
//     m_txtManufacturer->setStyleSheet(inputStyle);
//     gridLayout->addWidget(lblManufacturer, 3, 2);
//     gridLayout->addWidget(m_txtManufacturer, 3, 3);

//     formLayout->addWidget(basicInfoGroup);

//     QGroupBox* categoryGroup = new QGroupBox("Danh mục thuốc", scrollContent);
//     categoryGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #0284C7; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding-top: 16px; } QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
//     m_categoriesLayout = new QHBoxLayout(categoryGroup);
//     m_categoriesLayout->setContentsMargins(16, 16, 16, 16);
//     m_categoriesLayout->setSpacing(12);
//     formLayout->addWidget(categoryGroup);

//     QGroupBox* ingredientsGroup = new QGroupBox("Hoạt chất thành phần", scrollContent);
//     ingredientsGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #0284C7; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding-top: 16px; } QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
//     QVBoxLayout* ingLayout = new QVBoxLayout(ingredientsGroup);
//     ingLayout->setContentsMargins(16, 16, 16, 16);
//     ingLayout->setSpacing(12);

//     QPushButton* btnAddIngredient = new QPushButton("+ Thêm hoạt chất", ingredientsGroup);
//     btnAddIngredient->setStyleSheet(
//         "QPushButton { background-color: #0284C7; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
//         "QPushButton:hover { background-color: #0369A1; }"
//     );
//     ingLayout->addWidget(btnAddIngredient, 0, Qt::AlignLeft);

//     m_ingredientsTable = new QTableWidget(ingredientsGroup);
//     m_ingredientsTable->setColumnCount(4);
//     m_ingredientsTable->setHorizontalHeaderLabels({"ID", "Tên hoạt chất", "Hàm lượng (Strength)", "Thao tác"});
//     m_ingredientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//     m_ingredientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
//     m_ingredientsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
//     m_ingredientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
//     m_ingredientsTable->verticalHeader()->setVisible(false);
//     m_ingredientsTable->setMinimumHeight(150);
//     m_ingredientsTable->setStyleSheet(
//         "QTableWidget { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 6px; font-size: 13px; }"
//         "QHeaderView::section { background-color: #F8FAFC; color: #475569; font-weight: bold; border: none; border-bottom: 2px solid #E2E8F0; padding: 6px; }"
//     );
//     ingLayout->addWidget(m_ingredientsTable);
//     formLayout->addWidget(ingredientsGroup);

//     QGroupBox* descGroup = new QGroupBox("Mô tả / Ghi chú", scrollContent);
//     descGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #0284C7; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding-top: 16px; } QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
//     QVBoxLayout* descLayout = new QVBoxLayout(descGroup);
//     descLayout->setContentsMargins(16, 16, 16, 16);
//     m_txtDescription = new QTextEdit(descGroup);
//     m_txtDescription->setPlaceholderText("Nhập mô tả chi tiết của thuốc hoặc hướng dẫn bảo quản...");
//     m_txtDescription->setMinimumHeight(80);
//     m_txtDescription->setStyleSheet("QTextEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px; font-size: 13px; }");
//     descLayout->addWidget(m_txtDescription);
//     formLayout->addWidget(descGroup);

//     scrollArea->setWidget(scrollContent);
//     mainLayout->addWidget(scrollArea);

//     QHBoxLayout* actionLayout = new QHBoxLayout();
//     actionLayout->setSpacing(12);
//     actionLayout->addStretch();

//     QPushButton* btnCancel = new QPushButton("Hủy bỏ", this);
//     btnCancel->setStyleSheet(
//         "QPushButton { background-color: #FFFFFF; color: #475569; border: 1px solid #CBD5E1; border-radius: 6px; padding: 10px 20px; font-weight: bold; }"
//         "QPushButton:hover { background-color: #F8FAFC; }"
//     );
    
//     QPushButton* btnSave = new QPushButton("Lưu lại", this);
//     btnSave->setStyleSheet(
//         "QPushButton { background-color: #0284C7; color: white; border: none; border-radius: 6px; padding: 10px 24px; font-weight: bold; }"
//         "QPushButton:hover { background-color: #0369A1; }"
//     );

//     actionLayout->addWidget(btnCancel);
//     actionLayout->addWidget(btnSave);
//     mainLayout->addLayout(actionLayout);

//     connect(btnAddIngredient, &QPushButton::clicked, this, &AddMedicationDialog::openIngredientSearch);
//     connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
//     connect(btnSave, &QPushButton::clicked, this, &AddMedicationDialog::handleSave);
// }