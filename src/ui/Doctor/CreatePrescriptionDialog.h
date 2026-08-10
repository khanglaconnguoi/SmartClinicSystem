#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include "dto/PrescriptionDTOs.h"
#include "dto/MedicationDTOs.h"
#include "service/PharmacyService.h"
#include "service/PatientService.h"
#include <QMessageBox>
#include <QLabel>
#include <memory>

class CreatePrescriptionDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreatePrescriptionDialog(std::shared_ptr<PharmacyService> pharmacyService = nullptr,
                                     std::shared_ptr<PatientService> patientService = nullptr,
                                     int patientId = 0,
                                     QWidget *parent = nullptr);
    void setPharmacyService(std::shared_ptr<PharmacyService> pharmacyService);
    void setPatientService(std::shared_ptr<PatientService> patientService, int patientId);
    PrescriptionInputDTO getPrescriptionInput() const;

    void setRecordId(const QString &recordId);
    void setDoctorId(const QString &doctorId);
    void setDoctorName(const QString &doctorName);

private slots:
    void onSearchClicked();
    void onPrevPageClicked();
    void onNextPageClicked();
    void onSearchTableSelectionChanged();
    void onAddSelectedClicked();
    void onRemoveMedicineClicked();
    void onSaveClicked();
    void onExportPdfClicked();
    void onPrescriptionCellChanged(int row, int col);

private:
    void setupUI();
    void setupStyleSheets();
    void addMedicineRow(int medicationId, const QString &name, double price, int qty, const QString &dosage, const QString &freq, int days, const QString &note, int maxStock);
    void updateSearchResults();
    void updateSelectedDetails();

    std::shared_ptr<PharmacyService> m_pharmacyService;
    std::shared_ptr<PatientService>  m_patientService;
    int m_patientId = 0;

    int m_searchPage = 1;
    int m_searchTotalPages = 1;
    const int m_searchPageSize = 8;
    QList<MedicationSummaryDTO> m_currentSearchResults;
    std::optional<MedicationSummaryDTO> m_selectedMedication;

    // UI Elements - Info & Notes
    QLineEdit   *txtRecordId;
    QLineEdit   *txtDoctorId;
    QComboBox   *cboDoctorName;
    QTextEdit   *txtGeneralNotes;

    // UI Elements - Left Search Panel
    QLineEdit    *txtSearchKeyword;
    QTableWidget *tblSearchResults;
    QPushButton  *btnPrevPage;
    QPushButton  *btnNextPage;
    QLabel       *lblPageIndicator;
    QLabel       *lblMedicationDetails;
    QPushButton  *btnAddSelected;

    // UI Elements - Right Prescription Panel
    QTableWidget *tblPrescription;

    // Action Buttons
    QPushButton  *btnSave;
    QPushButton  *btnExportPdf;
    QPushButton  *btnCancel;
};