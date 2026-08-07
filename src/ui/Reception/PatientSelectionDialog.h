#pragma once

#include <QDialog>
#include <QList>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include "dto/PatientDTOs.h"

class PatientSelectionDialog : public QDialog {
  Q_OBJECT

public:
  explicit PatientSelectionDialog(const QList<PatientShortDTO> &patients, QWidget *parent = nullptr);
  ~PatientSelectionDialog() override = default;

  PatientShortDTO getSelectedPatient() const;

private slots:
  void onSelectClicked();
  void onRowDoubleClicked(int row, int column);

private:
  void setupUi();
  void populateTable();

  QList<PatientShortDTO> m_patients;
  PatientShortDTO m_selectedPatient;

  QTableWidget *m_tablePatients = nullptr;
  QPushButton *m_btnSelect = nullptr;
  QPushButton *m_btnCancel = nullptr;
};
