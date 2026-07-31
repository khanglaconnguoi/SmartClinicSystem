#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QFrame>
#include <memory>
#include "../../service/StaffService.h"
#include "../../model/Pharmacist.h"

class ManagePharmacistsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManagePharmacistsWidget(std::shared_ptr<StaffService> staffService, QWidget* parent = nullptr);
    ~ManagePharmacistsWidget() override = default;

    void loadPharmacistsList();

private slots:
    void showAddPharmacistDialog();
    void showEditPharmacistDialog(std::shared_ptr<Pharmacist> doc);

private:
    void buildUI();
    QFrame* makeCard(QWidget* parent);

    std::shared_ptr<StaffService> m_staffService;
    QTableWidget* m_tblPharmacists;
};
