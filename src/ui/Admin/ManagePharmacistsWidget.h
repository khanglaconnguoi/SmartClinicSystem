#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QFrame>
#include <memory>
#include "../../service/StaffService.h"
#include "../../model/Pharmacist.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class ManagePharmacistsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManagePharmacistsWidget(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~ManagePharmacistsWidget() override = default;

    void loadPharmacistsList();

private slots:
    void showAddPharmacistDialog();
    void showEditPharmacistDialog(std::shared_ptr<Pharmacist> doc);
    void handleFilterChanged();
    void handleResetFilters();
    void handlePrevPage();
    void handleNextPage();

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    QTableWidget* m_tblPharmacists;

    // Filter controls
    QLineEdit* m_txtSearchKey;
    QLineEdit* m_txtSectionFilter;
    QComboBox* m_cbDepartmentFilter;
    QComboBox* m_cbShiftFilter;
    QComboBox* m_cbStatusFilter;
    QPushButton* m_btnResetFilters;

    // Pagination controls
    int m_currentPage = 1;
    int m_totalPages = 1;
    QPushButton* m_btnPrevPage;
    QPushButton* m_btnNextPage;
    QLabel* m_lblPageInfo;
};
