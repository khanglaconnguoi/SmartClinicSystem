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

class ManageNursesWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManageNursesWidget(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~ManageNursesWidget() override = default;

    void loadNursesList();

private slots:
    void showAddNurseDialog();
    void showEditNurseDialog(std::shared_ptr<SystemUser> nurse);
    void handleFilterChanged();
    void handleResetFilters();
    void handlePrevPage();
    void handleNextPage();

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    QTableWidget* m_tblNurses;

    // Filter controls
    QLineEdit* m_txtSearchKey;
    QComboBox* m_cbLevelFilter;
    QComboBox* m_cbDepartmentFilter;
    QComboBox* m_cbRoomFilter;
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
