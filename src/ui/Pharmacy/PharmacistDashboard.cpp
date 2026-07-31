#include "PharmacistDashboard.h"
#include "AddMedicationDialog.h"
#include "service/PharmacyService.h"
#include "service/BillingService.h"
#include "dto/MedicationDTOs.h"
#include "dto/PrescriptionDTOs.h"
#include "dto/BillingDTOs.h"
#include "model/CommonEnums.h"
#include "model/SystemUser.h"
#include "../view/Profile.h"

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTextEdit>
#include <QInputDialog>
#include <QDoubleSpinBox>
#include <QLocale>

PharmacistDashboardWidget::PharmacistDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<AppointmentService> appointmentService,
    std::shared_ptr<MedicalRecordService> medicalRecordService,
    std::shared_ptr<PharmacyService> pharmacyService,
    std::shared_ptr<BillingService> billingService,
    QWidget *parent)
    : BaseDashboardWidget(user, staffService, patientService, appointmentService, parent),
      m_medicalRecordService(medicalRecordService),
      m_pharmacyService(pharmacyService),
      m_billingService(billingService) {
    initializeDashboard();
}

void PharmacistDashboardWidget::fillDashboardData() {
    buildSidebar();

    if (m_currentUser && m_docNameLabel) {
        m_docNameLabel->setText(m_currentUser->getFullName() + " (Dược sĩ)");
    }

    m_stackedWidget = new QStackedWidget(m_mainContentWidget);
    m_stackedWidget->setObjectName("StackedWidget");
    m_stackedWidget->setStyleSheet("QStackedWidget#StackedWidget > QWidget { background-color: #EEF2F6; }");
    m_mainContentLayout->addWidget(m_stackedWidget, 1);

    buildOverviewPage();
    buildInventoryPage();
    buildDispensingPage();
    buildBillingPage();
    buildReportsPage();

    switchPage(0, m_btnDash);
}

void PharmacistDashboardWidget::buildSidebar() {
    if (!m_sidebarLayout) return;

    m_btnDash = new QPushButton("Tổng Quan", m_sidebarFrame);
    m_btnInventory = new QPushButton("Kho Thuốc", m_sidebarFrame);
    m_btnDispensing = new QPushButton("Cấp Phát Thuốc", m_sidebarFrame);
    m_btnBilling = new QPushButton("Hóa Đơn & Thu Tiền", m_sidebarFrame);
    m_btnReports = new QPushButton("Báo Cáo Thống Kê", m_sidebarFrame);

    m_sidebarLayout->addWidget(m_btnDash);
    m_sidebarLayout->addWidget(m_btnInventory);
    m_sidebarLayout->addWidget(m_btnDispensing);
    m_sidebarLayout->addWidget(m_btnBilling);
    m_sidebarLayout->addWidget(m_btnReports);
    m_sidebarLayout->addStretch();

    m_btnLogout = new QPushButton("Đăng Xuất", m_sidebarFrame);
    m_btnLogout->setStyleSheet(
        "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; "
        "color: #D32F2F; border: none; border-radius: 0px; background-color: "
        "transparent; font-weight: bold; }"
        "QPushButton:hover { background-color: #FFEBEE; }");
    m_btnLogout->setCursor(Qt::PointingHandCursor);
    m_sidebarLayout->addWidget(m_btnLogout);
    m_sidebarLayout->addSpacing(30);

    connect(m_btnLogout, &QPushButton::clicked, this, &BaseDashboardWidget::logoutRequested);

    connect(m_btnDash, &QPushButton::clicked, this, [this]() { switchPage(0, m_btnDash); });
    connect(m_btnInventory, &QPushButton::clicked, this, [this]() { switchPage(1, m_btnInventory); });
    connect(m_btnDispensing, &QPushButton::clicked, this, [this]() { switchPage(2, m_btnDispensing); });
    connect(m_btnBilling, &QPushButton::clicked, this, [this]() { switchPage(3, m_btnBilling); });
    connect(m_btnReports, &QPushButton::clicked, this, [this]() { switchPage(4, m_btnReports); });
}

void PharmacistDashboardWidget::switchPage(int index, QPushButton* activeBtn) {
    if (!m_stackedWidget) return;

    m_stackedWidget->setCurrentIndex(index);

    QPushButton* buttons[] = { m_btnDash, m_btnInventory, m_btnDispensing, m_btnBilling, m_btnReports };
    for (auto* btn : buttons) {
        if (btn) {
            btn->setObjectName("");
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
        }
    }

    if (activeBtn) {
        activeBtn->setObjectName("activeBtn");
        activeBtn->style()->unpolish(activeBtn);
        activeBtn->style()->polish(activeBtn);
    }

    if (index == 0) refreshOverviewStats();
    else if (index == 1) performInventorySearch();
    else if (index == 2) performPrescriptionSearch();
    else if (index == 3) {
        performInvoiceSearch();
        loadPendingBilling();
    }
    else if (index == 4) generateReport();
}

QFrame* PharmacistDashboardWidget::makeCard(QWidget* parent) {
    QFrame* card = new QFrame(parent ? parent : m_mainContentWidget);
    card->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #EAEAEA; border-radius: 8px; }");
    
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(10);
    shadow->setOffset(0, 2);
    shadow->setColor(QColor(0, 0, 0, 15));
    card->setGraphicsEffect(shadow);
    
    return card;
}

// ─────────────────────────────────────────────────────────────────────────────
// PAGE 1: OVERVIEW (TỔNG QUAN)
// ─────────────────────────────────────────────────────────────────────────────
void PharmacistDashboardWidget::buildOverviewPage() {
    m_overviewPage = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_overviewPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(20);

    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(16);

    QFrame* card1 = makeCard(m_overviewPage);
    QVBoxLayout* lay1 = new QVBoxLayout(card1);
    lay1->addWidget(new QLabel("Tổng số loại thuốc", card1));
    m_lblStatTotalMeds = new QLabel("0", card1);
    m_lblStatTotalMeds->setStyleSheet("font-size: 24px; font-weight: bold; color: #2B6CB0;");
    lay1->addWidget(m_lblStatTotalMeds);
    cardsLayout->addWidget(card1);

    QFrame* card2 = makeCard(m_overviewPage);
    QVBoxLayout* lay2 = new QVBoxLayout(card2);
    lay2->addWidget(new QLabel("Thuốc sắp hết hàng", card2));
    m_lblStatLowStock = new QLabel("0", card2);
    m_lblStatLowStock->setStyleSheet("font-size: 24px; font-weight: bold; color: #E53E3E;");
    lay2->addWidget(m_lblStatLowStock);
    cardsLayout->addWidget(card2);

    QFrame* card3 = makeCard(m_overviewPage);
    QVBoxLayout* lay3 = new QVBoxLayout(card3);
    lay3->addWidget(new QLabel("Thuốc sắp hết hạn (<30 ngày)", card3));
    m_lblStatExpiring = new QLabel("0", card3);
    m_lblStatExpiring->setStyleSheet("font-size: 24px; font-weight: bold; color: #DD6B20;");
    lay3->addWidget(m_lblStatExpiring);
    cardsLayout->addWidget(card3);

    QFrame* card4 = makeCard(m_overviewPage);
    QVBoxLayout* lay4 = new QVBoxLayout(card4);
    lay4->addWidget(new QLabel("Đơn thuốc chờ cấp phát", card4));
    m_lblStatPendingPresc = new QLabel("0", card4);
    m_lblStatPendingPresc->setStyleSheet("font-size: 24px; font-weight: bold; color: #319795;");
    lay4->addWidget(m_lblStatPendingPresc);
    cardsLayout->addWidget(card4);

    mainLayout->addLayout(cardsLayout);

    QFrame* tableCard = makeCard(m_overviewPage);
    QVBoxLayout* tableCardLayout = new QVBoxLayout(tableCard);
    
    QLabel* lblAlertTitle = new QLabel("CẢNH BÁO TỒN KHO KHẨN CẤP", tableCard);
    lblAlertTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #2D3748;");
    tableCardLayout->addWidget(lblAlertTitle);

    m_tblOverviewAlerts = new QTableWidget(tableCard);
    m_tblOverviewAlerts->setColumnCount(5);
    m_tblOverviewAlerts->setHorizontalHeaderLabels({"Tên thuốc", "Tồn kho", "Đơn vị", "Hạn dùng", "Cảnh báo"});
    m_tblOverviewAlerts->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tblOverviewAlerts->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableCardLayout->addWidget(m_tblOverviewAlerts);

    mainLayout->addWidget(tableCard, 1);
    m_stackedWidget->addWidget(m_overviewPage);
}

void PharmacistDashboardWidget::refreshOverviewStats() {
    auto lowStock = m_pharmacyService->getLowStockMedications();
    auto expiring = m_pharmacyService->getExpiringMedications(30);

    m_lblStatLowStock->setText(QString::number(lowStock.size()));
    m_lblStatExpiring->setText(QString::number(expiring.size()));

    MedicationSearchCriteria criteria;
    criteria.pageSize = 1;
    auto pagedMeds = m_pharmacyService->searchMedicationsPaged(criteria);
    m_lblStatTotalMeds->setText(QString::number(pagedMeds.totalCount));

    PrescriptionSearchCriteria pCriteria;
    pCriteria.status = "PENDING";
    pCriteria.pageSize = 1;
    auto pagedPresc = m_pharmacyService->searchPrescriptionsPaged(pCriteria);
    m_lblStatPendingPresc->setText(QString::number(pagedPresc.totalCount));

    m_tblOverviewAlerts->setRowCount(0);
    int row = 0;

    QList<std::pair<MedicationSummaryDTO, QString>> alerts;
    for (const auto& m : lowStock) {
        alerts.append({m, "Sắp hết hàng"});
    }
    for (const auto& m : expiring) {
        bool exists = false;
        for (const auto& a : alerts) {
            if (a.first.medicationId == m.medicationId) {
                exists = true; break;
            }
        }
        if (!exists) alerts.append({m, "Sắp hết hạn"});
    }

    for (const auto& alert : alerts) {
        m_tblOverviewAlerts->insertRow(row);
        m_tblOverviewAlerts->setItem(row, 0, new QTableWidgetItem(alert.first.brandName));
        m_tblOverviewAlerts->setItem(row, 1, new QTableWidgetItem(QString::number(alert.first.stockQuantity)));
        m_tblOverviewAlerts->setItem(row, 2, new QTableWidgetItem(alert.first.unit));
        m_tblOverviewAlerts->setItem(row, 3, new QTableWidgetItem(alert.first.expiryDate.toString("dd/MM/yyyy")));
        
        QTableWidgetItem* statusItem = new QTableWidgetItem(alert.second);
        if (alert.second == "Sắp hết hàng") {
            statusItem->setForeground(QBrush(QColor("#E53E3E")));
            statusItem->setFont(QFont("Arial", -1, QFont::Bold));
        } else {
            statusItem->setForeground(QBrush(QColor("#DD6B20")));
            statusItem->setFont(QFont("Arial", -1, QFont::Bold));
        }
        m_tblOverviewAlerts->setItem(row, 4, statusItem);
        row++;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PAGE 2: INVENTORY (KHO THUỐC)
// ─────────────────────────────────────────────────────────────────────────────
void PharmacistDashboardWidget::buildInventoryPage() {
    m_inventoryPage = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_inventoryPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(10);

    m_txtInvKeyword = new QLineEdit(m_inventoryPage);
    m_txtInvKeyword->setPlaceholderText("Tìm theo tên thuốc, nhà sản xuất...");
    m_txtInvKeyword->setFixedWidth(250);

    m_cbInvCategory = new QComboBox(m_inventoryPage);
    m_cbInvCategory->addItem("Tất cả danh mục", "");
    m_cbInvCategory->addItems({"Kháng sinh", "Giảm đau", "Kháng viêm", "Hạ sốt", "Tim mạch", "Tiêu hóa", "Khác"});

    m_cbInvStatus = new QComboBox(m_inventoryPage);
    m_cbInvStatus->addItems({"Tất cả trạng thái", "Còn hàng", "Hết hàng", "Sắp hết hàng", "Sắp hết hạn"});

    QPushButton* btnSearch = new QPushButton("Tìm kiếm", m_inventoryPage);
    btnSearch->setStyleSheet("background-color: #4B94F2; color: white; padding: 6px 16px; border-radius: 4px; border: none; font-weight: bold;");
    
    QPushButton* btnAddMed = new QPushButton("+ Thêm thuốc mới", m_inventoryPage);
    btnAddMed->setStyleSheet("background-color: #10B981; color: white; padding: 6px 16px; border-radius: 4px; border: none; font-weight: bold;");

    filterLayout->addWidget(m_txtInvKeyword);
    filterLayout->addWidget(m_cbInvCategory);
    filterLayout->addWidget(m_cbInvStatus);
    filterLayout->addWidget(btnSearch);
    filterLayout->addStretch();
    filterLayout->addWidget(btnAddMed);
    mainLayout->addLayout(filterLayout);

    QFrame* tblCard = makeCard(m_inventoryPage);
    QVBoxLayout* tblLayout = new QVBoxLayout(tblCard);

    m_tblInventory = new QTableWidget(tblCard);
    m_tblInventory->setColumnCount(8);
    m_tblInventory->setHorizontalHeaderLabels({"Mã", "Tên thương hiệu", "Đơn vị", "Đơn giá", "Tồn kho", "Hạn sử dụng", "Trạng thái", "Hành động"});
    m_tblInventory->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tblInventory->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblLayout->addWidget(m_tblInventory);

    QHBoxLayout* pageLayout = new QHBoxLayout();
    m_btnInvPrev = new QPushButton("Trang trước", tblCard);
    m_btnInvNext = new QPushButton("Trang sau", tblCard);
    m_lblInvPageInfo = new QLabel("Trang 1 / 1", tblCard);
    
    pageLayout->addStretch();
    pageLayout->addWidget(m_btnInvPrev);
    pageLayout->addWidget(m_lblInvPageInfo);
    pageLayout->addWidget(m_btnInvNext);
    pageLayout->addStretch();
    tblLayout->addLayout(pageLayout);

    mainLayout->addWidget(tblCard, 1);
    m_stackedWidget->addWidget(m_inventoryPage);

    connect(btnSearch, &QPushButton::clicked, this, &PharmacistDashboardWidget::performInventorySearch);
    connect(m_txtInvKeyword, &QLineEdit::returnPressed, this, &PharmacistDashboardWidget::performInventorySearch);
    connect(m_btnInvPrev, &QPushButton::clicked, this, &PharmacistDashboardWidget::prevInventoryPage);
    connect(m_btnInvNext, &QPushButton::clicked, this, &PharmacistDashboardWidget::nextInventoryPage);
    
    connect(btnAddMed, &QPushButton::clicked, this, [this]() {
        AddMedicationDialog dlg(m_pharmacyService, -1, this);
        if (dlg.exec() == QDialog::Accepted) performInventorySearch();
    });
}

void PharmacistDashboardWidget::performInventorySearch() {
    m_tblInventory->setRowCount(0);

    MedicationSearchCriteria criteria;
    criteria.keyword = m_txtInvKeyword->text().simplified();
    criteria.page = m_invCurrentPage;
    criteria.pageSize = m_invPageSize;

    QString cat = m_cbInvCategory->currentText();
    if (cat != "Tất cả danh mục" && !cat.isEmpty()) {
        criteria.selectedCategories.append(cat);
    }

    QString status = m_cbInvStatus->currentText();
    if (status == "Còn hàng") criteria.inStockOnly = true;
    else if (status == "Hết hàng") criteria.inStockOnly = false;
    else if (status == "Sắp hết hàng") criteria.lowStockOnly = true;
    else if (status == "Sắp hết hạn") criteria.excludeExpired = false;

    auto result = m_pharmacyService->searchMedicationsPaged(criteria);
    m_invTotalPages = (result.totalCount + m_invPageSize - 1) / m_invPageSize;
    if (m_invTotalPages < 1) m_invTotalPages = 1;
    m_invCurrentPage = result.page;

    int row = 0;
    for (const auto& med : result.items) {
        m_tblInventory->insertRow(row);
        m_tblInventory->setItem(row, 0, new QTableWidgetItem(QString::number(med.medicationId)));
        m_tblInventory->setItem(row, 1, new QTableWidgetItem(med.brandName));
        m_tblInventory->setItem(row, 2, new QTableWidgetItem(med.unit));
        m_tblInventory->setItem(row, 3, new QTableWidgetItem(QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(med.unitPrice, 'f', 0))));
        m_tblInventory->setItem(row, 4, new QTableWidgetItem(QString::number(med.stockQuantity)));
        m_tblInventory->setItem(row, 5, new QTableWidgetItem(med.expiryDate.toString("dd/MM/yyyy")));
        
        QString statusText = "Bình thường";
        QColor statusColor = QColor("#2F855A");
        if (med.isCriticalStock) {
            statusText = "Tồn kho khẩn cấp";
            statusColor = QColor("#C53030");
        } else if (med.isLowStock) {
            statusText = "Sắp hết hàng";
            statusColor = QColor("#DD6B20");
        } else if (med.isExpiringSoon) {
            statusText = "Sắp hết hạn";
            statusColor = QColor("#D69E2E");
        }
        
        QTableWidgetItem* statusItem = new QTableWidgetItem(statusText);
        statusItem->setForeground(QBrush(statusColor));
        statusItem->setFont(QFont("Arial", -1, QFont::Bold));
        m_tblInventory->setItem(row, 6, statusItem);

        QPushButton* btnEdit = new QPushButton("Chỉnh sửa", m_tblInventory);
        btnEdit->setStyleSheet("background-color: #E8F0FE; color: #4B94F2; border: 1px solid #4B94F2; border-radius: 4px; padding: 2px 8px;");
        connect(btnEdit, &QPushButton::clicked, this, [this, medId = med.medicationId]() {
            AddMedicationDialog dlg(m_pharmacyService, medId, this);
            if (dlg.exec() == QDialog::Accepted) performInventorySearch();
        });
        m_tblInventory->setCellWidget(row, 7, btnEdit);
        row++;
    }

    m_lblInvPageInfo->setText(QString("Trang %1 / %2").arg(m_invCurrentPage).arg(m_invTotalPages));
    m_btnInvPrev->setEnabled(m_invCurrentPage > 1);
    m_btnInvNext->setEnabled(m_invCurrentPage < m_invTotalPages);
}

void PharmacistDashboardWidget::prevInventoryPage() {
    if (m_invCurrentPage > 1) {
        m_invCurrentPage--;
        performInventorySearch();
    }
}

void PharmacistDashboardWidget::nextInventoryPage() {
    if (m_invCurrentPage < m_invTotalPages) {
        m_invCurrentPage++;
        performInventorySearch();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PAGE 3: DISPENSING (CẤP PHÁT THUỐC)
// ─────────────────────────────────────────────────────────────────────────────
void PharmacistDashboardWidget::buildDispensingPage() {
    m_dispensingPage = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(m_dispensingPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(12);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    m_txtPrescKeyword = new QLineEdit(m_dispensingPage);
    m_txtPrescKeyword->setPlaceholderText("Tìm bệnh nhân, bác sĩ...");
    m_cbPrescStatus = new QComboBox(m_dispensingPage);
    m_cbPrescStatus->addItem("Tất cả trạng thái", "");
    m_cbPrescStatus->addItem(prescriptionStatusToVi(PrescriptionStatus::Pending), "PENDING");
    m_cbPrescStatus->addItem(prescriptionStatusToVi(PrescriptionStatus::Dispensed), "DISPENSED");
    m_cbPrescStatus->addItem(prescriptionStatusToVi(PrescriptionStatus::Cancelled), "CANCELLED");
    
    QPushButton* btnSearch = new QPushButton("Lọc", m_dispensingPage);
    btnSearch->setStyleSheet("background-color: #4B94F2; color: white; padding: 5px 12px; border-radius: 4px; border: none; font-weight: bold;");

    searchLayout->addWidget(m_txtPrescKeyword);
    searchLayout->addWidget(m_cbPrescStatus);
    searchLayout->addWidget(btnSearch);
    leftLayout->addLayout(searchLayout);

    QFrame* tblCard = makeCard(m_dispensingPage);
    QVBoxLayout* tblLayout = new QVBoxLayout(tblCard);
    
    m_tblPrescriptions = new QTableWidget(tblCard);
    m_tblPrescriptions->setColumnCount(6);
    m_tblPrescriptions->setHorizontalHeaderLabels({"Mã đơn", "Bệnh nhân", "Tuổi", "Bác sĩ", "Ngày kê", "Trạng thái"});
    m_tblPrescriptions->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tblPrescriptions->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblPrescriptions->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tblPrescriptions->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblLayout->addWidget(m_tblPrescriptions);

    QHBoxLayout* pageLayout = new QHBoxLayout();
    m_btnPrescPrev = new QPushButton("Trước", tblCard);
    m_btnPrescNext = new QPushButton("Sau", tblCard);
    m_lblPrescPageInfo = new QLabel("Trang 1 / 1", tblCard);
    pageLayout->addStretch();
    pageLayout->addWidget(m_btnPrescPrev);
    pageLayout->addWidget(m_lblPrescPageInfo);
    pageLayout->addWidget(m_btnPrescNext);
    pageLayout->addStretch();
    tblLayout->addLayout(pageLayout);

    leftLayout->addWidget(tblCard);
    mainLayout->addLayout(leftLayout, 6);

    QFrame* detCard = makeCard(m_dispensingPage);
    QVBoxLayout* detLayout = new QVBoxLayout(detCard);
    detLayout->setSpacing(14);

    QLabel* lblDetTitle = new QLabel("CHI TIẾT ĐƠN THUỐC & CẤP PHÁT", detCard);
    lblDetTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #4A5568; border-bottom: 2px solid #E2E8F0; padding-bottom: 8px;");
    detLayout->addWidget(lblDetTitle);

    m_lblDetPatientName = new QLabel("<b>Bệnh nhân:</b> Chưa chọn", detCard);
    m_lblDetPatientInfo = new QLabel("Tuổi: -- | Giới tính: --", detCard);
    m_lblDetDiagnosis = new QLabel("Chẩn đoán: --", detCard);
    detLayout->addWidget(m_lblDetPatientName);
    detLayout->addWidget(m_lblDetPatientInfo);
    detLayout->addWidget(m_lblDetDiagnosis);

    m_tblDetItems = new QTableWidget(detCard);
    m_tblDetItems->setColumnCount(3);
    m_tblDetItems->setHorizontalHeaderLabels({"Tên thuốc", "SL", "Liều dùng"});
    m_tblDetItems->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tblDetItems->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tblDetItems->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tblDetItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
    detLayout->addWidget(m_tblDetItems);

    m_lblDetTotalCost = new QLabel("Tổng giá trị thuốc: 0 VND", detCard);
    m_lblDetTotalCost->setStyleSheet("font-weight: bold; color: #2B6CB0; font-size: 14px;");
    detLayout->addWidget(m_lblDetTotalCost);

    detLayout->addWidget(new QLabel("Ghi chú cho dược sĩ:", detCard));
    m_txtDetNotes = new QTextEdit(detCard);
    m_txtDetNotes->setReadOnly(true);
    m_txtDetNotes->setMaximumHeight(80);
    detLayout->addWidget(m_txtDetNotes);

    QHBoxLayout* actLayout = new QHBoxLayout();
    m_btnDispenseAction = new QPushButton("Cấp Thuốc", detCard);
    m_btnDispenseAction->setStyleSheet("background-color: #10B981; color: white; padding: 8px 12px; border-radius: 4px; border: none; font-weight: bold;");
    m_btnDispenseAction->setEnabled(false);

    m_btnCancelPrescAction = new QPushButton("Hủy Đơn", detCard);
    m_btnCancelPrescAction->setStyleSheet("background-color: #EF4444; color: white; padding: 8px 12px; border-radius: 4px; border: none; font-weight: bold;");
    m_btnCancelPrescAction->setEnabled(false);

    m_btnPrintReceiptAction = new QPushButton("In Phiếu", detCard);
    m_btnPrintReceiptAction->setStyleSheet("background-color: #3B82F6; color: white; padding: 8px 12px; border-radius: 4px; border: none; font-weight: bold;");
    m_btnPrintReceiptAction->setEnabled(false);

    actLayout->addWidget(m_btnDispenseAction);
    actLayout->addWidget(m_btnCancelPrescAction);
    actLayout->addWidget(m_btnPrintReceiptAction);
    detLayout->addLayout(actLayout);

    mainLayout->addWidget(detCard, 4);
    m_stackedWidget->addWidget(m_dispensingPage);

    connect(btnSearch, &QPushButton::clicked, this, &PharmacistDashboardWidget::performPrescriptionSearch);
    connect(m_txtPrescKeyword, &QLineEdit::returnPressed, this, &PharmacistDashboardWidget::performPrescriptionSearch);
    connect(m_btnPrescPrev, &QPushButton::clicked, this, &PharmacistDashboardWidget::prevPrescriptionPage);
    connect(m_btnPrescNext, &QPushButton::clicked, this, &PharmacistDashboardWidget::nextPrescriptionPage);
    connect(m_tblPrescriptions, &QTableWidget::cellClicked, this, &PharmacistDashboardWidget::selectPrescriptionRow);
    connect(m_btnDispenseAction, &QPushButton::clicked, this, &PharmacistDashboardWidget::handleDispensePrescription);
    connect(m_btnCancelPrescAction, &QPushButton::clicked, this, &PharmacistDashboardWidget::handleCancelPrescription);
    connect(m_btnPrintReceiptAction, &QPushButton::clicked, this, &PharmacistDashboardWidget::handlePrintReceipt);
}

void PharmacistDashboardWidget::performPrescriptionSearch() {
    m_tblPrescriptions->setRowCount(0);
    m_selectedPrescriptionId = -1;
    m_btnDispenseAction->setEnabled(false);
    m_btnCancelPrescAction->setEnabled(false);
    m_btnPrintReceiptAction->setEnabled(false);

    PrescriptionSearchCriteria criteria;
    criteria.keyword = m_txtPrescKeyword->text().simplified();
    criteria.page = m_prescCurrentPage;
    criteria.pageSize = m_prescPageSize;

    QString status = m_cbPrescStatus->currentData().toString();
    if (!status.isEmpty()) criteria.status = status;

    auto result = m_pharmacyService->searchPrescriptionsPaged(criteria);
    m_prescTotalPages = (result.totalCount + m_prescPageSize - 1) / m_prescPageSize;
    if (m_prescTotalPages < 1) m_prescTotalPages = 1;
    m_prescCurrentPage = result.page;

    int row = 0;
    for (const auto& presc : result.items) {
        m_tblPrescriptions->insertRow(row);
        
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(presc.prescriptionId));
        QTableWidgetItem* patItem = new QTableWidgetItem(presc.patientName);
        QTableWidgetItem* ageItem = new QTableWidgetItem(QString::number(presc.patientAge));
        QTableWidgetItem* docItem = new QTableWidgetItem(presc.doctorName);
        QTableWidgetItem* dateItem = new QTableWidgetItem(presc.prescribedAt.toString("dd/MM/yyyy HH:mm"));

        QString statStr = prescriptionStatusToVi(presc.status);
        QTableWidgetItem* statItem = new QTableWidgetItem(statStr);

        if (presc.status == PrescriptionStatus::Pending) statItem->setForeground(QBrush(QColor("#DD6B20")));
        else if (presc.status == PrescriptionStatus::Dispensed) statItem->setForeground(QBrush(QColor("#2F855A")));
        else statItem->setForeground(QBrush(QColor("#A0AEC0")));
        statItem->setFont(QFont("Arial", -1, QFont::Bold));

        idItem->setData(Qt::UserRole, presc.recordId);
        idItem->setData(Qt::UserRole + 1, presc.patientName);
        idItem->setData(Qt::UserRole + 2, presc.patientAge);
        idItem->setData(Qt::UserRole + 3, presc.patientGender);
        idItem->setData(Qt::UserRole + 4, presc.diagnosis);
        idItem->setData(Qt::UserRole + 5, presc.notes);
        idItem->setData(Qt::UserRole + 6, presc.totalAmount);
        idItem->setData(Qt::UserRole + 7, static_cast<int>(presc.status));

        m_tblPrescriptions->setItem(row, 0, idItem);
        m_tblPrescriptions->setItem(row, 1, patItem);
        m_tblPrescriptions->setItem(row, 2, ageItem);
        m_tblPrescriptions->setItem(row, 3, docItem);
        m_tblPrescriptions->setItem(row, 4, dateItem);
        m_tblPrescriptions->setItem(row, 5, statItem);
        row++;
    }

    m_lblPrescPageInfo->setText(QString("Trang %1 / %2").arg(m_prescCurrentPage).arg(m_prescTotalPages));
    m_btnPrescPrev->setEnabled(m_prescCurrentPage > 1);
    m_btnPrescNext->setEnabled(m_prescCurrentPage < m_prescTotalPages);
}

void PharmacistDashboardWidget::prevPrescriptionPage() {
    if (m_prescCurrentPage > 1) {
        m_prescCurrentPage--;
        performPrescriptionSearch();
    }
}

void PharmacistDashboardWidget::nextPrescriptionPage() {
    if (m_prescCurrentPage < m_prescTotalPages) {
        m_prescCurrentPage++;
        performPrescriptionSearch();
    }
}

void PharmacistDashboardWidget::selectPrescriptionRow(int row) {
    if (row < 0) return;

    QTableWidgetItem* idItem = m_tblPrescriptions->item(row, 0);
    if (!idItem) return;

    m_selectedPrescriptionId = idItem->text().toInt();
    m_selectedPrescRecordId = idItem->data(Qt::UserRole).toInt();
    
    QString patName = idItem->data(Qt::UserRole + 1).toString();
    int patAge = idItem->data(Qt::UserRole + 2).toInt();
    QString patGender = idItem->data(Qt::UserRole + 3).toString();
    QString diagnosis = idItem->data(Qt::UserRole + 4).toString();
    QString notes = idItem->data(Qt::UserRole + 5).toString();
    double totalAmount = idItem->data(Qt::UserRole + 6).toDouble();
    PrescriptionStatus status = static_cast<PrescriptionStatus>(idItem->data(Qt::UserRole + 7).toInt());

    m_lblDetPatientName->setText(QString("<b>Bệnh nhân:</b> %1").arg(patName));
    m_lblDetPatientInfo->setText(QString("Tuổi: %1 | Giới tính: %2").arg(patAge).arg(patGender));
    m_lblDetDiagnosis->setText(QString("<b>Chẩn đoán:</b> %1").arg(diagnosis));
    m_txtDetNotes->setText(notes);
    m_lblDetTotalCost->setText(QString("Tổng giá trị thuốc: %1 VND").arg(QLocale(QLocale::Vietnamese).toString(totalAmount, 'f', 0)));

    m_tblDetItems->setRowCount(0);
    auto optPresc = m_pharmacyService->getPrescriptionByRecordId(m_selectedPrescRecordId);
    if (optPresc.has_value()) {
        int rRow = 0;
        for (const auto& item : optPresc.value().items) {
            m_tblDetItems->insertRow(rRow);
            m_tblDetItems->setItem(rRow, 0, new QTableWidgetItem(item.brandName));
            m_tblDetItems->setItem(rRow, 1, new QTableWidgetItem(QString::number(item.quantity)));
            m_tblDetItems->setItem(rRow, 2, new QTableWidgetItem(QString("%1 - %2").arg(item.dosage).arg(item.frequency)));
            rRow++;
        }
    }

    if (status == PrescriptionStatus::Pending) {
        m_btnDispenseAction->setEnabled(true);
        m_btnCancelPrescAction->setEnabled(true);
        m_btnPrintReceiptAction->setEnabled(true);
    } else if (status == PrescriptionStatus::Dispensed) {
        m_btnDispenseAction->setEnabled(false);
        m_btnCancelPrescAction->setEnabled(false);
        m_btnPrintReceiptAction->setEnabled(true);
    } else {
        m_btnDispenseAction->setEnabled(false);
        m_btnCancelPrescAction->setEnabled(false);
        m_btnPrintReceiptAction->setEnabled(false);
    }
}

void PharmacistDashboardWidget::handleDispensePrescription() {
    if (m_selectedPrescriptionId == -1) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Cấp phát thuốc", "Xác nhận đã chuẩn bị và phát đủ thuốc cho bệnh nhân?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        int pharmacistId = m_currentUser ? m_currentUser->getAccountId() : 0;
        QString err = m_pharmacyService->dispensePrescription(m_selectedPrescriptionId, pharmacistId);
        
        if (err.isEmpty()) {
            QMessageBox::information(this, "Thành công", "Cấp phát đơn thuốc thành công!");
            
            int recordId = m_selectedPrescRecordId;
            auto optPresc = m_pharmacyService->getPrescriptionByRecordId(recordId);
            auto optRecord = m_medicalRecordService->getMedicalRecordById(recordId);

            performPrescriptionSearch(); // Cập nhật lại danh sách cấp phát thuốc

            if (optPresc.has_value() && optRecord.has_value()) {
                showCreateInvoiceDialog(optPresc.value(), optRecord->patientId);
            }
        } else {
            QMessageBox::critical(this, "Thất bại", err);
        }
    }
}

void PharmacistDashboardWidget::handleCancelPrescription() {
    if (m_selectedPrescriptionId == -1) return;

    bool ok;
    QString reason = QInputDialog::getText(this, "Hủy đơn thuốc", "Nhập lý do hủy đơn thuốc:*", QLineEdit::Normal, "", &ok).trimmed();

    if (ok) {
        if (reason.isEmpty()) {
            QMessageBox::warning(this, "Cảnh báo", "Bắt buộc nhập lý do hủy đơn.");
            return;
        }
        
        int pharmacistId = m_currentUser ? m_currentUser->getAccountId() : 0;
        QString err = m_pharmacyService->cancelPrescription(m_selectedPrescriptionId, pharmacistId, reason);
        
        if (err.isEmpty()) {
            QMessageBox::information(this, "Thành công", "Đã hủy đơn thuốc thành công!");
            performPrescriptionSearch();
        } else {
            QMessageBox::critical(this, "Thất bại", err);
        }
    }
}

void PharmacistDashboardWidget::handlePrintReceipt() {
    if (m_selectedPrescriptionId == -1) return;
    
    auto optPresc = m_pharmacyService->getPrescriptionByRecordId(m_selectedPrescRecordId);
    if (!optPresc.has_value()) return;
    
    auto presc = optPresc.value();
    
    QString html = "<html><head><style>"
                   "body { font-family: 'Segoe UI', Arial, sans-serif; margin: 20px; color: #333; }"
                   ".header { text-align: center; margin-bottom: 20px; }"
                   ".title { font-size: 20px; font-weight: bold; color: #1E3A8A; text-transform: uppercase; }"
                   ".info-table { width: 100%; margin-bottom: 20px; border-collapse: collapse; }"
                   ".info-table td { padding: 4px; vertical-align: top; }"
                   ".items-table { width: 100%; border-collapse: collapse; margin-top: 10px; }"
                   ".items-table th, .items-table td { border: 1px solid #ddd; padding: 8px; text-align: left; }"
                   ".items-table th { background-color: #f2f2f2; font-weight: bold; }"
                   ".total { text-align: right; font-size: 16px; font-weight: bold; margin-top: 15px; margin-bottom: 30px; }"
                   ".signatures { width: 100%; margin-top: 30px; }"
                   ".signatures td { text-align: center; width: 50%; }"
                   "</style></head><body>"
                   "<div class='header'>"
                   "  <div style='font-size: 14px; font-weight: bold;'>NOVA CARE CLINIC</div>"
                   "  <div style='font-size: 11px;'>Địa chỉ: 123 Đường Nguyễn Văn Cừ, Quận 5, TP. HCM</div>"
                   "  <hr/>"
                   "  <div class='title'>PHIẾU XUẤT KHO THUỐC</div>"
                   "  <div>Mã đơn thuốc: " + QString::number(presc.prescriptionId) + "</div>"
                   "</div>"
                   "<table class='info-table'>"
                   "  <tr>"
                   "    <td><b>Bệnh nhân:</b> " + presc.patientName + "</td>"
                   "    <td><b>Tuổi:</b> " + QString::number(presc.patientAge) + " | <b>Giới tính:</b> " + presc.patientGender + "</td>"
                   "  </tr>"
                   "  <tr>"
                   "    <td><b>Bác sĩ chỉ định:</b> " + presc.doctorName + "</td>"
                   "    <td><b>Ngày xuất:</b> " + QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm") + "</td>"
                   "  </tr>"
                   "  <tr>"
                   "    <td colspan='2'><b>Chẩn đoán:</b> " + presc.diagnosis + "</td>"
                   "  </tr>"
                   "</table>"
                   "<table class='items-table'>"
                   "  <thead>"
                   "    <tr>"
                   "      <th>STT</th>"
                   "      <th>Tên thuốc</th>"
                   "      <th>Số lượng</th>"
                   "      <th>Cách dùng</th>"
                   "    </tr>"
                   "  </thead>"
                   "  <tbody>";
                   
    int index = 1;
    for (const auto& item : presc.items) {
        html += "    <tr>"
                "      <td>" + QString::number(index++) + "</td>"
                "      <td>" + item.brandName + "</td>"
                "      <td>" + QString::number(item.quantity) + "</td>"
                "      <td>" + item.dosage + " - " + item.frequency + " trong " + QString::number(item.durationDays) + " ngày</td>"
                "    </tr>";
    }
    
    html += "  </tbody>"
            "</table>"
            "<div class='total'>Tổng số loại thuốc: " + QString::number(presc.items.size()) + "</div>"
            "<table class='signatures'>"
            "  <tr>"
            "    <td><b>Người nhận thuốc</b><br/><small>(Ký và ghi rõ họ tên)</small></td>"
            "    <td><b>Dược sĩ cấp phát</b><br/><small>(Ký và ghi rõ họ tên)</small><br/><br/><br/>" + (m_currentUser ? m_currentUser->getFullName() : "") + "</td>"
            "  </tr>"
            "</table>"
            "</body></html>";

    QDialog previewDlg(this);
    previewDlg.setWindowTitle("Xem trước Phiếu xuất kho");
    previewDlg.setMinimumSize(600, 500);
    QVBoxLayout* prevLay = new QVBoxLayout(&previewDlg);
    
    QTextEdit* view = new QTextEdit(&previewDlg);
    view->setHtml(html);
    view->setReadOnly(true);
    prevLay->addWidget(view);
    
    QHBoxLayout* btnLay = new QHBoxLayout();
    QPushButton* btnPrint = new QPushButton("In Phiếu", &previewDlg);
    btnPrint->setStyleSheet("background-color: #4B94F2; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold;");
    QPushButton* btnClose = new QPushButton("Đóng", &previewDlg);
    btnLay->addWidget(btnPrint);
    btnLay->addWidget(btnClose);
    prevLay->addLayout(btnLay);
    
    connect(btnClose, &QPushButton::clicked, &previewDlg, &QDialog::reject);
    connect(btnPrint, &QPushButton::clicked, &previewDlg, [&previewDlg]() {
        QMessageBox::information(&previewDlg, "Thông báo", "Lệnh in đã được gửi đến máy in mặc định.");
        previewDlg.accept();
    });
    
    previewDlg.exec();
}

// ─────────────────────────────────────────────────────────────────────────────
// PAGE 4: BILLING & PAYMENT COLLECTION (LẬP HÓA ĐƠN & THU TIỀN)
// ─────────────────────────────────────────────────────────────────────────────
void PharmacistDashboardWidget::buildBillingPage() {
    m_billingPage = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_billingPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    m_tabBillingContainer = new QTabWidget(m_billingPage);

    // TAB 1: Danh sách hóa đơn lâm sàng
    QWidget* tabList = new QWidget(this);
    QHBoxLayout* tabListLayout = new QHBoxLayout(tabList);
    tabListLayout->setContentsMargins(10, 10, 10, 10);
    tabListLayout->setSpacing(14);

    QVBoxLayout* leftBill = new QVBoxLayout();
    leftBill->setSpacing(10);

    QHBoxLayout* billSearchLay = new QHBoxLayout();
    m_txtBillKeyword = new QLineEdit(tabList);
    m_txtBillKeyword->setPlaceholderText("Tìm mã hóa đơn...");
    m_cbBillStatus = new QComboBox(tabList);
    m_cbBillStatus->addItems({"Chưa thanh toán", "Đã thanh toán", "Đã hủy", "Tất cả trạng thái"});
    
    QPushButton* btnSearchBill = new QPushButton("Lọc hóa đơn", tabList);
    btnSearchBill->setStyleSheet("background-color: #4B94F2; color: white; padding: 4px 10px; border: none; border-radius: 4px; font-weight: bold;");
    billSearchLay->addWidget(m_txtBillKeyword);
    billSearchLay->addWidget(m_cbBillStatus);
    billSearchLay->addWidget(btnSearchBill);
    leftBill->addLayout(billSearchLay);

    m_tblInvoices = new QTableWidget(tabList);
    m_tblInvoices->setColumnCount(6);
    m_tblInvoices->setHorizontalHeaderLabels({"Mã HĐ", "Mã bệnh án", "Bệnh nhân", "Tổng tiền", "Ngày lập", "Trạng thái"});
    m_tblInvoices->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_tblInvoices->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tblInvoices->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tblInvoices->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftBill->addWidget(m_tblInvoices);
    tabListLayout->addLayout(leftBill, 6);

    QFrame* billDetCard = makeCard(tabList);
    QVBoxLayout* rightBill = new QVBoxLayout(billDetCard);
    rightBill->setSpacing(12);

    QLabel* lblBillDetTitle = new QLabel("CHI TIẾT HÓA ĐƠN", billDetCard);
    lblBillDetTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #4A5568; border-bottom: 2px solid #E2E8F0; padding-bottom: 6px;");
    rightBill->addWidget(lblBillDetTitle);

    m_lblBillPatientName = new QLabel("<b>Bệnh nhân:</b> Chưa chọn", billDetCard);
    m_lblBillDetails = new QLabel("Mã hóa đơn: -- | Ngày lập: --", billDetCard);
    rightBill->addWidget(m_lblBillPatientName);
    rightBill->addWidget(m_lblBillDetails);

    m_tblBillItems = new QTableWidget(billDetCard);
    m_tblBillItems->setColumnCount(3);
    m_tblBillItems->setHorizontalHeaderLabels({"Khoản thu", "SL", "Thành tiền"});
    m_tblBillItems->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tblBillItems->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tblBillItems->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rightBill->addWidget(m_tblBillItems);

    m_lblBillTotalAmount = new QLabel("Tổng thanh toán: 0 VND", billDetCard);
    m_lblBillTotalAmount->setStyleSheet("font-weight: bold; color: #E53E3E; font-size: 16px;");
    rightBill->addWidget(m_lblBillTotalAmount);

    QHBoxLayout* billActLay = new QHBoxLayout();
    m_btnCollectPayment = new QPushButton("Thu Tiền", billDetCard);
    m_btnCollectPayment->setStyleSheet("background-color: #10B981; color: white; padding: 8px 16px; border-radius: 4px; border: none; font-weight: bold;");
    m_btnCollectPayment->setEnabled(false);

    m_btnPrintInvoice = new QPushButton("In Hóa Đơn", billDetCard);
    m_btnPrintInvoice->setStyleSheet("background-color: #4B94F2; color: white; padding: 8px 16px; border-radius: 4px; border: none; font-weight: bold;");
    m_btnPrintInvoice->setEnabled(false);

    billActLay->addWidget(m_btnCollectPayment);
    billActLay->addWidget(m_btnPrintInvoice);
    rightBill->addLayout(billActLay);

    tabListLayout->addWidget(billDetCard, 4);
    m_tabBillingContainer->addTab(tabList, "Hóa đơn đã lập");

    // TAB 2: Danh sách chờ xuất hóa đơn
    QWidget* tabPending = new QWidget(this);
    QVBoxLayout* tabPendingLayout = new QVBoxLayout(tabPending);
    tabPendingLayout->setContentsMargins(10, 10, 10, 10);
    tabPendingLayout->setSpacing(10);

    QHBoxLayout* pendingSearchLay = new QHBoxLayout();
    m_txtPendingPatientId = new QLineEdit(tabPending);
    m_txtPendingPatientId->setPlaceholderText("Nhập Mã Bệnh Nhân (Patient ID) để tra cứu đơn thuốc...");
    m_btnSearchPending = new QPushButton("Tìm kiếm đơn thuốc", tabPending);
    m_btnSearchPending->setStyleSheet("background-color: #4B94F2; color: white; padding: 5px 12px; border: none; border-radius: 4px; font-weight: bold;");
    pendingSearchLay->addWidget(m_txtPendingPatientId);
    pendingSearchLay->addWidget(m_btnSearchPending);
    tabPendingLayout->addLayout(pendingSearchLay);

    m_tblPendingBilling = new QTableWidget(tabPending);
    m_tblPendingBilling->setColumnCount(6);
    m_tblPendingBilling->setHorizontalHeaderLabels({"Bệnh nhân", "Chẩn đoán", "Bác sĩ", "Ngày kê", "Tiền thuốc dự kiến", "Hành động"});
    m_tblPendingBilling->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tblPendingBilling->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tabPendingLayout->addWidget(m_tblPendingBilling);
    m_tabBillingContainer->addTab(tabPending, "Chờ xuất hóa đơn");

    mainLayout->addWidget(m_tabBillingContainer);
    m_stackedWidget->addWidget(m_billingPage);

    connect(btnSearchBill, &QPushButton::clicked, this, &PharmacistDashboardWidget::performInvoiceSearch);
    connect(m_txtBillKeyword, &QLineEdit::returnPressed, this, &PharmacistDashboardWidget::performInvoiceSearch);
    connect(m_tblInvoices, &QTableWidget::cellClicked, this, &PharmacistDashboardWidget::selectInvoiceRow);
    connect(m_btnCollectPayment, &QPushButton::clicked, this, &PharmacistDashboardWidget::handleCollectPayment);
    connect(m_btnPrintInvoice, &QPushButton::clicked, this, &PharmacistDashboardWidget::handlePrintInvoice);
    connect(m_btnSearchPending, &QPushButton::clicked, this, &PharmacistDashboardWidget::loadPendingBilling);
    connect(m_txtPendingPatientId, &QLineEdit::returnPressed, this, &PharmacistDashboardWidget::loadPendingBilling);
}

void PharmacistDashboardWidget::performInvoiceSearch() {
    m_tblInvoices->setRowCount(0);
    m_selectedInvoiceId = -1;
    m_btnCollectPayment->setEnabled(false);
    m_btnPrintInvoice->setEnabled(false);

    InvoiceSearchCriteria criteria;
    criteria.searchKey = m_txtBillKeyword->text().simplified();

    QString status = m_cbBillStatus->currentText();
    if (status == "Chưa thanh toán") criteria.status = "UNPAID";
    else if (status == "Đã thanh toán") criteria.status = "PAID";
    else if (status == "Đã hủy") criteria.status = "CANCELLED";

    auto result = m_billingService->searchInvoicesPaged(criteria);

    int row = 0;
    for (const auto& inv : result.items) {
        m_tblInvoices->insertRow(row);
        
        QTableWidgetItem* codeItem = new QTableWidgetItem(inv.invoiceCode);
        QTableWidgetItem* recItem = new QTableWidgetItem(inv.recordId.has_value() ? QString::number(inv.recordId.value()) : "--");
        QTableWidgetItem* patientItem = new QTableWidgetItem(QString("Mã bệnh nhân: %1").arg(inv.patientId));
        QTableWidgetItem* amountItem = new QTableWidgetItem(QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(inv.totalAmount, 'f', 0)));
        QTableWidgetItem* dateItem = new QTableWidgetItem(inv.issuedDate.toString("dd/MM/yyyy"));
        
        QString statusVi = "Chưa thanh toán";
        QColor statusColor = QColor("#DD6B20");
        if (inv.status == "PAID") {
            statusVi = "Đã thanh toán";
            statusColor = QColor("#2F855A");
        } else if (inv.status == "CANCELLED") {
            statusVi = "Đã hủy";
            statusColor = QColor("#718096");
        }
        
        QTableWidgetItem* statItem = new QTableWidgetItem(statusVi);
        statItem->setForeground(QBrush(statusColor));
        statItem->setFont(QFont("Arial", -1, QFont::Bold));

        codeItem->setData(Qt::UserRole, inv.invoiceId);
        codeItem->setData(Qt::UserRole + 1, inv.recordId.has_value() ? inv.recordId.value() : -1);
        codeItem->setData(Qt::UserRole + 2, inv.status);

        m_tblInvoices->setItem(row, 0, codeItem);
        m_tblInvoices->setItem(row, 1, recItem);
        m_tblInvoices->setItem(row, 2, patientItem);
        m_tblInvoices->setItem(row, 3, amountItem);
        m_tblInvoices->setItem(row, 4, dateItem);
        m_tblInvoices->setItem(row, 5, statItem);
        row++;
    }
}

void PharmacistDashboardWidget::selectInvoiceRow(int row) {
    if (row < 0) return;

    QTableWidgetItem* codeItem = m_tblInvoices->item(row, 0);
    if (!codeItem) return;

    m_selectedInvoiceId = codeItem->data(Qt::UserRole).toInt();
    int recordId = codeItem->data(Qt::UserRole + 1).toInt();
    QString status = codeItem->data(Qt::UserRole + 2).toString();

    auto optInv = m_billingService->getInvoiceByRecordId(recordId);
    if (optInv.has_value()) {
        auto inv = optInv.value();
        m_lblBillPatientName->setText(QString("<b>Hóa đơn số:</b> %1").arg(inv.invoiceCode));
        m_lblBillDetails->setText(QString("Ngày lập: %1 | Trạng thái: %2").arg(inv.issuedDate.toString("dd/MM/yyyy")).arg(inv.status));
        m_lblBillTotalAmount->setText(QString("Tổng thanh toán: %1 VND").arg(QLocale(QLocale::Vietnamese).toString(inv.totalAmount, 'f', 0)));

        m_tblBillItems->setRowCount(0);
        int itemRow = 0;
        for (const auto& item : inv.items) {
            m_tblBillItems->insertRow(itemRow);
            
            QString desc = item.description;
            if (item.itemType == "CONSULTATION") desc = "Phí khám bệnh";
            
            m_tblBillItems->setItem(itemRow, 0, new QTableWidgetItem(desc));
            m_tblBillItems->setItem(itemRow, 1, new QTableWidgetItem(QString::number(item.quantity)));
            m_tblBillItems->setItem(itemRow, 2, new QTableWidgetItem(QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(item.subtotal, 'f', 0))));
            itemRow++;
        }
    }

    if (status == "UNPAID") {
        m_btnCollectPayment->setEnabled(true);
        m_btnPrintInvoice->setEnabled(true);
    } else if (status == "PAID") {
        m_btnCollectPayment->setEnabled(false);
        m_btnPrintInvoice->setEnabled(true);
    } else {
        m_btnCollectPayment->setEnabled(false);
        m_btnPrintInvoice->setEnabled(false);
    }
}

void PharmacistDashboardWidget::handleCollectPayment() {
    if (m_selectedInvoiceId == -1) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Thu tiền hóa đơn", "Xác nhận đã thu đủ số tiền của hóa đơn này?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        bool success = m_billingService->collectPayment(m_selectedInvoiceId);
        if (success) {
            QMessageBox::information(this, "Thành công", "Đã thu tiền hóa đơn thành công!");
            performInvoiceSearch();
        } else {
            QMessageBox::critical(this, "Lỗi", "Không thể cập nhật trạng thái hóa đơn.");
        }
    }
}

void PharmacistDashboardWidget::loadPendingBilling() {
    m_tblPendingBilling->setRowCount(0);
    
    QString patientIdText = m_txtPendingPatientId->text().trimmed();
    if (patientIdText.isEmpty()) return;
    
    bool ok;
    int patientId = patientIdText.toInt(&ok);
    if (!ok || patientId <= 0) {
        QMessageBox::warning(this, "Lỗi", "Patient ID phải là số nguyên dương.");
        return;
    }

    QList<PrescriptionResultDTO> prescriptions = m_pharmacyService->getPrescriptionsByPatient(patientId);
    int row = 0;
    for (const auto& presc : prescriptions) {
        auto optInvoice = m_billingService->getInvoiceByRecordId(presc.recordId);
        if (!optInvoice.has_value()) {
            m_tblPendingBilling->insertRow(row);
            
            QTableWidgetItem* patientItem = new QTableWidgetItem(presc.patientName);
            QTableWidgetItem* diagItem = new QTableWidgetItem(presc.diagnosis);
            QTableWidgetItem* docItem = new QTableWidgetItem(presc.doctorName);
            QTableWidgetItem* dateItem = new QTableWidgetItem(presc.prescribedAt.toString("dd/MM/yyyy HH:mm"));
            
            double medTotal = m_billingService->calculateMedicationTotal(presc.items);
            QTableWidgetItem* costItem = new QTableWidgetItem(QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(medTotal, 'f', 0)));
            
            patientItem->setData(Qt::UserRole, presc.prescriptionId);
            patientItem->setData(Qt::UserRole + 1, presc.recordId);
            patientItem->setData(Qt::UserRole + 2, presc.doctorId);
            patientItem->setData(Qt::UserRole + 3, presc.patientName);
            patientItem->setData(Qt::UserRole + 4, presc.patientAge);
            patientItem->setData(Qt::UserRole + 5, presc.patientGender);

            m_tblPendingBilling->setItem(row, 0, patientItem);
            m_tblPendingBilling->setItem(row, 1, diagItem);
            m_tblPendingBilling->setItem(row, 2, docItem);
            m_tblPendingBilling->setItem(row, 3, dateItem);
            m_tblPendingBilling->setItem(row, 4, costItem);
            
            QPushButton* btnCreateBill = new QPushButton("Lập Hóa Đơn", m_tblPendingBilling);
            btnCreateBill->setStyleSheet("background-color: #10B981; color: white; border: none; border-radius: 4px; padding: 4px 8px; font-weight: bold;");
            
            connect(btnCreateBill, &QPushButton::clicked, this, [this, row, patientId]() {
                handleCreateInvoiceFromPending(row, patientId);
            });
            m_tblPendingBilling->setCellWidget(row, 5, btnCreateBill);
            row++;
        }
    }

    if (row == 0) {
        QMessageBox::information(this, "Thông báo", "Không tìm thấy đơn thuốc chưa lập hóa đơn nào cho bệnh nhân này.");
    }
}

void PharmacistDashboardWidget::handleCreateInvoiceFromPending(int rowIndex, int patientId) {
    QTableWidgetItem* patientItem = m_tblPendingBilling->item(rowIndex, 0);
    if (!patientItem) return;
    
    int recordId = patientItem->data(Qt::UserRole + 1).toInt();
    
    auto optPresc = m_pharmacyService->getPrescriptionByRecordId(recordId);
    if (!optPresc.has_value()) return;
    
    auto presc = optPresc.value();
    showCreateInvoiceDialog(presc, patientId);
}

void PharmacistDashboardWidget::showCreateInvoiceDialog(const PrescriptionResultDTO& presc, int patientId) {
    QDialog dlg(this);
    dlg.setWindowTitle("Lập hóa đơn lâm sàng");
    dlg.setMinimumSize(450, 350);
    QVBoxLayout* lay = new QVBoxLayout(&dlg);
    
    lay->addWidget(new QLabel(QString("<b>Bệnh nhân:</b> %1").arg(presc.patientName)));
    lay->addWidget(new QLabel(QString("<b>Tuổi:</b> %1 | <b>Giới tính:</b> %2").arg(presc.patientAge).arg(presc.patientGender)));
    lay->addWidget(new QLabel(QString("<b>Bác sĩ kê đơn:</b> %1").arg(presc.doctorName)));
    lay->addWidget(new QLabel(QString("<b>Ngày kê đơn:</b> %1").arg(presc.prescribedAt.toString("dd/MM/yyyy HH:mm"))));
    
    QHBoxLayout* feeLay = new QHBoxLayout();
    feeLay->addWidget(new QLabel("Phí khám bệnh (VND):"));
    QDoubleSpinBox* spinFee = new QDoubleSpinBox(&dlg);
    spinFee->setRange(0, 10000000);
    spinFee->setSingleStep(50000);
    spinFee->setValue(100000);
    spinFee->setDecimals(0);
    feeLay->addWidget(spinFee);
    lay->addLayout(feeLay);
    
    double medTotal = m_billingService->calculateMedicationTotal(presc.items);
    lay->addWidget(new QLabel(QString("<b>Tiền thuốc:</b> %1 VND").arg(QLocale(QLocale::Vietnamese).toString(medTotal, 'f', 0))));
    
    QLabel* lblGrandTotal = new QLabel(&dlg);
    auto updateGrandTotal = [spinFee, medTotal, lblGrandTotal]() {
        double grand = spinFee->value() + medTotal;
        lblGrandTotal->setText(QString("<h3>Tổng thanh toán: <span style='color:#E53E3E;'>%1 VND</span></h3>").arg(QLocale(QLocale::Vietnamese).toString(grand, 'f', 0)));
    };
    connect(spinFee, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateGrandTotal);
    updateGrandTotal();
    lay->addWidget(lblGrandTotal);
    
    QHBoxLayout* btnLay = new QHBoxLayout();
    QPushButton* btnCancel = new QPushButton("Hủy bỏ", &dlg);
    QPushButton* btnSave = new QPushButton("Xác nhận lập hóa đơn", &dlg);
    btnSave->setStyleSheet("background-color: #10B981; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold;");
    btnLay->addWidget(btnCancel);
    btnLay->addWidget(btnSave);
    lay->addLayout(btnLay);
    
    connect(btnCancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, &dlg, [this, &dlg, presc, patientId, spinFee]() {
        bool success = m_billingService->createInvoice(
            patientId, 
            presc.recordId, 
            PatientType::Outpatient, 
            spinFee->value(), 
            presc.items
        );
        if (success) {
            QMessageBox::information(&dlg, "Thành công", "Đã lập hóa đơn thành công!");
            dlg.accept();
        } else {
            QMessageBox::critical(&dlg, "Thất bại", "Không thể lập hóa đơn.");
        }
    });
    
    if (dlg.exec() == QDialog::Accepted) {
        switchPage(3, m_btnBilling);
    }
}

void PharmacistDashboardWidget::handlePrintInvoice() {
    int selectedRow = m_tblInvoices->currentRow();
    if (selectedRow < 0) return;
    int recordId = m_tblInvoices->item(selectedRow, 0)->data(Qt::UserRole + 1).toInt();

    auto optInvoice = m_billingService->getInvoiceByRecordId(recordId);
    if (!optInvoice.has_value()) return;
    auto inv = optInvoice.value();

    QString html = "<html><head><style>"
                   "body { font-family: 'Segoe UI', Arial, sans-serif; margin: 20px; color: #333; }"
                   ".header { text-align: center; margin-bottom: 20px; }"
                   ".title { font-size: 22px; font-weight: bold; color: #1E3A8A; text-transform: uppercase; }"
                   ".info-table { width: 100%; margin-bottom: 20px; border-collapse: collapse; }"
                   ".info-table td { padding: 4px; vertical-align: top; }"
                   ".items-table { width: 100%; border-collapse: collapse; margin-top: 15px; }"
                   ".items-table th, .items-table td { border: 1px solid #ddd; padding: 10px; text-align: left; }"
                   ".items-table th { background-color: #f2f2f2; font-weight: bold; }"
                   ".totals-table { width: 40%; margin-left: 60%; margin-top: 20px; border-collapse: collapse; }"
                   ".totals-table td { padding: 6px; }"
                   ".totals-table .bold { font-weight: bold; font-size: 15px; color: #C53030; }"
                   ".signatures { width: 100%; margin-top: 40px; }"
                   ".signatures td { text-align: center; width: 50%; }"
                   "</style></head><body>"
                   "<div class='header'>"
                   "  <div style='font-size: 16px; font-weight: bold;'>NOVA CARE CLINIC</div>"
                   "  <div style='font-size: 11px;'>Địa chỉ: 123 Đường Nguyễn Văn Cừ, Quận 5, TP. HCM</div>"
                   "  <hr/>"
                   "  <div class='title'>HÓA ĐƠN VIỆN PHÍ</div>"
                   "  <div>Mã hóa đơn: " + inv.invoiceCode + "</div>"
                   "</div>"
                   "<table class='info-table'>"
                   "  <tr>"
                   "    <td><b>Mã bệnh nhân:</b> " + QString::number(inv.patientId) + "</td>"
                   "    <td><b>Ngày phát hành:</b> " + inv.issuedDate.toString("dd/MM/yyyy") + "</td>"
                   "  </tr>"
                   "  <tr>"
                   "    <td><b>Loại hình:</b> Ngoại trú</td>"
                   "    <td><b>Trạng thái:</b> " + (inv.status == "PAID" ? "ĐÃ THANH TOÁN" : "CHƯA THANH TOÁN") + "</td>"
                   "  </tr>"
                   "</table>"
                   "<table class='items-table'>"
                   "  <thead>"
                   "    <tr>"
                   "      <th>STT</th>"
                   "      <th>Nội dung thanh toán</th>"
                   "      <th>Số lượng</th>"
                   "      <th>Đơn giá</th>"
                   "      <th>Thành tiền</th>"
                   "    </tr>"
                   "  </thead>"
                   "  <tbody>";
                   
    int index = 1;
    for (const auto& item : inv.items) {
        QString desc = item.description;
        if (item.itemType == "CONSULTATION") desc = "Phí khám bệnh";
        html += "    <tr>"
                "      <td>" + QString::number(index++) + "</td>"
                "      <td>" + desc + "</td>"
                "      <td>" + QString::number(item.quantity) + "</td>"
                "      <td>" + QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(item.unitPrice, 'f', 0)) + "</td>"
                "      <td>" + QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(item.subtotal, 'f', 0)) + "</td>"
                "    </tr>";
    }
    
    html += "  </tbody>"
            "</table>"
            "<table class='totals-table'>"
            "  <tr>"
            "    <td>Phí khám lâm sàng:</td>"
            "    <td style='text-align:right;'>" + QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(inv.consultationFee, 'f', 0)) + "</td>"
            "  </tr>"
            "  <tr>"
            "    <td>Tiền thuốc/vật tư:</td>"
            "    <td style='text-align:right;'>" + QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(inv.medicationFee, 'f', 0)) + "</td>"
            "  </tr>"
            "  <tr style='border-top:1px solid #ddd;'>"
            "    <td class='bold'>Tổng thanh toán:</td>"
            "    <td class='bold' style='text-align:right;'>" + QString("%1 VND").arg(QLocale(QLocale::Vietnamese).toString(inv.totalAmount, 'f', 0)) + "</td>"
            "  </tr>"
            "</table>"
            "<table class='signatures'>"
            "  <tr>"
            "    <td><b>Bệnh nhân / Người nộp</b><br/><small>(Ký và ghi rõ họ tên)</small></td>"
            "    <td><b>Thu ngân / Dược sĩ</b><br/><small>(Ký và ghi rõ họ tên)</small><br/><br/><br/>" + (m_currentUser ? m_currentUser->getFullName() : "") + "</td>"
            "  </tr>"
            "</table>"
            "</body></html>";

    QDialog previewDlg(this);
    previewDlg.setWindowTitle("In hóa đơn viện phí");
    previewDlg.setMinimumSize(600, 520);
    QVBoxLayout* prevLay = new QVBoxLayout(&previewDlg);
    
    QTextEdit* view = new QTextEdit(&previewDlg);
    view->setHtml(html);
    view->setReadOnly(true);
    prevLay->addWidget(view);
    
    QHBoxLayout* btnLay = new QHBoxLayout();
    QPushButton* btnPrint = new QPushButton("In hóa đơn", &previewDlg);
    btnPrint->setStyleSheet("background-color: #4B94F2; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold;");
    QPushButton* btnClose = new QPushButton("Đóng", &previewDlg);
    btnLay->addWidget(btnPrint);
    btnLay->addWidget(btnClose);
    prevLay->addLayout(btnLay);
    
    connect(btnClose, &QPushButton::clicked, &previewDlg, &QDialog::reject);
    connect(btnPrint, &QPushButton::clicked, &previewDlg, [&previewDlg]() {
        QMessageBox::information(&previewDlg, "Thông báo", "Lệnh in hóa đơn đã được gửi đến máy in mặc định.");
        previewDlg.accept();
    });
    
    previewDlg.exec();
}

// ─────────────────────────────────────────────────────────────────────────────
// PAGE 5: PERIODIC REPORT (BÁO CÁO ĐỊNH KỲ)
// ─────────────────────────────────────────────────────────────────────────────
void PharmacistDashboardWidget::buildReportsPage() {
    m_reportsPage = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_reportsPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QHBoxLayout* rangeLayout = new QHBoxLayout();
    rangeLayout->setSpacing(10);

    m_dateReportFrom = new QDateEdit(m_reportsPage);
    m_dateReportFrom->setCalendarPopup(true);
    m_dateReportFrom->setDate(QDate::currentDate().addDays(-30));

    m_dateReportTo = new QDateEdit(m_reportsPage);
    m_dateReportTo->setCalendarPopup(true);
    m_dateReportTo->setDate(QDate::currentDate());

    QPushButton* btnGenerate = new QPushButton("Tạo Báo Cáo", m_reportsPage);
    btnGenerate->setStyleSheet("background-color: #4B94F2; color: white; padding: 6px 16px; border-radius: 4px; border: none; font-weight: bold;");

    rangeLayout->addWidget(new QLabel("Từ ngày:"));
    rangeLayout->addWidget(m_dateReportFrom);
    rangeLayout->addWidget(new QLabel("Đến ngày:"));
    rangeLayout->addWidget(m_dateReportTo);
    rangeLayout->addWidget(btnGenerate);
    rangeLayout->addStretch();
    mainLayout->addLayout(rangeLayout);

    QHBoxLayout* kpiLay = new QHBoxLayout();
    QFrame* kpi1 = makeCard(m_reportsPage);
    QVBoxLayout* kpi1Lay = new QVBoxLayout(kpi1);
    kpi1Lay->addWidget(new QLabel("Tổng lượng thuốc đã cấp phát (Đơn vị)", kpi1));
    m_lblReportTotalQty = new QLabel("0", kpi1);
    m_lblReportTotalQty->setStyleSheet("font-size: 20px; font-weight: bold; color: #2B6CB0;");
    kpi1Lay->addWidget(m_lblReportTotalQty);
    kpiLay->addWidget(kpi1);
    mainLayout->addLayout(kpiLay);

    QHBoxLayout* workLayout = new QHBoxLayout();
    workLayout->setSpacing(16);

    QFrame* tableCard = makeCard(m_reportsPage);
    QVBoxLayout* tableCardLayout = new QVBoxLayout(tableCard);
    
    QLabel* lblTblTitle = new QLabel("CHI TIẾT TIÊU THỤ THUỐC", tableCard);
    lblTblTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #4A5568;");
    tableCardLayout->addWidget(lblTblTitle);

    m_tblReportUsage = new QTableWidget(tableCard);
    m_tblReportUsage->setColumnCount(3);
    m_tblReportUsage->setHorizontalHeaderLabels({"Tên thuốc", "Tổng lượng sử dụng", "Đơn vị tính"});
    m_tblReportUsage->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tblReportUsage->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableCardLayout->addWidget(m_tblReportUsage);
    workLayout->addWidget(tableCard, 6);

    QFrame* chartCard = makeCard(m_reportsPage);
    QVBoxLayout* chartLayout = new QVBoxLayout(chartCard);
    
    QLabel* lblChartTitle = new QLabel("TOP 5 THUỐC SỬ DỤNG NHIỀU NHẤT", chartCard);
    lblChartTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #4A5568;");
    chartLayout->addWidget(lblChartTitle);

    m_chartView = new QChartView(chartCard);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    chartLayout->addWidget(m_chartView);
    workLayout->addWidget(chartCard, 4);

    mainLayout->addLayout(workLayout, 1);
    m_stackedWidget->addWidget(m_reportsPage);

    connect(btnGenerate, &QPushButton::clicked, this, &PharmacistDashboardWidget::generateReport);
}

void PharmacistDashboardWidget::generateReport() {
    m_tblReportUsage->setRowCount(0);

    QDateTime from(m_dateReportFrom->date(), QTime(0, 0, 0));
    QDateTime to(m_dateReportTo->date(), QTime(23, 59, 59));

    PrescriptionSearchCriteria criteria;
    criteria.fromDate = from;
    criteria.toDate = to;
    criteria.status = "DISPENSED";
    
    QList<PrescriptionResultDTO> prescriptions = m_pharmacyService->searchPrescriptions(criteria);
    QMap<QString, int> usage;
    for (const auto& presc : prescriptions) {
        for (const auto& item : presc.items) {
            usage[item.brandName] += item.quantity;
        }
    }

    int totalQty = 0;
    int row = 0;
    QList<QPair<QString, int>> sortedList;

    for (auto it = usage.begin(); it != usage.end(); ++it) {
        m_tblReportUsage->insertRow(row);
        m_tblReportUsage->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_tblReportUsage->setItem(row, 1, new QTableWidgetItem(QString::number(it.value())));
        m_tblReportUsage->setItem(row, 2, new QTableWidgetItem("Đơn vị"));

        totalQty += it.value();
        sortedList.append({it.key(), it.value()});
        row++;
    }

    m_lblReportTotalQty->setText(QString::number(totalQty));

    std::sort(sortedList.begin(), sortedList.end(), [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
        return a.second > b.second;
    });

    QChart* chart = new QChart();
    chart->setTitle("Tần suất sử dụng thuốc (Đơn vị tính)");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QBarSeries* series = new QBarSeries();
    QBarSet* barSet = new QBarSet("Số lượng");

    QStringList categories;
    int maxVal = 0;
    
    int topCount = qMin(5, sortedList.size());
    for (int i = 0; i < topCount; ++i) {
        *barSet << sortedList[i].second;
        categories << sortedList[i].first;
        if (sortedList[i].second > maxVal) {
            maxVal = sortedList[i].second;
        }
    }

    series->append(barSet);
    chart->addSeries(series);

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, maxVal + (maxVal / 5 + 1));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView->setChart(chart);
}
