#include "PatientDashboard.h"
#include "model/IAuthenticatable.h"
#include "service/AppointmentService.h"
#include "service/AuthService.h"
#include "service/PatientService.h"
#include "service/StaffService.h"
#include "model/CommonEnums.h"
#include <QDate>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QScrollBar>


// =============================================================================
// CONSTRUCTOR
// =============================================================================
PatientDashboardWidget::PatientDashboardWidget(
    std::shared_ptr<IAuthenticatable> user,
    std::shared_ptr<StaffService> staffService,
    std::shared_ptr<PatientService> patientService,
    std::shared_ptr<AppointmentService> appointmentService, QWidget *parent)
    : BaseDashboardWidget(user, staffService, patientService,
                          appointmentService, parent) {
  initializeDashboard();
}

// =============================================================================
// fillDashboardData() — điểm hook lớp cha gọi vào
// =============================================================================
void PatientDashboardWidget::fillDashboardData() {
  buildPatientSidebar();
  buildTopbar();
  buildScrollableContent();
}

// =============================================================================
// SIDEBAR — menu dành riêng cho bệnh nhân
// =============================================================================
void PatientDashboardWidget::buildPatientSidebar() {
  if (!m_sidebarFrame || !m_sidebarLayout)
    return;

  // --- Logo / tiêu đề phòng khám ---
  if (m_logoLabel) {
    m_logoLabel->setText("Nova Care");
    m_logoLabel->setStyleSheet("font-size: 20px; font-weight: bold;"
                               "color: #4B94F2; margin-bottom: 8px;");
  }

  // --- Nhãn phân nhóm ---
  QLabel *menuHeader = new QLabel("MENU", m_sidebarFrame);
  menuHeader->setStyleSheet(
      "font-size: 10px; font-weight: bold; color: #B0B8C4;"
      "letter-spacing: 2px; margin: 8px 4px 4px 4px;");
  m_sidebarLayout->addWidget(menuHeader);

  // --- Tạo các nút sidebar bệnh nhân ---
  m_btnOverview = new QPushButton("Tổng Quan", m_sidebarFrame);
  m_btnMyAppoint = new QPushButton("Lịch Hẹn Của Tôi", m_sidebarFrame);
  m_btnMedRecord = new QPushButton("Hồ Sơ Bệnh Án", m_sidebarFrame);
  m_btnLabResult = new QPushButton("Kết Quả Xét Nghiệm", m_sidebarFrame);
  m_btnPrescript = new QPushButton("Đơn Thuốc", m_sidebarFrame);

  // Nhãn phân nhóm thứ 2
  QLabel *accountHeader = new QLabel("TÀI KHOẢN", m_sidebarFrame);
  accountHeader->setStyleSheet(
      "font-size: 10px; font-weight: bold; color: #B0B8C4;"
      "letter-spacing: 2px; margin: 14px 4px 4px 4px;");

  m_btnProfile = new QPushButton("Thông Tin Cá Nhân", m_sidebarFrame);

  // Thêm vào layout sidebar
  m_sidebarLayout->addWidget(m_btnOverview);
  m_sidebarLayout->addWidget(m_btnMyAppoint);
  m_sidebarLayout->addWidget(m_btnMedRecord);
  m_sidebarLayout->addWidget(m_btnLabResult);
  m_sidebarLayout->addWidget(m_btnPrescript);
  m_sidebarLayout->addWidget(accountHeader);
  m_sidebarLayout->addWidget(m_btnProfile);

  // Giữ nút Logout xuống cuối
  m_sidebarLayout->addStretch();

  m_btnLogout = new QPushButton("Đăng Xuất", m_sidebarFrame);
  m_btnLogout->setStyleSheet(
      "QPushButton { text-align: left; padding: 12px 20px; font-size: 14px; "
      "color: #D32F2F; border: none; border-radius: 0px; background-color: "
      "transparent; font-weight: bold; }"
      "QPushButton:hover { background-color: #FFEBEE; }");
  m_btnLogout->setCursor(Qt::PointingHandCursor);
  m_sidebarLayout->addWidget(m_btnLogout);
  connect(m_btnLogout, &QPushButton::clicked, this,
          &BaseDashboardWidget::logoutRequested);

  // Đặt nút đang active mặc định
  setActiveSidebarBtn(m_btnOverview);
}

// =============================================================================
// TOPBAR — tên bệnh nhân + avatar
// =============================================================================
void PatientDashboardWidget::buildTopbar() {
  // --- Tên người dùng ---
  if (m_currentUser && m_docNameLabel) {
    QString name = m_currentUser->getFullName();
    m_docNameLabel->setText(name.isEmpty() ? "Bệnh Nhân" : name);
    m_docNameLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: "
                                  "#3C4043; font-family: 'Segoe UI';");
  }

  // --- Ẩn thanh search (bệnh nhân không cần search toàn hệ thống) ---
  if (m_searchInput) {
    m_searchInput->setPlaceholderText("Tìm kiếm...");
    m_searchInput->setFixedWidth(280);
  }

  // --- Avatar ---
  if (m_docAvatarBtn && m_currentUser) {
    QPixmap raw = m_currentUser->getAvatar();
    if (raw.isNull()) {
      raw = QPixmap(36, 36);
      raw.fill(QColor("#4B94F2"));
    }

    const int sz = 36;
    m_docAvatarBtn->setFixedSize(sz, sz);

    QPixmap scaled = raw.scaled(sz, sz, Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);
    QPixmap target(sz, sz);
    target.fill(Qt::transparent);

    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, sz, sz);
    p.setClipPath(path);
    p.drawPixmap(0, 0, scaled);
    p.end();

    m_docAvatarBtn->setIconSize(QSize(sz, sz));
    m_docAvatarBtn->setIcon(QIcon(target));
    m_docAvatarBtn->setStyleSheet("QPushButton { background:transparent; "
                                  "border:none; padding:0; margin:0; }");
    m_docAvatarBtn->update();
  }
}

// =============================================================================
// NỘI DUNG CHÍNH — cuộn được
// =============================================================================
void PatientDashboardWidget::buildScrollableContent() {
  if (!m_mainContentLayout)
    return;

  // Tạo ScrollArea bao bên ngoài
  m_scrollArea = new QScrollArea(m_mainContentWidget);
  m_scrollArea->setFrameShape(QFrame::NoFrame);
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_scrollArea->setStyleSheet(
      "QScrollArea { background: transparent; border: none; }"
      "QScrollBar:vertical {"
      "   background: #F1F3F4; width: 6px; border-radius: 3px;"
      "}"
      "QScrollBar::handle:vertical {"
      "   background: #C5CAD4; border-radius: 3px; min-height: 30px;"
      "}"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: "
      "0px; }");

  m_scrollContent = new QWidget();
  m_scrollContent->setStyleSheet("background: transparent;");
  m_scrollLayout = new QVBoxLayout(m_scrollContent);
  m_scrollLayout->setContentsMargins(0, 0, 12, 20);
  m_scrollLayout->setSpacing(24);

  m_scrollArea->setWidget(m_scrollContent);

  // Vẽ từng khối nội dung
  createWelcomeBanner();
  createQuickStatCards();
  createUpcomingAppointments();
  createRecentMedicalRecords();
  createHealthTimeline();

  m_scrollLayout->addStretch();
  m_mainContentLayout->addWidget(m_scrollArea, 1);
}

// =============================================================================
// 1. WELCOME BANNER
// =============================================================================
void PatientDashboardWidget::createWelcomeBanner() {
  QFrame *banner = new QFrame(m_scrollContent);
  banner->setMinimumHeight(100);
  banner->setStyleSheet("QFrame {"
                        "   background-color: #FFFFFF;"
                        "   border: 1px solid #E5E7EB;"
                        "   border-left: 5px solid #4B94F2;"
                        "   border-radius: 8px;"
                        "}");

  QHBoxLayout *hl = new QHBoxLayout(banner);
  hl->setContentsMargins(24, 20, 24, 20);

  QVBoxLayout *textCol = new QVBoxLayout();
  textCol->setSpacing(6);

  QString firstName = "Bệnh nhân";
  if (m_currentUser) {
    QStringList parts = m_currentUser->getFullName().split(' ');
    if (!parts.isEmpty())
      firstName = parts.last();
  }

  QLabel *greeting = new QLabel(QString("Xin chào, %1").arg(firstName), banner);
  greeting->setStyleSheet("color: #111827; font-size: 20px; font-weight: bold; "
                          "background: transparent; border: none;");

  QLabel *sub = new QLabel("Chúc bạn sức khỏe dồi dào. Đây là tổng quan hồ sơ "
                           "sức khỏe của bạn hôm nay.",
                           banner);
  sub->setWordWrap(true);
  sub->setStyleSheet("color: #5F6368; font-size: 13px; background: "
                     "transparent; border: none;");

  textCol->addWidget(greeting);
  textCol->addWidget(sub);

  // Nhãn ngày hôm nay bên phải
  QLabel *dateLabel =
      new QLabel(QDate::currentDate().toString("dddd, dd/MM/yyyy"), banner);
  dateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  dateLabel->setStyleSheet("color: #5F6368; font-size: 13px; font-weight: 500; "
                           "background: transparent; border: none;");

  hl->addLayout(textCol, 1);
  hl->addWidget(dateLabel);

  m_scrollLayout->addWidget(banner);
}

// =============================================================================
// 2. QUICK STAT CARDS
// =============================================================================
void PatientDashboardWidget::createQuickStatCards() {
  QHBoxLayout *row = new QHBoxLayout();
  row->setSpacing(16);

  struct CardInfo {
    QString title;
    QString value;
    QString sub;
  };

  QList<CardInfo> cards = {
      {"Lịch hẹn sắp tới", "2", "Trong 7 ngày tới"},
      {"Đơn thuốc đang dùng", "3", "Đang trong liệu trình"},
      {"Kết quả chờ xem", "1", "Cập nhật hôm nay"},
      {"Lần khám gần nhất", "15/06/2026", "Nội khoa tổng quát"},
  };

  for (const auto &info : cards) {
    QFrame *card = makeCard(m_scrollContent);
    card->setMinimumHeight(110);

    QVBoxLayout *vl = new QVBoxLayout(card);
    vl->setContentsMargins(20, 18, 20, 18);
    vl->setSpacing(8);

    QLabel *titleLbl = new QLabel(info.title, card);
    titleLbl->setStyleSheet(
        "color: #5F6368; font-size: 12px; font-weight: 500; border: none;");

    QLabel *valueLbl = new QLabel(info.value, card);
    valueLbl->setStyleSheet(
        "color: #4B94F2; font-size: 22px; font-weight: bold; border: none;");

    QLabel *subLbl = new QLabel(info.sub, card);
    subLbl->setStyleSheet("color: #9CA3AF; font-size: 11px; border: none;");

    vl->addWidget(titleLbl);
    vl->addWidget(valueLbl);
    vl->addWidget(subLbl);

    row->addWidget(card);
  }

  m_scrollLayout->addLayout(row);
}

// =============================================================================
// 3. UPCOMING APPOINTMENTS
// =============================================================================
void PatientDashboardWidget::createUpcomingAppointments() {
  QFrame *section = makeCard(m_scrollContent);
  QVBoxLayout *vl = new QVBoxLayout(section);
  vl->setContentsMargins(24, 20, 24, 20);
  vl->setSpacing(16);

  // Header hàng
  QHBoxLayout *hdr = new QHBoxLayout();
  QLabel *title = new QLabel("Lịch Hẹn Sắp Tới", section);
  title->setStyleSheet("font-size: 16px; font-weight: bold; color: #111827;");

  QPushButton *btnAll = new QPushButton("Xem tất cả →", section);
  btnAll->setCursor(Qt::PointingHandCursor);
  btnAll->setStyleSheet(
      "QPushButton { background: transparent; color: #4B94F2; font-size: 13px;"
      "font-weight: 600; border: none; padding: 0; }"
      "QPushButton:hover { color: #398CBF; }");
  hdr->addWidget(title);
  hdr->addStretch();
  hdr->addWidget(btnAll);
  vl->addLayout(hdr);

  // Separator
  QFrame *sep = new QFrame(section);
  sep->setFrameShape(QFrame::HLine);
  sep->setStyleSheet("background: #F3F4F6; border: none; max-height: 1px;");
  vl->addWidget(sep);

  // Gọi Service để lấy dữ liệu lịch hẹn thực tế
  auto records = m_baseAppointmentService->getPatientAppointments(
      m_currentUser->getAccountId());

  if (records.isEmpty()) {
    QLabel *emptyLbl = new QLabel("Không có lịch hẹn sắp tới.", section);
    emptyLbl->setStyleSheet("color: #6B7280; font-style: italic;");
    vl->addWidget(emptyLbl);
  } else {
    // Chỉ hiển thị tối đa 3 lịch hẹn gần nhất (upcoming)
    int count = 0;
    for (const auto &a : records) {
      if (count >= 3)
        break;

      QFrame *item = new QFrame(section);
      item->setObjectName("AppointmentItem");
      item->setStyleSheet("QFrame#AppointmentItem { background: #F9FAFB; "
                          "border-radius: 10px; border: 1px solid #EAEAEA; }"
                          "QFrame#AppointmentItem:hover { background: #EEF2FF; "
                          "border-color: #C7D2FE; }");
      item->setCursor(Qt::PointingHandCursor);

      QHBoxLayout *hl = new QHBoxLayout(item);
      hl->setContentsMargins(16, 12, 16, 12);
      hl->setSpacing(16);

      // Khối ngày tháng
      QFrame *dateBadge = new QFrame(item);
      dateBadge->setFixedSize(52, 52);
      dateBadge->setStyleSheet(
          "QFrame { background: #ACDEF2; border-radius: 8px; border: none; }");
      QVBoxLayout *dv = new QVBoxLayout(dateBadge);
      dv->setContentsMargins(4, 4, 4, 4);
      dv->setSpacing(0);
      QStringList dateParts = a.appointmentDate.split('-');
      QString dayStr = dateParts.size() == 3 ? dateParts[2] : "??";
      QString monthStr = dateParts.size() == 3 ? dateParts[1] : "??";

      QLabel *dayLbl = new QLabel(dayStr, dateBadge);
      dayLbl->setAlignment(Qt::AlignCenter);
      dayLbl->setStyleSheet(
          "font-size: 18px; font-weight: bold; color: #4B94F2;");
      QLabel *monLbl = new QLabel(QString("Th.%1").arg(monthStr), dateBadge);
      monLbl->setAlignment(Qt::AlignCenter);
      monLbl->setStyleSheet(
          "font-size: 10px; color: #4B94F2; font-weight: 600;");
      dv->addWidget(dayLbl);
      dv->addWidget(monLbl);

      // Thông tin chính
      QVBoxLayout *info = new QVBoxLayout();
      info->setSpacing(3);
      QLabel *drLbl = new QLabel(a.doctorName, item);
      drLbl->setStyleSheet(
          "font-size: 14px; font-weight: 600; color: #111827;");
      QString spTime = QString("%1  ·  %2 - %3")
                           .arg(a.doctorSpecialty, a.startTime, a.endTime);
      QLabel *spLbl = new QLabel(spTime, item);
      spLbl->setStyleSheet("font-size: 12px; color: #6B7280;");
      info->addWidget(drLbl);
      info->addWidget(spLbl);

      // Badge trạng thái
      QString statusColor = "#5F6368";
      QString statusBg = "#F1F3F4";
      if (a.status == AppointmentStatusText::SCHEDULED || a.status == AppointmentStatusText::CHECKED_IN) {
        statusColor = "#F59E0B";
        statusBg = "#FEF3C7";
      } else if (a.status == AppointmentStatusText::COMPLETED) {
        statusColor = "#10B981";
        statusBg = "#D1FAE5";
      } else if (a.status == AppointmentStatusText::CANCELLED) {
        statusColor = "#EF4444";
        statusBg = "#FEE2E2";
      }

      QLabel *statusLbl = new QLabel(AppointmentStatusText::toVi(a.status), item);
      statusLbl->setAlignment(Qt::AlignCenter);
      statusLbl->setFixedHeight(26);
      statusLbl->setContentsMargins(10, 0, 10, 0);
      statusLbl->setStyleSheet(
          QString("color: %1; background: %2; border-radius: 6px;"
                  "font-size: 11px; font-weight: 600; padding: 0 10px;")
              .arg(statusColor, statusBg));

      hl->addWidget(dateBadge);
      hl->addLayout(info, 1);
      hl->addWidget(statusLbl);

      // Nút Hủy
      if (a.status == AppointmentStatusText::SCHEDULED) {
        QPushButton *btnCancel = new QPushButton("Hủy", item);
        btnCancel->setCursor(Qt::PointingHandCursor);
        btnCancel->setStyleSheet(
            "QPushButton { background: #FEE2E2; color: #EF4444; border: 1px "
            "solid #FCA5A5; border-radius: 4px; padding: 4px 8px; font-weight: "
            "bold; font-size: 11px; }"
            "QPushButton:hover { background: #FECACA; }");
        int id = a.appointmentId;
        connect(btnCancel, &QPushButton::clicked, this,
                [this, id]() { onCancelAppointmentClicked(id); });
        hl->addWidget(btnCancel);
      }

      vl->addWidget(item);
      count++;
    }
  }

  m_scrollLayout->addWidget(section);
}

void PatientDashboardWidget::onCancelAppointmentClicked(int appointmentId) {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Xác nhận hủy lịch", "Bạn có chắc chắn muốn hủy lịch khám này không?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QString errorMsg = m_baseAppointmentService->cancelAppointment(appointmentId);
        if (errorMsg.isEmpty()) {
            QMessageBox::information(this, "Thành công", "Đã hủy lịch hẹn thành công.");
            // Refresh dashboard
            if (m_scrollArea) {
                m_mainContentLayout->removeWidget(m_scrollArea);
                m_scrollArea->deleteLater();
                m_scrollArea = nullptr;
            }
            buildScrollableContent();
        } else {
            QMessageBox::warning(this, "Lỗi", errorMsg);
        }
    }
}

// =============================================================================
// 4. RECENT MEDICAL RECORDS
// =============================================================================
void PatientDashboardWidget::createRecentMedicalRecords() {
  QHBoxLayout *row = new QHBoxLayout();
  row->setSpacing(16);

  // ---- Card trái: Hồ sơ bệnh án gần nhất ----
  QFrame *medCard = makeCard(m_scrollContent);
  medCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  QVBoxLayout *mv = new QVBoxLayout(medCard);
  mv->setContentsMargins(24, 20, 24, 20);
  mv->setSpacing(14);

  QLabel *medTitle = new QLabel("Lần Khám Gần Nhất", medCard);
  medTitle->setStyleSheet(
      "font-size: 16px; font-weight: bold; color: #111827;");
  mv->addWidget(medTitle);

  QFrame *sep = new QFrame(medCard);
  sep->setFrameShape(QFrame::HLine);
  sep->setStyleSheet("background: #F3F4F6; border: none; max-height: 1px;");
  mv->addWidget(sep);

  // Row thông tin
  auto addInfoRow = [&](QVBoxLayout *layout, const QString &label,
                        const QString &value, QWidget *parent) {
    QHBoxLayout *hl = new QHBoxLayout();
    QLabel *lbl = new QLabel(label, parent);
    lbl->setStyleSheet(
        "color: #9CA3AF; font-size: 12px; font-weight: 500; min-width: 140px;");
    QLabel *val = new QLabel(value, parent);
    val->setStyleSheet("color: #111827; font-size: 13px; font-weight: 600;");
    val->setWordWrap(true);
    hl->addWidget(lbl);
    hl->addWidget(val, 1);
    layout->addLayout(hl);
  };

  addInfoRow(mv, "Ngày khám:", "15/06/2026", medCard);
  addInfoRow(mv, "Bác sĩ điều trị:", "BS. Nguyễn Văn An", medCard);
  addInfoRow(mv, "Chuyên khoa:", "Nội khoa tổng quát", medCard);
  addInfoRow(mv, "Chẩn đoán:", "Viêm họng cấp, Cảm cúm A", medCard);
  addInfoRow(mv, "Ghi chú bác sĩ:",
             "Nghỉ ngơi, uống nhiều nước, tái khám sau 7 ngày.", medCard);

  mv->addStretch();

  QPushButton *btnView = new QPushButton("Xem hồ sơ đầy đủ →", medCard);
  btnView->setCursor(Qt::PointingHandCursor);
  btnView->setStyleSheet("QPushButton { background: #4B94F2; color: white; "
                         "border: none; border-radius: 8px;"
                         "font-size: 13px; font-weight: 600; padding: 10px 0; }"
                         "QPushButton:hover { background: #398CBF; }"
                         "QPushButton:pressed { background: #62B7D9; }");
  mv->addWidget(btnView);

  row->addWidget(medCard, 3);

  // ---- Card phải: Đơn thuốc hiện tại ----
  QFrame *rxCard = makeCard(m_scrollContent);
  rxCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  QVBoxLayout *rv = new QVBoxLayout(rxCard);
  rv->setContentsMargins(24, 20, 24, 20);
  rv->setSpacing(14);

  QLabel *rxTitle = new QLabel("Đơn Thuốc Hiện Tại", rxCard);
  rxTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #111827;");
  rv->addWidget(rxTitle);

  QFrame *sep2 = new QFrame(rxCard);
  sep2->setFrameShape(QFrame::HLine);
  sep2->setStyleSheet("background: #F3F4F6; border: none; max-height: 1px;");
  rv->addWidget(sep2);

  struct MedInfo {
    QString name, dose, freq, days;
  };
  QList<MedInfo> meds = {
      {"Amoxicillin 500mg", "1 viên", "3 lần/ngày", "7 ngày"},
      {"Paracetamol 500mg", "1 viên", "Khi sốt", "5 ngày"},
      {"Vitamin C 1000mg", "1 viên", "1 lần/ngày", "14 ngày"},
  };

  for (const auto &med : meds) {
    QFrame *mItem = new QFrame(rxCard);
    mItem->setStyleSheet(
        "background: #F9FAFB; border-radius: 8px; border: 1px solid #F3F4F6;");
    QHBoxLayout *mhl = new QHBoxLayout(mItem);
    mhl->setContentsMargins(14, 10, 14, 10);

    QLabel *pill = new QLabel("•", mItem);
    pill->setStyleSheet("color: #4B94F2; font-size: 18px; font-weight: bold;");
    pill->setFixedWidth(16);
    QVBoxLayout *mi = new QVBoxLayout();
    mi->setSpacing(2);
    QLabel *mName = new QLabel(med.name, mItem);
    mName->setStyleSheet("font-size: 13px; font-weight: 600; color: #111827;");
    QLabel *mDetail = new QLabel(
        QString("%1 · %2 · %3").arg(med.dose, med.freq, med.days), mItem);
    mDetail->setStyleSheet("font-size: 11px; color: #6B7280;");
    mi->addWidget(mName);
    mi->addWidget(mDetail);

    mhl->addWidget(pill);
    mhl->addLayout(mi, 1);
    rv->addWidget(mItem);
  }

  rv->addStretch();

  QPushButton *btnRx = new QPushButton("Xem tất cả đơn thuốc →", rxCard);
  btnRx->setCursor(Qt::PointingHandCursor);
  btnRx->setStyleSheet("QPushButton { background: #ACDEF2; color: #4B94F2; "
                       "border: 1px solid #C2E7DA;"
                       "border-radius: 8px; font-size: 13px; font-weight: 600; "
                       "padding: 10px 0; }"
                       "QPushButton:hover { background: #D0EFE4; }");
  rv->addWidget(btnRx);

  row->addWidget(rxCard, 2);
  m_scrollLayout->addLayout(row);
}

// =============================================================================
// 5. HEALTH TIMELINE
// =============================================================================
void PatientDashboardWidget::createHealthTimeline() {
  QFrame *section = makeCard(m_scrollContent);
  QVBoxLayout *vl = new QVBoxLayout(section);
  vl->setContentsMargins(24, 20, 24, 20);
  vl->setSpacing(16);

  QLabel *title = new QLabel("Lịch Sử Hoạt Động Sức Khỏe", section);
  title->setStyleSheet("font-size: 16px; font-weight: bold; color: #111827;");
  vl->addWidget(title);

  QFrame *sep = new QFrame(section);
  sep->setFrameShape(QFrame::HLine);
  sep->setStyleSheet("background: #F3F4F6; border: none; max-height: 1px;");
  vl->addWidget(sep);

  struct TimelineItem {
    QString date, event, detail, dotColor;
  };

  QList<TimelineItem> items = {
      {"15/06/2026", "Khám bệnh",
       "Nội khoa — BS. Nguyễn Văn An — Viêm họng cấp", "#4B94F2"},
      {"10/06/2026", "Kết quả xét nghiệm",
       "Xét nghiệm máu toàn phần — Kết quả bình thường", "#4B94F2"},
      {"02/06/2026", "Lịch hẹn",
       "Tim mạch — BS. Trần Thị Bình — Kiểm tra định kỳ", "#4B94F2"},
      {"20/05/2026", "Cấp đơn thuốc", "Vitamin tổng hợp — Liệu trình 30 ngày",
       "#4B94F2"},
      {"05/05/2026", "Khám bệnh",
       "Da liễu — BS. Lê Minh Tuấn — Điều trị mụn trứng cá", "#4B94F2"},
  };

  for (int i = 0; i < items.size(); ++i) {
    const auto &item = items[i];

    QHBoxLayout *row = new QHBoxLayout();
    row->setSpacing(16);

    // Cột trái: chấm tròn + đường thẳng
    QWidget *dotCol = new QWidget(section);
    dotCol->setFixedWidth(20);
    QVBoxLayout *dotV = new QVBoxLayout(dotCol);
    dotV->setContentsMargins(0, 0, 0, 0);
    dotV->setSpacing(0);

    QLabel *dot = new QLabel("●", dotCol);
    dot->setAlignment(Qt::AlignHCenter);
    dot->setStyleSheet(
        QString("color: %1; font-size: 12px;").arg(item.dotColor));
    dotV->addWidget(dot);

    if (i < items.size() - 1) {
      QFrame *line = new QFrame(dotCol);
      line->setFrameShape(QFrame::VLine);
      line->setStyleSheet("color: #E5E7EB;");
      line->setMinimumHeight(30);
      dotV->addWidget(line, 1);
    }

    // Cột phải: thông tin
    QVBoxLayout *infoV = new QVBoxLayout();
    infoV->setSpacing(2);

    QHBoxLayout *topRow = new QHBoxLayout();
    QLabel *evtLbl = new QLabel(item.event, section);
    evtLbl->setStyleSheet("font-size: 13px; font-weight: 600; color: #111827;");
    QLabel *dateLbl = new QLabel(item.date, section);
    dateLbl->setStyleSheet("font-size: 12px; color: #9CA3AF;");
    topRow->addWidget(evtLbl);
    topRow->addStretch();
    topRow->addWidget(dateLbl);

    QLabel *detailLbl = new QLabel(item.detail, section);
    detailLbl->setStyleSheet("font-size: 12px; color: #6B7280;");
    detailLbl->setWordWrap(true);

    infoV->addLayout(topRow);
    infoV->addWidget(detailLbl);

    row->addWidget(dotCol);
    row->addLayout(infoV, 1);

    vl->addLayout(row);
  }

  m_scrollLayout->addWidget(section);
}

// =============================================================================
// HELPERS
// =============================================================================
QFrame *PatientDashboardWidget::makeCard(QWidget *parent) {
  QFrame *card = new QFrame(parent ? parent : m_scrollContent);
  card->setObjectName("DashboardCard");
  card->setStyleSheet("QFrame#DashboardCard {"
                      "   background-color: #FFFFFF;"
                      "   border: 1px solid #E5E7EB;"
                      "   border-radius: 14px;"
                      "}");

  // Drop shadow nhẹ
  auto *shadow = new QGraphicsDropShadowEffect(card);
  shadow->setBlurRadius(16);
  shadow->setOffset(0, 2);
  shadow->setColor(QColor(0, 0, 0, 18));
  card->setGraphicsEffect(shadow);

  return card;
}

void PatientDashboardWidget::setActiveSidebarBtn(QPushButton *btn) {
  QPushButton *allBtns[] = {m_btnOverview,  m_btnMyAppoint, m_btnMedRecord,
                            m_btnLabResult, m_btnPrescript, m_btnProfile};
  for (auto *b : allBtns) {
    if (b)
      b->setObjectName("");
  }
  if (btn)
    btn->setObjectName("activeBtn");

  // Refresh style
  if (m_sidebarFrame)
    m_sidebarFrame->setStyleSheet(m_sidebarFrame->styleSheet());
}
