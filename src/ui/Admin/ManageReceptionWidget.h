#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QFrame>
#include <memory>
#include "../../service/StaffService.h"

class SystemUser;

class ManageReceptionWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManageReceptionWidget(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~ManageReceptionWidget() override = default;

    void loadReceptionList();

private slots:
    void showAddReceptionDialog();
    void showEditReceptionDialog(std::shared_ptr<SystemUser> reception);

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    QTableWidget* m_tblReception;
};
