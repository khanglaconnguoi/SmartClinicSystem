/**
 * @file    MainWindow.cpp
 * @brief   Implementation cho MainWindow — khởi tạo module Patient.
 */
#include "MainWindow.h"
#include "repository/DatabaseManager.h"
#include "repository/PatientRepository.h"
#include "service/PatientService.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Nova Care");
  resize(1200, 700);

  auto &dbManager = DatabaseManager::instance();
  auto repo = std::make_shared<PatientRepository>(dbManager.database());
  m_patientService = new PatientService(repo);

  setupUi();
}

void MainWindow::setupUi() {
  auto *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() {
  // m_patientService cần delete thủ công, repo là shared_ptr sẽ tự giải phóng.
  delete m_patientService;
}
