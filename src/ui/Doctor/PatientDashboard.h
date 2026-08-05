#pragma once

#include "ui/BaseDashboard.h"
#include "model/IAuthenticatable.h"
#include "service/StaffService.h"
#include "service/PatientService.h"
#include "service/AppointmentService.h"
#include <QFrame>
#include <QScrollArea>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>

class PatientDashboardWidget : public BaseDashboardWidget {
  Q_OBJECT

public:
  explicit PatientDashboardWidget(
      std::shared_ptr<IAuthenticatable> user = nullptr,
      std::shared_ptr<StaffService> staffService = nullptr,
      std::shared_ptr<PatientService> patientService = nullptr,
      std::shared_ptr<AppointmentService> appointmentService = nullptr,
      QWidget *parent = nullptr);
  virtual ~PatientDashboardWidget() override = default;

protected:
  virtual void fillDashboardData() override;

private:
  std::shared_ptr<PatientService> m_patientService;
  std::shared_ptr<AppointmentService> m_appointmentService;


  // Sidebar buttons dành riêng cho bệnh nhân
  QPushButton *m_btnOverview = nullptr;  // Tổng quan
  QPushButton *m_btnMyAppoint = nullptr; // Lịch hẹn của tôi
  QPushButton *m_btnMedRecord = nullptr; // Hồ sơ bệnh án
  QPushButton *m_btnLabResult = nullptr; // Kết quả xét nghiệm
  QPushButton *m_btnPrescript = nullptr; // Đơn thuốc
  QPushButton *m_btnProfile = nullptr;   // Thông tin cá nhân

private:
  // Vùng nội dung chính cuộn được
  QScrollArea *m_scrollArea = nullptr;
  QWidget *m_scrollContent = nullptr;
  QVBoxLayout *m_scrollLayout = nullptr;

  // --- Các hàm vẽ từng khối giao diện ---
  void buildPatientSidebar();
  void buildTopbar();
  void buildScrollableContent();

  void createWelcomeBanner();
  void createQuickStatCards();
  void createUpcomingAppointments();
  void createRecentMedicalRecords();
  void createHealthTimeline();

  // Helper
  QFrame *makeCard(QWidget *parent = nullptr);
  void setActiveSidebarBtn(QPushButton *btn);

private slots:
  void onCancelAppointmentClicked(int appointmentId);
};
