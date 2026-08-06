// PatientWidget.cpp
#include "PatientWidget.h"
#include "ui/Doctor/PatientDetailDialog.h"
#include "ui/Doctor/ScheduleFollowUpDialog.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QFrame>
#include <QSpacerItem>
#include <QGraphicsDropShadowEffect>
#include "model/CommonEnums.h"
#include <QComboBox>
#include "../utils/UIValidationUtils.h"
#include "../../service/Validation.h"

PatientWidget::PatientWidget(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void PatientWidget::setupUi() {
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    createListView();

    m_mainLayout->addWidget(m_listViewWidget);
}

void PatientWidget::createListView() {
    m_listViewWidget = new QWidget(this);
    QVBoxLayout* listLayout = new QVBoxLayout(m_listViewWidget);
    listLayout->setContentsMargins(16, 12, 16, 16);
    listLayout->setSpacing(16);

    QScrollArea* scrollArea = new QScrollArea(m_listViewWidget);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { background: #F8FAFC; width: 6px; border-radius: 3px; }"
        "QScrollBar::handle:vertical { background: #CBD5E1; border-radius: 3px; min-height: 30px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    QWidget* scrollContent = new QWidget();
    scrollContent->setStyleSheet("background: transparent;");
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 12, 10);
    scrollLayout->setSpacing(20);
    scrollArea->setWidget(scrollContent);

    QFrame* container = new QFrame(scrollContent);
    container->setStyleSheet("QFrame { background-color: #FFFFFF; border: 1px solid #E2E8F0; border-radius: 14px; }");
    auto* containerShadow = new QGraphicsDropShadowEffect(container);
    containerShadow->setBlurRadius(16); containerShadow->setOffset(0, 2); containerShadow->setColor(QColor(0, 0, 0, 10));
    container->setGraphicsEffect(containerShadow);

    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(24, 20, 24, 24);
    containerLayout->setSpacing(16);

    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(12);

    QLabel *title = new QLabel("DANH SÁCH BỆNH NHÂN", container);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #111827; border: none;");

    m_searchBar = new QLineEdit(container);
    m_searchBar->setPlaceholderText("Tìm kiếm tên, mã bệnh nhân, số điện thoại...");
    m_searchBar->setFixedWidth(380);
    m_searchBar->setStyleSheet(
        "QLineEdit { border: 1px solid #CBD5E1; border-radius: 6px; padding: 8px 12px; font-size: 13px; color: #0F172A; background-color: #FFFFFF; }"
        "QLineEdit:focus { border: 1px solid #2563EB; background-color: #EFF6FF; }"
    );

    QPushButton *btnRefresh = new QPushButton("Tải lại", container);
    btnRefresh->setCursor(Qt::PointingHandCursor);
    btnRefresh->setStyleSheet("QPushButton { background-color: #EFF6FF; color: #2563EB; border: 1px solid #2563EB; font-weight: bold; padding: 6px 14px; border-radius: 6px; } QPushButton:hover { background-color: #DBEAFE; }");
    connect(btnRefresh, &QPushButton::clicked, this, &PatientWidget::loadPatientsData);

    toolbarLayout->addWidget(title);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_searchBar);
    toolbarLayout->addWidget(btnRefresh);
    containerLayout->addLayout(toolbarLayout);

    m_patientTable = new QTableWidget(container);
    m_patientTable->setColumnCount(5);
    m_patientTable->setHorizontalHeaderLabels({"STT", "Mã bệnh nhân", "Tên bệnh nhân", "Số điện thoại", "Thao tác"});
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_patientTable->setAlternatingRowColors(true);
    m_patientTable->setShowGrid(false);
    m_patientTable->setFocusPolicy(Qt::NoFocus);
    m_patientTable->verticalHeader()->setVisible(false);
    m_patientTable->verticalHeader()->setDefaultSectionSize(46);
    
    m_patientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_patientTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_patientTable->horizontalHeader()->resizeSection(0, 50);
    m_patientTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_patientTable->horizontalHeader()->resizeSection(1, 150);
    m_patientTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    m_patientTable->horizontalHeader()->resizeSection(4, 250);
    m_patientTable->horizontalHeader()->setFixedHeight(40);
    m_patientTable->setFocusPolicy(Qt::NoFocus);
    m_patientTable->setStyleSheet(
        "QTableWidget { background-color: #FFFFFF; border: none; gridline-color: transparent; font-size: 13px; color: #334155; outline: none; }"
        "QTableWidget::item { background: transparent; padding: 12px 8px; border-bottom: 1px solid #F1F5F9; outline: none; }"
        "QTableWidget::item:focus { outline: none; border: none; }"
        "QTableWidget::item:selected { background-color: #EFF6FF; color: #2563EB; font-weight: 600; }"
        "QHeaderView::section { background-color: #F8FAFC; color: #475569; font-weight: bold; font-size: 12px; border: none; border-bottom: 2px solid #E2E8F0; padding: 4px; }"
    );

    containerLayout->addWidget(m_patientTable);

    // Pagination Layout
    QHBoxLayout *paginationLayout = new QHBoxLayout();
    paginationLayout->setContentsMargins(15, 10, 15, 10);

    m_btnPrevPage = new QPushButton("Trang trước", container);
    m_btnPrevPage->setCursor(Qt::PointingHandCursor);
    m_btnPrevPage->setStyleSheet(
        "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: 6px; padding: 6px 12px; font-weight: bold; border: none; min-height: 32px; }"
        "QPushButton:hover { background-color: #CBD5E1; }"
        "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }"
    );

    m_lblPageInfo = new QLabel(QString("%1 / %2").arg(m_currentPage).arg(m_totalPages), container);
    m_lblPageInfo->setStyleSheet("background: transparent; border: none; font-size: 13px; font-weight: bold; color: #475569;");

    m_btnNextPage = new QPushButton("Trang sau", container);
    m_btnNextPage->setCursor(Qt::PointingHandCursor);
    m_btnNextPage->setStyleSheet(
        "QPushButton { background-color: #E2E8F0; color: #1E293B; border-radius: 6px; padding: 6px 12px; font-weight: bold; border: none; min-height: 32px; }"
        "QPushButton:hover { background-color: #CBD5E1; }"
        "QPushButton:disabled { background-color: #F1F5F9; color: #94A3B8; }"
    );

    paginationLayout->addWidget(m_btnPrevPage);
    paginationLayout->addStretch();
    paginationLayout->addWidget(m_lblPageInfo);
    paginationLayout->addStretch();
    paginationLayout->addWidget(m_btnNextPage);

    containerLayout->addLayout(paginationLayout);

    connect(m_btnPrevPage, &QPushButton::clicked, this, &PatientWidget::handlePrevPage);
    connect(m_btnNextPage, &QPushButton::clicked, this, &PatientWidget::handleNextPage);

    scrollLayout->addWidget(container);
    listLayout->addWidget(scrollArea, 1);

    connect(m_searchBar, &QLineEdit::textChanged, this, [this](const QString &) {
        m_currentPage = 1;
        loadPatientsData();
    });
}

void PatientWidget::setPatientService(std::shared_ptr<PatientService> patientService) {
    m_patientService = patientService;
    loadPatientsData();
}

void PatientWidget::loadPatientsData() {
    if (!m_patientService || !m_patientTable) return;
    
    PatientSearchCriteria criteria;
    if (m_searchBar) {
        criteria.searchKey = m_searchBar->text().trimmed();
    }

    criteria.page = m_currentPage;
    criteria.pageSize = 10;

    auto pagedResult = m_patientService->searchPatientsPaged(criteria);
    m_patientList = pagedResult.items;
    m_totalPages = pagedResult.totalPages();
    if (m_totalPages == 0) m_totalPages = 1;
    updatePaginationInfo();
    
    m_patientTable->setRowCount(m_patientList.size());
    for (int i = 0; i < m_patientList.size(); ++i) {
        const auto& p = m_patientList[i];
        m_patientTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        m_patientTable->setItem(i, 1, new QTableWidgetItem(p.patientCode));
        m_patientTable->setItem(i, 2, new QTableWidgetItem(p.fullName));
        m_patientTable->setItem(i, 3, new QTableWidgetItem(p.phone));

        QWidget* actWidget = new QWidget(m_patientTable);
        QHBoxLayout* actLayout = new QHBoxLayout(actWidget);
        actLayout->setContentsMargins(4, 3, 4, 3);
        actLayout->setSpacing(12);

        QPushButton* btnInfo = new QPushButton("Xem thông tin", actWidget);
        btnInfo->setCursor(Qt::PointingHandCursor);
        btnInfo->setMinimumHeight(38);
        btnInfo->setStyleSheet(
            "QPushButton { background-color: #0284C7; color: white; font-size: 12px; font-weight: bold; font-family: 'Segoe UI'; border-radius: 6px; padding: 6px 14px; border: none; }"
            "QPushButton:hover { background-color: #0369A1; }"
        );

        QPushButton* btnFollowUp = new QPushButton("Tái khám", actWidget);
        btnFollowUp->setCursor(Qt::PointingHandCursor);
        btnFollowUp->setMinimumHeight(38);
        btnFollowUp->setStyleSheet(
            "QPushButton { background-color: #D97706; color: white; font-size: 12px; font-weight: bold; font-family: 'Segoe UI'; border-radius: 6px; padding: 6px 14px; border: none; }"
            "QPushButton:hover { background-color: #B45309; }"
        );

        actLayout->addWidget(btnInfo);
        actLayout->addWidget(btnFollowUp);
        actLayout->setAlignment(Qt::AlignCenter);

        m_patientTable->setRowHeight(i, 64);

        int patId = p.patientId;
        QString patName = p.fullName;

        connect(btnInfo, &QPushButton::clicked, this, [this, i]() {
            handlePatientSelected(i, 0);
        });

        connect(btnFollowUp, &QPushButton::clicked, this, [this, patId, patName]() {
            if (!m_appointmentService) {
                QMessageBox::warning(this, "Lỗi", "Dịch vụ cuộc hẹn chưa được khởi tạo.");
                return;
            }
            ScheduleFollowUpDialog dialog(patId, patName, m_doctorId, m_appointmentService, this);
            dialog.exec();
        });

        m_patientTable->setCellWidget(i, 4, actWidget);
    }
}

void PatientWidget::handlePatientSelected(int row, int /*column*/) {
    if (row >= 0 && row < m_patientList.size()) {
        const auto summary = m_patientList[row];
        PatientDetailDialog dialog(m_patientService, m_pharmacyService, m_medicalRecordService, summary.patientId, this);
        dialog.exec();
    }
}

void PatientWidget::updatePaginationInfo() {
    if (!m_lblPageInfo || !m_btnPrevPage || !m_btnNextPage) return;

    m_lblPageInfo->setText(QString("%1 / %2").arg(m_currentPage).arg(m_totalPages));
    m_btnPrevPage->setEnabled(m_currentPage > 1);
    m_btnNextPage->setEnabled(m_currentPage < m_totalPages);
}

void PatientWidget::handlePrevPage() {
    if (m_currentPage > 1) {
        m_currentPage--;
        loadPatientsData();
    }
}

void PatientWidget::handleNextPage() {
    if (m_currentPage < m_totalPages) {
        m_currentPage++;
        loadPatientsData();
    }
}

PatientWidget::~PatientWidget() {}
