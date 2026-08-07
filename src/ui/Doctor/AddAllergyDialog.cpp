#include "AddAllergyDialog.h"

// ============================================================================
// ADD ACTIVE INGREDIENT ALLERGY DIALOG
// ============================================================================
AddActiveIngredientAllergyDialog::AddActiveIngredientAllergyDialog(std::shared_ptr<PharmacyService> pharmacyService, QWidget* parent)
    : QDialog(parent), m_pharmacyService(pharmacyService), m_currentPage(1), m_pageSize(7), m_totalPages(1) {
    setWindowTitle("Thêm dị ứng hoạt chất");
    resize(620, 500);
    setupUI();
    performSearch();
}

void AddActiveIngredientAllergyDialog::setupUI() {
    this->setStyleSheet(
        "QDialog { background-color: #FFFFFF; font-family: 'Segoe UI'; }"
        "QLabel { font-size: 13px; font-weight: bold; color: #334155; background-color: transparent; border: none; }"
        "QLineEdit, QComboBox { font-size: 13px; border: 1px solid #CBD5E1; border-radius: 6px; padding: 6px 10px; color: #0F172A; }"
        "QLineEdit:focus, QComboBox:focus { border: 1px solid #2563EB; }"
        "QTableWidget { border: 1px solid #E2E8F0; border-radius: 6px; gridline-color: #F1F5F9; selection-background-color: #EFF6FF; selection-color: #1E40AF; outline: none; }"
        "QTableWidget::item { outline: none; border: none; }"
        "QTableWidget::item:focus { outline: none; border: none; }"
        "QHeaderView::section { background-color: #F8FAFC; padding: 6px; font-weight: bold; border: none; border-bottom: 1px solid #E2E8F0; color: #475569; font-size: 12px; }"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(12);

    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText("Nhập tên hoạt chất để tìm kiếm tự động...");
    m_searchBar->setClearButtonEnabled(true);
    mainLayout->addWidget(m_searchBar);

    m_resultsTable = new QTableWidget(this);
    m_resultsTable->setColumnCount(3);
    m_resultsTable->setHorizontalHeaderLabels({"ID", "Tên Hoạt Chất", "Mô Tả / Ghi Chú"});
    m_resultsTable->verticalHeader()->setVisible(false);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultsTable->setFocusPolicy(Qt::NoFocus);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_resultsTable->setColumnWidth(1, 200);

    mainLayout->addWidget(m_resultsTable);

    QHBoxLayout* pageLayout = new QHBoxLayout();
    m_btnPrev = new QPushButton("< Trước", this);
    m_btnNext = new QPushButton("Sau >", this);
    m_lblPageInfo = new QLabel("Trang 1 / 1", this);
    m_lblPageInfo->setStyleSheet("font-weight: normal; color: #64748B; background-color: transparent; border: none;");

    QString btnPageStyle = "QPushButton { border: 1px solid #CBD5E1; background: white; border-radius: 4px; padding: 4px 12px; font-size: 12px; cursor: pointer; }"
                           "QPushButton:hover { background-color: #F1F5F9; }"
                           "QPushButton:disabled { color: #94A3B8; background: #F8FAFC; cursor: default; }";
    m_btnPrev->setStyleSheet(btnPageStyle);
    m_btnNext->setStyleSheet(btnPageStyle);

    pageLayout->addWidget(m_btnPrev);
    pageLayout->addWidget(m_lblPageInfo);
    pageLayout->addWidget(m_btnNext);
    pageLayout->addStretch();
    mainLayout->addLayout(pageLayout);

    QGridLayout* optLayout = new QGridLayout();
    optLayout->setSpacing(10);

    QLabel* lblSeverity = new QLabel("Mức độ dị ứng:", this);
    lblSeverity->setStyleSheet("background-color: transparent; border: none;");
    m_cbSeverity = new QComboBox(this);
    m_cbSeverity->addItem(severityToVi(Severity::Mild), static_cast<int>(Severity::Mild));
    m_cbSeverity->addItem(severityToVi(Severity::Moderate), static_cast<int>(Severity::Moderate));
    m_cbSeverity->addItem(severityToVi(Severity::Severe), static_cast<int>(Severity::Severe));

    QLabel* lblNotes = new QLabel("Ghi chú dị ứng:", this);
    lblNotes->setStyleSheet("background-color: transparent; border: none;");
    m_txtNotes = new QLineEdit(this);
    m_txtNotes->setPlaceholderText("Ghi chú phản ứng (nổi mẩn đỏ, sưng môi, phế quản, phát ban...)...");

    optLayout->addWidget(lblSeverity, 0, 0);
    optLayout->addWidget(m_cbSeverity, 0, 1);
    optLayout->addWidget(lblNotes, 1, 0);
    optLayout->addWidget(m_txtNotes, 1, 1);

    mainLayout->addLayout(optLayout);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->addStretch();

    QPushButton* btnCancel = new QPushButton("Hủy", this);
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet("QPushButton { border: 1px solid #CBD5E1; background: white; color: #475569; border-radius: 6px; padding: 8px 18px; font-weight: bold; }"
                             "QPushButton:hover { background-color: #F1F5F9; }");

    m_btnAdd = new QPushButton("Thêm Vào Danh Sách Dị Ứng", this);
    m_btnAdd->setCursor(Qt::PointingHandCursor);
    m_btnAdd->setStyleSheet("QPushButton { background-color: #059669; color: white; border-radius: 6px; font-size: 13px; font-weight: bold; padding: 8px 18px; }"
                            "QPushButton:hover { background-color: #047857; }");

    actionLayout->addWidget(btnCancel);
    actionLayout->addWidget(m_btnAdd);
    mainLayout->addLayout(actionLayout);

    connect(m_searchBar, &QLineEdit::textChanged, this, &AddActiveIngredientAllergyDialog::performSearch);
    connect(m_btnPrev, &QPushButton::clicked, this, &AddActiveIngredientAllergyDialog::goToPreviousPage);
    connect(m_btnNext, &QPushButton::clicked, this, &AddActiveIngredientAllergyDialog::goToNextPage);
    connect(m_btnAdd, &QPushButton::clicked, this, &AddActiveIngredientAllergyDialog::handleAddSelected);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_resultsTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        if (row >= 0) handleAddSelected();
    });
}

void AddActiveIngredientAllergyDialog::performSearch() {
    m_currentPage = 1;
    loadPageData();
}

void AddActiveIngredientAllergyDialog::goToPreviousPage() {
    if (m_currentPage > 1) {
        m_currentPage--;
        loadPageData();
    }
}

void AddActiveIngredientAllergyDialog::goToNextPage() {
    if (m_currentPage < m_totalPages) {
        m_currentPage++;
        loadPageData();
    }
}

void AddActiveIngredientAllergyDialog::loadPageData() {
    m_resultsTable->setRowCount(0);
    if (!m_pharmacyService) return;

    IngredientSearchCriteria criteria;
    criteria.keyword = m_searchBar->text().trimmed();
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

void AddActiveIngredientAllergyDialog::updatePaginationButtons() {
    m_btnPrev->setEnabled(m_currentPage > 1);
    m_btnNext->setEnabled(m_currentPage < m_totalPages);
}

void AddActiveIngredientAllergyDialog::handleAddSelected() {
    int row = m_resultsTable->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Chưa chọn hoạt chất", "Vui lòng chọn một hoạt chất từ danh sách kết quả.");
        return;
    }

    int ingredientId = m_resultsTable->item(row, 0)->text().toInt();
    QString ingredientName = m_resultsTable->item(row, 1)->text();

    m_selectedAllergy.ingredientId = ingredientId;
    m_selectedAllergy.allergenName = ingredientName;
    m_selectedAllergy.severity = static_cast<Severity>(m_cbSeverity->currentData().toInt());
    m_selectedAllergy.notes = m_txtNotes->text().trimmed();

    emit allergyAdded(m_selectedAllergy);
    accept();
}


// ============================================================================
// ADD OTHER ALLERGY DIALOG
// ============================================================================
AddOtherAllergyDialog::AddOtherAllergyDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Thêm dị ứng khác");
    resize(420, 240);

    this->setStyleSheet(
        "QDialog { background-color: #FFFFFF; font-family: 'Segoe UI'; }"
        "QLabel { font-size: 13px; font-weight: bold; color: #334155; background-color: transparent; border: none; }"
        "QLineEdit, QComboBox { font-size: 13px; border: 1px solid #CBD5E1; border-radius: 6px; padding: 6px 10px; color: #0F172A; }"
        "QLineEdit:focus, QComboBox:focus { border: 1px solid #2563EB; }"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    QGridLayout* formGrid = new QGridLayout();
    formGrid->setSpacing(12);

    QLabel* lblName = new QLabel("Tên dị ứng / Tác nhân:", this);
    lblName->setStyleSheet("background-color: transparent; border: none;");
    m_txtName = new QLineEdit(this);
    m_txtName->setPlaceholderText("Vd: Hải sản, Phấn hoa, Đậu phộng, Côn trùng...");

    QLabel* lblSeverity = new QLabel("Mức độ dị ứng:", this);
    lblSeverity->setStyleSheet("background-color: transparent; border: none;");
    m_cbSeverity = new QComboBox(this);
    m_cbSeverity->addItem(severityToVi(Severity::Mild), static_cast<int>(Severity::Mild));
    m_cbSeverity->addItem(severityToVi(Severity::Moderate), static_cast<int>(Severity::Moderate));
    m_cbSeverity->addItem(severityToVi(Severity::Severe), static_cast<int>(Severity::Severe));

    QLabel* lblNotes = new QLabel("Ghi chú thêm:", this);
    lblNotes->setStyleSheet("background-color: transparent; border: none;");
    m_txtNotes = new QLineEdit(this);
    m_txtNotes->setPlaceholderText("Mô tả phản ứng dị ứng...");

    formGrid->addWidget(lblName, 0, 0);
    formGrid->addWidget(m_txtName, 0, 1);
    formGrid->addWidget(lblSeverity, 1, 0);
    formGrid->addWidget(m_cbSeverity, 1, 1);
    formGrid->addWidget(lblNotes, 2, 0);
    formGrid->addWidget(m_txtNotes, 2, 1);

    mainLayout->addLayout(formGrid);
    mainLayout->addStretch();

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->addStretch();

    QPushButton* btnCancel = new QPushButton("Hủy", this);
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet("QPushButton { border: 1px solid #CBD5E1; background: white; color: #475569; border-radius: 6px; padding: 8px 18px; font-weight: bold; }"
                             "QPushButton:hover { background-color: #F1F5F9; }");

    QPushButton* btnConfirm = new QPushButton("Thêm Dị Ứng", this);
    btnConfirm->setCursor(Qt::PointingHandCursor);
    btnConfirm->setStyleSheet("QPushButton { background-color: #2563EB; color: white; border-radius: 6px; font-size: 13px; font-weight: bold; padding: 8px 18px; }"
                              "QPushButton:hover { background-color: #1D4ED8; }");

    actionLayout->addWidget(btnCancel);
    actionLayout->addWidget(btnConfirm);
    mainLayout->addLayout(actionLayout);

    connect(btnConfirm, &QPushButton::clicked, this, &AddOtherAllergyDialog::handleConfirm);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void AddOtherAllergyDialog::handleConfirm() {
    QString name = m_txtName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Thiếu thông tin", "Vui lòng nhập tên dị ứng / tác nhân gây dị ứng.");
        return;
    }

    m_selectedAllergy.ingredientId = std::nullopt;
    m_selectedAllergy.allergenName = name;
    m_selectedAllergy.severity = static_cast<Severity>(m_cbSeverity->currentData().toInt());
    m_selectedAllergy.notes = m_txtNotes->text().trimmed();

    emit allergyAdded(m_selectedAllergy);
    accept();
}