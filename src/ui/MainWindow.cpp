/**
 * @file    MainWindow.cpp
 * @brief   Implementation cho MainWindow — khởi tạo module Patient.
 */
#include "MainWindow.h"
#include "PatientView.h"
#include "repository/DatabaseManager.h"
#include "repository/PatientRepository.h"
#include "service/PatientService.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("Smart Clinic System");
    resize(1000, 600);

    // Khởi tạo dependency chain:
    // DatabaseManager (singleton) → PatientRepository → PatientService → PatientView
    auto& dbManager = DatabaseManager::instance();

    m_patientRepo = new PatientRepository(dbManager.database());
    m_patientService = new PatientService(m_patientRepo);
    m_patientView = new PatientView(m_patientService, this);

    setCentralWidget(m_patientView);
}

MainWindow::~MainWindow() {
    // m_patientView được Qt tự delete (có parent = this)
    // m_patientService và m_patientRepo cần delete thủ công
    delete m_patientService;
    delete m_patientRepo;
}
