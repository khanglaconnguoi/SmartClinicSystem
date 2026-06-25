/**
 * @file    MainWindow.h
/**
 * @file    MainWindow.h
 * @brief   Cửa sổ chính của ứng dụng Smart Clinic System.
 */
#pragma once

#include <QMainWindow>

class PatientRepository;
class PatientService;
class QStackedWidget;
class QLabel;
class QPushButton;

/**
 * @brief MainWindow — khởi tạo dependency chain.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

private:
  void setupUi();

  PatientService *m_patientService;

  QStackedWidget *m_stackedWidget;
  QLabel *m_lblTitle;
  QLabel *m_lblUserInfo;
};
