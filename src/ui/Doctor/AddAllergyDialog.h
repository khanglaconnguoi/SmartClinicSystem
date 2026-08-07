#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <memory>

#include "service/PharmacyService.h"
#include "dto/PatientDTOs.h"
#include "dto/MedicationDTOs.h"
#include "model/CommonEnums.h"

// Dialog tìm kiếm Hoạt chất dị ứng với tìm kiếm tự động, phân trang & điền mức độ/ghi chú bên dưới
class AddActiveIngredientAllergyDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddActiveIngredientAllergyDialog(std::shared_ptr<PharmacyService> pharmacyService, QWidget* parent = nullptr);

    AllergyInputDTO getAllergyDTO() const { return m_selectedAllergy; }

signals:
    void allergyAdded(const AllergyInputDTO& dto);

private slots:
    void performSearch();
    void goToPreviousPage();
    void goToNextPage();
    void handleAddSelected();

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
    QComboBox* m_cbSeverity;
    QLineEdit* m_txtNotes;
    QPushButton* m_btnAdd;

    AllergyInputDTO m_selectedAllergy;
    int m_currentPage = 1;
    int m_pageSize = 7;
    int m_totalPages = 1;
};

// Dialog nhập dị ứng khác (Free-text allergen name)
class AddOtherAllergyDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddOtherAllergyDialog(QWidget* parent = nullptr);

    AllergyInputDTO getAllergyDTO() const { return m_selectedAllergy; }

signals:
    void allergyAdded(const AllergyInputDTO& dto);

private slots:
    void handleConfirm();

private:
    QLineEdit* m_txtName;
    QComboBox* m_cbSeverity;
    QLineEdit* m_txtNotes;
    AllergyInputDTO m_selectedAllergy;
};
