#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QFrame>
#include <memory>
#include "service/StaffService.h"
#include "model/SystemUser.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class ManageReceptionWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManageReceptionWidget(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~ManageReceptionWidget() override = default;

    void loadReceptionList();

private slots:
    void showAddReceptionDialog();
    void showEditReceptionDialog(std::shared_ptr<SystemUser> reception);
    void handleFilterChanged();
    void handleResetFilters();
    void handlePrevPage();
    void handleNextPage();

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    QTableWidget* m_tblReception;

    // Filter controls
    QLineEdit* m_txtSearchKey;
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
