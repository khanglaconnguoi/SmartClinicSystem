#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QFrame>
#include <memory>
#include "../../service/StaffService.h"

class SystemUser;

class ManageNursesWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManageNursesWidget(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~ManageNursesWidget() override = default;

    void loadNursesList();

private slots:
    void showAddNurseDialog();
    void showEditNurseDialog(std::shared_ptr<SystemUser> nurse);

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    QTableWidget* m_tblNurses;
};
