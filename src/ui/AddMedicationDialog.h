#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QCheckBox>

struct ActiveIngredientDTO {
    int ingredientId;
    QString ingredientName;
    QString description;
};

class ActiveIngredientSearchDialog : public QDialog {
    Q_OBJECT
public:
    explicit ActiveIngredientSearchDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Tra cứu hoạt chất");
        resize(500, 400);
        setupUI();
    }

private slots:
    void performSearch() {
        m_resultsTable->setRowCount(0);
        QString keyword = m_searchBar->text().trimmed().toLower();
        if (keyword.isEmpty()) return;

        QList<ActiveIngredientDTO> mockDatabase = {
            {1, "Paracetamol", "Giảm đau, hạ sốt"},
            {2, "Ibuprofen", "Kháng viêm không steroid"},
            {3, "Amoxicillin", "Kháng sinh nhóm penicillin"},
            {4, "Cetirizine", "Kháng histamine điều trị dị ứng"},
            {5, "Metformin", "Điều trị đái tháo đường tuýp 2"}
        };

        int row = 0;
        for (const auto& item : mockDatabase) {
            if (item.ingredientName.toLower().contains(keyword) || item.description.toLower().contains(keyword)) {
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
        }
    }

    void handleRowClicked(int row) {
        if (row < 0) return;

        QString ingredientName = m_resultsTable->item(row, 1)->text();

        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            "Xác nhận", 
            QString("Bạn có muốn thêm hoạt chất \"%1\" vào danh sách không?").arg(ingredientName),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            accept();
        }
    }

private:
    void setupUI() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(12);

        QHBoxLayout* searchLayout = new QHBoxLayout();
        m_searchBar = new QLineEdit(this);
        m_searchBar->setPlaceholderText("Nhập tên hoạt chất và ấn Enter để tìm kiếm...");
        m_searchBar->setStyleSheet(
            "QLineEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 12px; font-size: 13px; }"
            "QLineEdit:focus { border: 1px solid #0284C7; }"
        );
        searchLayout->addWidget(m_searchBar);
        mainLayout->addLayout(searchLayout);

        m_resultsTable = new QTableWidget(this);
        m_resultsTable->setColumnCount(3);
        m_resultsTable->setHorizontalHeaderLabels({"ID", "Tên hoạt chất", "Mô tả"});
        m_resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_resultsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_resultsTable->verticalHeader()->setVisible(false);
        m_resultsTable->setStyleSheet(
            "QTableWidget { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 6px; font-size: 13px; }"
            "QHeaderView::section { background-color: #F8FAFC; color: #475569; font-weight: bold; border: none; border-bottom: 2px solid #E2E8F0; padding: 6px; }"
        );
        mainLayout->addWidget(m_resultsTable);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addStretch();
        m_btnClose = new QPushButton("Đóng", this);
        m_btnClose->setStyleSheet(
            "QPushButton { background-color: #F1F5F9; color: #334155; border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: #E2E8F0; }"
        );
        buttonLayout->addWidget(m_btnClose);
        mainLayout->addLayout(buttonLayout);

        connect(m_searchBar, &QLineEdit::returnPressed, this, &ActiveIngredientSearchDialog::performSearch);
        connect(m_resultsTable, &QTableWidget::cellClicked, this, &ActiveIngredientSearchDialog::handleRowClicked);
        connect(m_btnClose, &QPushButton::clicked, this, &QDialog::reject);
    }

    QLineEdit* m_searchBar;
    QTableWidget* m_resultsTable;
    QPushButton* m_btnClose;
};

class AddMedicationDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddMedicationDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Thêm thuốc mới vào kho");
        resize(850, 700);
        setupUI();
    }

private slots:
    void openIngredientSearch() {
        ActiveIngredientSearchDialog dialog(this);
        dialog.exec();
    }

    void handleSave() {
        accept();
    }

private:
    void setupUI() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(16);

        QLabel* titleLabel = new QLabel("THÊM THUỐC MỚI VÀO HỆ THỐNG", this);
        titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #0F172A;");
        mainLayout->addWidget(titleLabel);

        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        
        QWidget* scrollContent = new QWidget();
        QVBoxLayout* formLayout = new QVBoxLayout(scrollContent);
        formLayout->setContentsMargins(0, 0, 8, 0);
        formLayout->setSpacing(16);

        QGroupBox* basicInfoGroup = new QGroupBox("Thông tin cơ bản", scrollContent);
        basicInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #0284C7; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding-top: 16px; } QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
        QGridLayout* gridLayout = new QGridLayout(basicInfoGroup);
        gridLayout->setContentsMargins(16, 16, 16, 16);
        gridLayout->setHorizontalSpacing(20);
        gridLayout->setVerticalSpacing(16);

        QString labelStyle = "QLabel { font-size: 13px; font-weight: bold; color: #334155; }";
        QString inputStyle = "QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px; font-size: 13px; color: #0F172A; background-color: #FFFFFF; }"
                             "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus { border: 1px solid #0284C7; }";

        QLabel* lblBrandName = new QLabel("Tên thuốc *", basicInfoGroup);
        lblBrandName->setStyleSheet(labelStyle);
        m_txtBrandName = new QLineEdit(basicInfoGroup);
        m_txtBrandName->setPlaceholderText("Nhập tên thương mại của thuốc");
        m_txtBrandName->setStyleSheet(inputStyle);
        gridLayout->addWidget(lblBrandName, 0, 0);
        gridLayout->addWidget(m_txtBrandName, 0, 1);

        QLabel* lblUnit = new QLabel("Đơn vị *", basicInfoGroup);
        lblUnit->setStyleSheet(labelStyle);
        m_cbUnit = new QComboBox(basicInfoGroup);
        m_cbUnit->addItems({"Viên", "Vỉ", "Hộp", "Chai", "Ống", "Gói"});
        m_cbUnit->setStyleSheet(inputStyle);
        gridLayout->addWidget(lblUnit, 0, 2);
        gridLayout->addWidget(m_cbUnit, 0, 3);

        QLabel* lblUnitPrice = new QLabel("Giá / Đơn vị (VND) *", basicInfoGroup);
        lblUnitPrice->setStyleSheet(labelStyle);
        m_spinUnitPrice = new QDoubleSpinBox(basicInfoGroup);
        m_spinUnitPrice->setRange(0, 999999999);
        m_spinUnitPrice->setSingleStep(1000);
        m_spinUnitPrice->setValue(0);
        m_spinUnitPrice->setStyleSheet(inputStyle);
        gridLayout->addWidget(lblUnitPrice, 1, 0);
        gridLayout->addWidget(m_spinUnitPrice, 1, 1);

        QLabel* lblStockQuantity = new QLabel("Số lượng tồn *", basicInfoGroup);
        lblStockQuantity->setStyleSheet(labelStyle);
        m_spinStockQuantity = new QSpinBox(basicInfoGroup);
        m_spinStockQuantity->setRange(0, 999999);
        m_spinStockQuantity->setStyleSheet(inputStyle);
        gridLayout->addWidget(lblStockQuantity, 1, 2);
        gridLayout->addWidget(m_spinStockQuantity, 1, 3);

        QLabel* lblMinStock = new QLabel("Ngưỡng tối thiểu *", basicInfoGroup);
        lblMinStock->setStyleSheet(labelStyle);
        m_spinMinimumStock = new QSpinBox(basicInfoGroup);
        m_spinMinimumStock->setRange(0, 9999);
        m_spinMinimumStock->setValue(10);
        m_spinMinimumStock->setStyleSheet(inputStyle);
        gridLayout->addWidget(lblMinStock, 2, 0);
        gridLayout->addWidget(m_spinMinimumStock, 2, 1);

        QLabel* lblReorder = new QLabel("Ngưỡng đặt hàng *", basicInfoGroup);
        lblReorder->setStyleSheet(labelStyle);
        m_spinReorderThreshold = new QSpinBox(basicInfoGroup);
        m_spinReorderThreshold->setRange(0, 9999);
        m_spinReorderThreshold->setValue(20);
        m_spinReorderThreshold->setStyleSheet(inputStyle);
        gridLayout->addWidget(lblReorder, 2, 2);
        gridLayout->addWidget(m_spinReorderThreshold, 2, 3);

        QLabel* lblExpiry = new QLabel("Hạn sử dụng *", basicInfoGroup);
        lblExpiry->setStyleSheet(labelStyle);
        m_dateExpiry = new QDateEdit(QDate::currentDate().addYears(2), basicInfoGroup);
        m_dateExpiry->setCalendarPopup(true);
        m_dateExpiry->setStyleSheet(inputStyle);
        gridLayout->addWidget(lblExpiry, 3, 0);
        gridLayout->addWidget(m_dateExpiry, 3, 1);

        QLabel* lblManufacturer = new QLabel("Nhà sản xuất", basicInfoGroup);
        lblManufacturer->setStyleSheet(labelStyle);
        m_txtManufacturer = new QLineEdit(basicInfoGroup);
        m_txtManufacturer->setPlaceholderText("Tên nhà máy / hãng sản xuất");
        m_txtManufacturer->setStyleSheet(inputStyle);
        gridLayout->addWidget(lblManufacturer, 3, 2);
        gridLayout->addWidget(m_txtManufacturer, 3, 3);

        formLayout->addWidget(basicInfoGroup);

        QGroupBox* categoryGroup = new QGroupBox("Danh mục thuốc", scrollContent);
        categoryGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #0284C7; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding-top: 16px; } QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
        m_categoriesLayout = new QHBoxLayout(categoryGroup);
        m_categoriesLayout->setContentsMargins(16, 16, 16, 16);
        m_categoriesLayout->setSpacing(12);

        QStringList mockCategories = {"Giảm đau", "Kháng sinh", "Kháng viêm", "Hạ áp", "Tim mạch", "Tiêu hóa"};
        for (const QString& cat : mockCategories) {
            QCheckBox* cb = new QCheckBox(cat, categoryGroup);
            cb->setStyleSheet("QCheckBox { font-size: 13px; color: #334155; }");
            m_categoriesLayout->addWidget(cb);
        }
        m_categoriesLayout->addStretch();
        formLayout->addWidget(categoryGroup);

        QGroupBox* ingredientsGroup = new QGroupBox("Hoạt chất thành phần", scrollContent);
        ingredientsGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #0284C7; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding-top: 16px; } QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
        QVBoxLayout* ingLayout = new QVBoxLayout(ingredientsGroup);
        ingLayout->setContentsMargins(16, 16, 16, 16);
        ingLayout->setSpacing(12);

        QPushButton* btnAddIngredient = new QPushButton("+ Thêm hoạt chất", ingredientsGroup);
        btnAddIngredient->setStyleSheet(
            "QPushButton { background-color: #0284C7; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: #0369A1; }"
        );
        ingLayout->addWidget(btnAddIngredient, 0, Qt::AlignLeft);

        m_ingredientsTable = new QTableWidget(ingredientsGroup);
        m_ingredientsTable->setColumnCount(4);
        m_ingredientsTable->setHorizontalHeaderLabels({"ID", "Tên hoạt chất", "Hàm lượng (Strength)", "Thao tác"});
        m_ingredientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_ingredientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        m_ingredientsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        m_ingredientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_ingredientsTable->verticalHeader()->setVisible(false);
        m_ingredientsTable->setMinimumHeight(150);
        m_ingredientsTable->setStyleSheet(
            "QTableWidget { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 6px; font-size: 13px; }"
            "QHeaderView::section { background-color: #F8FAFC; color: #475569; font-weight: bold; border: none; border-bottom: 2px solid #E2E8F0; padding: 6px; }"
        );
        ingLayout->addWidget(m_ingredientsTable);
        formLayout->addWidget(ingredientsGroup);

        QGroupBox* descGroup = new QGroupBox("Mô tả / Ghi chú", scrollContent);
        descGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #0284C7; border: 1px solid #E2E8F0; border-radius: 8px; margin-top: 12px; padding-top: 16px; } QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }");
        QVBoxLayout* descLayout = new QVBoxLayout(descGroup);
        descLayout->setContentsMargins(16, 16, 16, 16);
        m_txtDescription = new QTextEdit(descGroup);
        m_txtDescription->setPlaceholderText("Nhập mô tả chi tiết của thuốc hoặc hướng dẫn bảo quản...");
        m_txtDescription->setMinimumHeight(80);
        m_txtDescription->setStyleSheet("QTextEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px; font-size: 13px; }");
        descLayout->addWidget(m_txtDescription);
        formLayout->addWidget(descGroup);

        scrollArea->setWidget(scrollContent);
        mainLayout->addWidget(scrollArea);

        QHBoxLayout* actionLayout = new QHBoxLayout();
        actionLayout->setSpacing(12);
        actionLayout->addStretch();

        QPushButton* btnCancel = new QPushButton("Hủy bỏ", this);
        btnCancel->setStyleSheet(
            "QPushButton { background-color: #FFFFFF; color: #475569; border: 1px solid #CBD5E1; border-radius: 6px; padding: 10px 20px; font-weight: bold; }"
            "QPushButton:hover { background-color: #F8FAFC; }"
        );
        
        QPushButton* btnSave = new QPushButton("Lưu lại", this);
        btnSave->setStyleSheet(
            "QPushButton { background-color: #0284C7; color: white; border: none; border-radius: 6px; padding: 10px 24px; font-weight: bold; }"
            "QPushButton:hover { background-color: #0369A1; }"
        );

        actionLayout->addWidget(btnCancel);
        actionLayout->addWidget(btnSave);
        mainLayout->addLayout(actionLayout);

        connect(btnAddIngredient, &QPushButton::clicked, this, &AddMedicationDialog::openIngredientSearch);
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(btnSave, &QPushButton::clicked, this, &AddMedicationDialog::handleSave);
    }

    QLineEdit* m_txtBrandName;
    QHBoxLayout* m_categoriesLayout;
    QComboBox* m_cbUnit;
    QDoubleSpinBox* m_spinUnitPrice;
    QSpinBox* m_spinStockQuantity;
    QSpinBox* m_spinMinimumStock;
    QSpinBox* m_spinReorderThreshold;
    QDateEdit* m_dateExpiry;
    QLineEdit* m_txtManufacturer;
    QTextEdit* m_txtDescription;
    QTableWidget* m_ingredientsTable;
};