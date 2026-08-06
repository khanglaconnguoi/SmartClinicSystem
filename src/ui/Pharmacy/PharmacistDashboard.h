#pragma once

#include <QCalendarWidget>
#include <QDateEdit>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QLineEdit>
#include <memory>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include "model/IAuthenticatable.h"
#include "service/MedicalRecordService.h"
#include "service/PharmacyService.h"
#include "service/StaffService.h"
#include "service/BillingService.h"
#include "service/PatientService.h"
#include "ui/BaseDashboard.h"

class PharmacistDashboardWidget : public BaseDashboardWidget {
    Q_OBJECT

public:
    explicit PharmacistDashboardWidget(
        std::shared_ptr<IAuthenticatable> user = nullptr, 
        std::shared_ptr<StaffService> staffService = nullptr, 
        std::shared_ptr<MedicalRecordService> medicalRecordService = nullptr, 
        std::shared_ptr<PharmacyService> pharmacyService = nullptr, 
        std::shared_ptr<BillingService> billingService = nullptr,
        std::shared_ptr<PatientService> patientService = nullptr,
        QWidget *parent = nullptr
    );
    virtual ~PharmacistDashboardWidget() override = default;

protected:
    virtual void fillDashboardData() override;

private slots:
    void performInventorySearch();
    void prevInventoryPage();
    void nextInventoryPage();
    
    void performPrescriptionSearch();
    void prevPrescriptionPage();
    void nextPrescriptionPage();
    void selectPrescriptionRow(int row);
    void handleDispensePrescription();
    void handleCancelPrescription();
    void handlePrintReceipt();

    void performInvoiceSearch();
    void selectInvoiceRow(int row);
    void handleCollectPayment();
    void handlePrintInvoice();
    void loadPendingBilling();
    void handleCreateInvoiceFromPending(int rowIndex, int patientId);
    void showCreateInvoiceDialog(const PrescriptionResultDTO& presc, int patientId);

    void generateReport();

private:
    std::shared_ptr<MedicalRecordService> m_medicalRecordService;
    std::shared_ptr<PharmacyService> m_pharmacyService;
    std::shared_ptr<BillingService> m_billingService;
    std::shared_ptr<PatientService> m_patientService;

    QStackedWidget* m_stackedWidget = nullptr;

    // Sidebar navigation buttons
    QPushButton* m_btnDash = nullptr;
    QPushButton* m_btnInventory = nullptr;
    QPushButton* m_btnDispensing = nullptr;
    QPushButton* m_btnBilling = nullptr;

    // Pages
    QWidget* m_overviewPage = nullptr;
    QWidget* m_inventoryPage = nullptr;
    QWidget* m_dispensingPage = nullptr;
    QWidget* m_billingPage = nullptr;

    // ────────────────────────────────────────────────────────────────
    // Sub-components: Overview & Reports (Merged)
    // ────────────────────────────────────────────────────────────────
    QLabel* m_lblStatTotalMeds = nullptr;
    QLabel* m_lblStatInventoryValue = nullptr;
    QLabel* m_lblStatLowStock = nullptr;
    QLabel* m_lblStatOutOfStock = nullptr;
    QLabel* m_lblStatExpiring = nullptr;
    QLabel* m_lblStatPendingPresc = nullptr;
    QLabel* m_lblStatDispensedToday = nullptr;

    // ────────────────────────────────────────────────────────────────
    // Sub-components: Inventory (CRUD)
    // ────────────────────────────────────────────────────────────────
    QLineEdit* m_txtInvKeyword = nullptr;
    QComboBox* m_cbInvCategory = nullptr;
    QComboBox* m_cbInvStatus = nullptr;
    QTableWidget* m_tblInventory = nullptr;
    QPushButton* m_btnInvPrev = nullptr;
    QPushButton* m_btnInvNext = nullptr;
    QLabel* m_lblInvPageInfo = nullptr;
    int m_invCurrentPage = 1;
    int m_invPageSize = 10;
    int m_invTotalPages = 1;

    // ────────────────────────────────────────────────────────────────
    // Sub-components: Dispensing
    // ────────────────────────────────────────────────────────────────
    QLineEdit* m_txtPrescKeyword = nullptr;
    QComboBox* m_cbPrescStatus = nullptr;
    QTableWidget* m_tblPrescriptions = nullptr;
    QTableWidget* m_tblOverviewAlerts = nullptr; // Cảnh báo tồn kho khẩn cấp
    QPushButton* m_btnPrescPrev = nullptr;
    QPushButton* m_btnPrescNext = nullptr;
    QLabel* m_lblPrescPageInfo = nullptr;
    int m_prescCurrentPage = 1;
    int m_prescPageSize = 10;
    int m_prescTotalPages = 1;

    // Dispensing details panel (Right side)
    QLabel* m_lblDetPatientName = nullptr;
    QLabel* m_lblDetPatientInfo = nullptr;
    QLabel* m_lblDetDiagnosis = nullptr;
    QTableWidget* m_tblDetItems = nullptr;
    QLabel* m_lblDetTotalCost = nullptr;
    QTextEdit* m_txtDetNotes = nullptr;
    QPushButton* m_btnDispenseAction = nullptr;
    QPushButton* m_btnCancelPrescAction = nullptr;
    QPushButton* m_btnPrintReceiptAction = nullptr;
    int m_selectedPrescriptionId = -1;
    int m_selectedPrescRecordId = -1;
    int m_selectedPrescPatientId = -1;

    // ────────────────────────────────────────────────────────────────
    // Sub-components: Billing & Collection
    // ────────────────────────────────────────────────────────────────
    QTabWidget* m_tabBillingContainer = nullptr;
    
    // Tab 1: Created Invoices
    QLineEdit* m_txtBillKeyword = nullptr;
    QComboBox* m_cbBillStatus = nullptr;
    QTableWidget* m_tblInvoices = nullptr;
    QLabel* m_lblBillPatientName = nullptr;
    QLabel* m_lblBillDetails = nullptr;
    QLabel* m_lblBillInsurancePercent = nullptr; // New UI label to show insurance coverage %
    QTableWidget* m_tblBillItems = nullptr;
    QLabel* m_lblBillTotalAmount = nullptr;
    QPushButton* m_btnCollectPayment = nullptr;
    QPushButton* m_btnPrintInvoice = nullptr;
    int m_selectedInvoiceId = -1;
    int m_selectedInvoiceRecordId = -1;       // Cached record ID
    int m_selectedInvoicePatientId = -1;      // Cached patient ID
    double m_selectedInvoiceTotalAmount = 0.0; // Cached original total amount

    // Tab 2: Pending Invoices (Unbilled prescriptions)
    QLineEdit* m_txtPendingPatientId = nullptr;
    QPushButton* m_btnSearchPending = nullptr;
    QTableWidget* m_tblPendingBilling = nullptr;

    // ────────────────────────────────────────────────────────────────
    // Sub-components: Reports
    // ────────────────────────────────────────────────────────────────
    QDateEdit* m_dateReportFrom = nullptr;
    QDateEdit* m_dateReportTo = nullptr;
    QTableWidget* m_tblReportUsage = nullptr;
    QLabel* m_lblReportTotalQty = nullptr;
    QLabel* m_lblReportTotalValue = nullptr;
    QChartView* m_chartView = nullptr;
    QBarSeries* m_barSeries = nullptr;

    // Helper functions
    void buildSidebar();
    void buildOverviewPage();
    void buildInventoryPage();
    void buildDispensingPage();
    void buildBillingPage();
    
    QFrame* makeCard(QWidget* parent = nullptr);
    void switchPage(int index, QPushButton* activeBtn);
    void refreshOverviewStats();
    void refreshOverviewAlerts();
};
