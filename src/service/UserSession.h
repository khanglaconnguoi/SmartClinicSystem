#pragma once
#include "model/IAuthenticatable.h"
#include "model/SystemUser.h"
#include <QObject>
#include <memory>


class UserSession : public QObject {
    Q_OBJECT

public:
    // Singleton Instance: Lấy đối tượng duy nhất toàn hệ thống
    static UserSession& getInstance() {
        static UserSession instance;
        return instance;
    }

    // Xóa bỏ hoàn toàn Copy Constructor và Assignment Operator để đảm bảo tính duy nhất
    UserSession(const UserSession&) = delete;
    UserSession& operator=(const UserSession&) = delete;

    
    // Nạp tài khoản khi đăng nhập thành công
    void setCurrentAccount(std::shared_ptr<IAuthenticatable> account) {
        if (m_currentAccount != account) {
            m_currentAccount = account;
            emit sessionChanged(m_currentAccount); // Phát tín hiệu khi phiên thay đổi
        }
    }

    // Lấy con trỏ cha (Dùng chung cho mục đích định danh tổng quát)
    std::shared_ptr<IAuthenticatable> getCurrentAccount() const {
        return m_currentAccount;
    }

    // Xóa session khi Đăng xuất
    void clear() {
        if (m_currentAccount != nullptr) {
            m_currentAccount.reset();
            emit sessionChanged(nullptr); // Phát tín hiệu báo đã đăng xuất
        }
    }

    // Kiểm tra nhanh xem hệ thống đã có ai đăng nhập chưa
    bool isLoggedIn() const {
        return m_currentAccount != nullptr;
    }

    // 3. Các hàm Tiện ích (Helper Functions) giúp tầng UI code cực nhanh
    
    // Lấy nhanh Role hiện tại (Nếu chưa đăng nhập mặc định trả về Guest hoặc lỗi)
    UserRole getCurrentRole() const {
        if (!isLoggedIn()) return UserRole::Nurse; // Hoặc một giá trị mặc định/Guest nào đó tùy hệ thống của bạn
        return m_currentAccount->getRole();
    }

    // Kiểm tra nhanh xem người dùng hiện tại có phải Admin hay không
    bool isAdmin() const {
        return isLoggedIn() && (getCurrentRole() == UserRole::Admin);
    }

signals:
    // Tín hiệu phát đi mỗi khi Đăng nhập hoặc Đăng xuất thành công
    void sessionChanged(std::shared_ptr<IAuthenticatable> newAccount);

private:
    // Constructor private để ngăn chặn việc tự ý khởi tạo từ bên ngoài
    UserSession() = default;
    ~UserSession() override = default;

    std::shared_ptr<IAuthenticatable> m_currentAccount = nullptr;
};