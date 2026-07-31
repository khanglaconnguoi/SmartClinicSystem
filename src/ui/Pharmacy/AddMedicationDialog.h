#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollArea>
#include <QGroupBox>
#include <QCheckBox>
#include <QList>
#include <memory>

#include "service/PharmacyService.h"
#include "dto/MedicationDTOs.h"

class ActiveIngredientSearchDialog : public QDialog {
    Q_OBJECT
public:
    explicit ActiveIngredientSearchDialog(std::shared_ptr<PharmacyService> pharmacyService, QWidget* parent = nullptr);

signals:
    void ingredientSelected(int id, const QString& name);

private slots:
    void performSearch();
    void goToPreviousPage();
    void goToNextPage();
    void handleRowClicked(int row);

private:
    void loadPageData();
    void updatePaginationButtons();
    void setupUI();

    std::shared_ptr<PharmacyService> m_pharmacyService;
    QLineEdit* m_searchBar;
    QTableWidget* m_resultsTable;
    QPushButton* m_btnPrev;
    QPushButton* m_btnNext;
    QLabel* m_lblPageInfo;
    QPushButton* m_btnClose;

    int m_currentPage;
    int m_pageSize;
    int m_totalPages;
};

class AddMedicationDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddMedicationDialog(std::shared_ptr<PharmacyService> pharmacyService, int medicationId = -1, QWidget* parent = nullptr);

private slots:
    void openIngredientSearch();
    void addIngredientToTable(int id, const QString& name);
    void handleSave();
    void removeSelectedIngredient();

private:
    void loadCommonEnums();
    void setupUI();
    void loadMedicationData();

    std::shared_ptr<PharmacyService> m_pharmacyService;
    int m_medicationId;

    QLineEdit* m_txtBrandName;
    QHBoxLayout* m_categoriesLayout;
    QList<QCheckBox*> m_categoryCheckBoxes;
    QComboBox* m_cbUnit;
    QDoubleSpinBox* m_spinUnitPrice;
    QSpinBox* m_spinStockQuantity;
    QSpinBox* m_spinMinimumStock;
    QSpinBox* m_spinReorderThreshold;
    QDateEdit* m_dateExpiry;
    QLineEdit* m_txtManufacturer;
    QTextEdit* m_txtDescription;
    QTableWidget* m_ingredientsTable;
    QPushButton* m_btnAddIngredient;
    QPushButton* m_btnRemoveIngredient;
    QPushButton* m_btnSave;
    QPushButton* m_btnCancel;
};