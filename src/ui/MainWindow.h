/**
 * @file    MainWindow.h
 * @brief   Cửa sổ chính của ứng dụng Smart Clinic System.
 */
#pragma once

#include <QMainWindow>

class PatientRepository;
class PatientService;
class PatientView;

/**
 * @brief MainWindow — khởi tạo dependency chain và hiển thị PatientView.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    PatientRepository* m_patientRepo;
    PatientService*    m_patientService;
    PatientView*       m_patientView;
};
