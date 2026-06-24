/**
 * @file    PatientView.cpp
 * @brief   Implementation cho PatientView — trang quản lý bệnh nhân.
 */
#include "PatientView.h"
#include "PatientTableModel.h"
#include "PatientFormDialog.h"
#include "service/PatientService.h"

#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

PatientView::PatientView(PatientService* service,
                         QWidget* parent)
    : QWidget(parent)
    , m_service(service) {
    setupUi();
    refreshTable();
}

void PatientView::setupUi() {
    // --- Thanh tìm kiếm ---
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(tr("Tìm kiếm theo tên..."));

    m_btnSearch = new QPushButton(tr("Tìm"), this);

    auto* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(m_searchInput);
    searchLayout->addWidget(m_btnSearch);

    // --- Table ---
    m_model = new PatientTableModel(this);
    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSortingEnabled(false);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->hide();

    // --- Các nút hành động ---
    m_btnAdd = new QPushButton(tr("Thêm"), this);
    m_btnEdit = new QPushButton(tr("Sửa"), this);
    m_btnDelete = new QPushButton(tr("Xóa"), this);
    m_btnAdvanceState = new QPushButton(tr("Chuyển trạng thái \u25B6"), this);

    auto* actionLayout = new QHBoxLayout;
    actionLayout->addWidget(m_btnAdd);
    actionLayout->addWidget(m_btnEdit);
    actionLayout->addWidget(m_btnDelete);
    actionLayout->addWidget(m_btnAdvanceState);
    actionLayout->addStretch();

    // --- Main layout ---
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(m_tableView);
    mainLayout->addLayout(actionLayout);

    // --- Kết nối signal-slot (cú pháp con trỏ hàm, Convention §4.1) ---
    connect(m_btnAdd, &QPushButton::clicked,
            this, &PatientView::handleAddClicked);
    connect(m_btnEdit, &QPushButton::clicked,
            this, &PatientView::handleEditClicked);
    connect(m_btnDelete, &QPushButton::clicked,
            this, &PatientView::handleDeleteClicked);
    connect(m_btnSearch, &QPushButton::clicked,
            this, &PatientView::handleSearchClicked);
    connect(m_searchInput, &QLineEdit::textChanged,
            this, &PatientView::handleSearchTextChanged);
    connect(m_btnAdvanceState, &QPushButton::clicked,
            this, &PatientView::handleAdvanceStateClicked);
}

void PatientView::refreshTable() {
    auto patients = m_service->getAllPatients();
    m_model->setPatients(patients);

    // Tự động resize cột theo nội dung
    m_tableView->resizeColumnsToContents();
}

int PatientView::selectedRow() const {
    auto indexes = m_tableView->selectionModel()->selectedRows();
    if (indexes.isEmpty()) {
        return -1;
    }
    return indexes.first().row();
}

// --- Slot handlers ---

void PatientView::handleAddClicked() {
    PatientFormDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        auto patient = dialog.getPatient();
        if (m_service->addPatient(patient)) {
            refreshTable();
            qDebug() << "Patient added successfully.";
        } else {
            QMessageBox::warning(this, tr("Lỗi"),
                                 tr("Không thể thêm bệnh nhân."));
        }
    }
}

void PatientView::handleEditClicked() {
    int row = selectedRow();
    if (row < 0) {
        QMessageBox::information(this, tr("Thông báo"),
                                 tr("Vui lòng chọn một bệnh nhân."));
        return;
    }

    auto patient = m_model->patientAt(row);
    if (!patient.has_value()) {
        return;
    }

    PatientFormDialog dialog(patient.value(), this);

    if (dialog.exec() == QDialog::Accepted) {
        auto updated = dialog.getPatient();
        if (m_service->updatePatient(updated)) {
            refreshTable();
            qDebug() << "Patient updated successfully.";
        } else {
            QMessageBox::warning(this, tr("Lỗi"),
                                 tr("Không thể cập nhật bệnh nhân."));
        }
    }
}

void PatientView::handleDeleteClicked() {
    int row = selectedRow();
    if (row < 0) {
        QMessageBox::information(this, tr("Thông báo"),
                                 tr("Vui lòng chọn một bệnh nhân."));
        return;
    }

    auto patient = m_model->patientAt(row);
    if (!patient.has_value()) {
        return;
    }

    auto reply = QMessageBox::question(
        this, tr("Xác nhận xóa"),
        tr("Bạn có chắc muốn xóa bệnh nhân \"%1\"?")
            .arg(patient->fullName()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (m_service->deletePatient(patient->id())) {
            refreshTable();
            qDebug() << "Patient deleted successfully.";
        } else {
            QMessageBox::warning(this, tr("Lỗi"),
                                 tr("Không thể xóa bệnh nhân."));
        }
    }
}

void PatientView::handleSearchClicked() {
    QString keyword = m_searchInput->text().trimmed();
    auto patients = m_service->searchPatients(keyword);
    m_model->setPatients(patients);
    m_tableView->resizeColumnsToContents();
}

void PatientView::handleSearchTextChanged(const QString& text) {
    // Nếu ô tìm kiếm trống, load lại toàn bộ danh sách
    if (text.trimmed().isEmpty()) {
        refreshTable();
    }
}

void PatientView::handleAdvanceStateClicked() {
    int row = selectedRow();
    if (row < 0) {
        QMessageBox::information(this, tr("Thông báo"),
                                 tr("Vui lòng chọn một bệnh nhân."));
        return;
    }

    auto patient = m_model->patientAt(row);
    if (!patient.has_value()) {
        return;
    }

    if (!patient->canAdvance()) {
        QMessageBox::information(
            this, tr("Thông báo"),
            tr("Bệnh nhân \"%1\" đang ở trạng thái \"%2\" — "
               "đây là trạng thái cuối cùng.")
                .arg(patient->fullName())
                .arg(patient->stateName()));
        return;
    }

    // Hiển thị xác nhận chuyển trạng thái
    auto reply = QMessageBox::question(
        this, tr("Xác nhận chuyển trạng thái"),
        tr("Chuyển bệnh nhân \"%1\" từ trạng thái \"%2\" sang bước tiếp theo?")
            .arg(patient->fullName())
            .arg(patient->stateName()),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        if (m_service->advancePatientState(patient->id())) {
            refreshTable();
            qDebug() << "Patient state advanced successfully.";
        } else {
            QMessageBox::warning(this, tr("Lỗi"),
                                 tr("Không thể chuyển trạng thái bệnh nhân."));
        }
    }
}
