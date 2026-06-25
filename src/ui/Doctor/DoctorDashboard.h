#pragma once

#include "../BaseDashboard.h" // Đường dẫn trỏ tới lớp cha Base của bạn
#include <memory>

// Forward declaration các lớp cần thiết để tối ưu hóa thời gian biên dịch
class IAuthenticatable;
class QVBoxLayout;
class QHBoxLayout;

class DoctorDashboardWidget : public BaseDashboardWidget {
    Q_OBJECT

public:
    // Constructor nhận vào Smart Pointer của User đăng nhập giống như code gốc của bạn
    explicit DoctorDashboardWidget(std::shared_ptr<IAuthenticatable> user, QWidget *parent = nullptr);
    virtual ~DoctorDashboardWidget() override = default;

protected:
    virtual void fillDashboardData() override;

private:
    // Con trỏ lưu thông tin bác sĩ hiện tại dùng để lấy tên thật từ Database
    std::shared_ptr<IAuthenticatable> m_currentUser;

    // Các hàm Helper đảm nhận việc tính toán bố cục vẽ hình và nạp dữ liệu tĩnh/động
    void createDoctorCards();
    void createDoctorCharts();
    void createDoctorTable();
};