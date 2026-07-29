#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include "dto/PrescriptionDTOs.h"
#include "repository/PrescriptionRepository.h"
#include <QMessageBox>
#include <memory>
class PharmacyService;

class CreatePrescriptionDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreatePrescriptionDialog(std::shared_ptr<PharmacyService> pharmacyService = nullptr, QWidget *parent = nullptr);
    void setPharmacyService(std::shared_ptr<PharmacyService> pharmacyService);
    PrescriptionInputDTO getPrescriptionInput() const;

    void setRecordId(const QString &recordId);
    void setDoctorId(const QString &doctorId);
    void setDoctorName(const QString &doctorName);

private slots:
    void onAddMedicineClicked();
    void onRemoveMedicineClicked();
    void onSaveClicked();
    void onExportPdfClicked();

private:
    void setupUI();
    void setupStyleSheets();
    void addMedicineRow(const QString &name, double price, int qty, const QString &dosage, const QString &freq, int days, const QString &note);

    std::shared_ptr<PharmacyService> m_pharmacyService;

    QLineEdit   *txtRecordId;
    QLineEdit   *txtDoctorId;
    QComboBox   *cboDoctorName;
    QTextEdit   *txtGeneralNotes;

    QTableWidget *tblPrescription;
    QPushButton  *btnAddMedicine;

    QPushButton  *btnSave;
    QPushButton  *btnExportPdf;
    QPushButton  *btnCancel;
};