#include "PatientSelectionDialog.h"
#include "../../model/CommonEnums.h"
#include <QMessageBox>

PatientSelectionDialog::PatientSelectionDialog(const QList<PatientShortDTO> &patients, QWidget *parent)
    : QDialog(parent), m_patients(patients) {
  setupUi();
  populateTable();
}

void PatientSelectionDialog::setupUi() {
  setWindowTitle("Chọn Bệnh Nhân");
  setMinimumSize(600, 380);
  setStyleSheet("QDialog { background-color: #FFFFFF; }");

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(20, 20, 20, 20);
  mainLayout->setSpacing(15);

  QLabel *lblTitle = new QLabel(
      QString("Tìm thấy %1 bệnh nhân khớp với thông tin. Vui lòng chọn bệnh nhân:").arg(m_patients.size()), this);
  lblTitle->setStyleSheet("font-size: 15px; font-weight: bold; color: #1E293B;");
  mainLayout->addWidget(lblTitle);

  m_tablePatients = new QTableWidget(this);
  m_tablePatients->setColumnCount(4);
  m_tablePatients->setHorizontalHeaderLabels({"Mã BN", "Họ và tên", "Ngày sinh", "Giới tính"});
  m_tablePatients->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_tablePatients->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_tablePatients->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tablePatients->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tablePatients->setFocusPolicy(Qt::NoFocus);
  m_tablePatients->verticalHeader()->setVisible(false);
  m_tablePatients->setStyleSheet(
      "QTableWidget { background-color: #FFFFFF; border: 1px solid #CBD5E1; border-radius: 6px; gridline-color: #E2E8F0; outline: none; font-size: 13px; color: #0F172A; }"
      "QHeaderView::section { background-color: #EFF6FF; color: #1E40AF; font-weight: bold; padding: 10px; border: none; border-bottom: 2px solid #BFDBFE; outline: none; }"
      "QTableWidget::item { padding: 8px; outline: none; border: none; }"
      "QTableWidget::item:focus { outline: none; border: none; }"
      "QTableWidget::item:selected { background-color: #E0F2FE; color: #0369A1; outline: none; border: none; }");

  connect(m_tablePatients, &QTableWidget::cellDoubleClicked, this, &PatientSelectionDialog::onRowDoubleClicked);

  mainLayout->addWidget(m_tablePatients);

  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  m_btnCancel = new QPushButton("Hủy", this);
  m_btnCancel->setStyleSheet(
      "QPushButton { background-color: #E2E8F0; color: #475569; padding: 8px 20px; border-radius: 6px; font-weight: bold; font-size: 14px; border: none; }"
      "QPushButton:hover { background-color: #CBD5E1; }");
  m_btnCancel->setCursor(Qt::PointingHandCursor);
  connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);

  m_btnSelect = new QPushButton("Chọn bệnh nhân", this);
  m_btnSelect->setStyleSheet(
      "QPushButton { background-color: #4B94F2; color: #FFFFFF; padding: 8px 20px; border-radius: 6px; font-weight: bold; font-size: 14px; border: none; }"
      "QPushButton:hover { background-color: #2563EB; }");
  m_btnSelect->setCursor(Qt::PointingHandCursor);
  connect(m_btnSelect, &QPushButton::clicked, this, &PatientSelectionDialog::onSelectClicked);

  btnLayout->addWidget(m_btnCancel);
  btnLayout->addWidget(m_btnSelect);

  mainLayout->addLayout(btnLayout);
}

void PatientSelectionDialog::populateTable() {
  m_tablePatients->setRowCount(0);
  for (int i = 0; i < m_patients.size(); ++i) {
    const auto &p = m_patients[i];
    m_tablePatients->insertRow(i);

    QTableWidgetItem *itemCode = new QTableWidgetItem(p.patientCode.isEmpty() ? "--" : p.patientCode);
    QTableWidgetItem *itemName = new QTableWidgetItem(p.fullName.isEmpty() ? "--" : p.fullName);
    QTableWidgetItem *itemDob = new QTableWidgetItem(p.dateOfBirth.isValid() ? p.dateOfBirth.toString("dd/MM/yyyy") : "--");
    QTableWidgetItem *itemGender = new QTableWidgetItem(GenderText::toVi(p.gender));

    itemCode->setTextAlignment(Qt::AlignCenter);
    itemDob->setTextAlignment(Qt::AlignCenter);
    itemGender->setTextAlignment(Qt::AlignCenter);

    m_tablePatients->setItem(i, 0, itemCode);
    m_tablePatients->setItem(i, 1, itemName);
    m_tablePatients->setItem(i, 2, itemDob);
    m_tablePatients->setItem(i, 3, itemGender);
  }

  if (m_patients.size() > 0) {
    m_tablePatients->selectRow(0);
  }
}

void PatientSelectionDialog::onSelectClicked() {
  int currentRow = m_tablePatients->currentRow();
  if (currentRow >= 0 && currentRow < m_patients.size()) {
    m_selectedPatient = m_patients[currentRow];
    accept();
  } else {
    QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một bệnh nhân từ danh sách.");
  }
}

void PatientSelectionDialog::onRowDoubleClicked(int row, int column) {
  Q_UNUSED(column);
  if (row >= 0 && row < m_patients.size()) {
    m_selectedPatient = m_patients[row];
    accept();
  }
}

PatientShortDTO PatientSelectionDialog::getSelectedPatient() const {
  return m_selectedPatient;
}
