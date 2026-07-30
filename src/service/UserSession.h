#pragma once
#include "model/IAuthenticatable.h"
#include "model/SystemUser.h"
#include <QObject>
#include <memory>
#include <optional>

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

    // Lấy con trỏ tài khoản hiện tại
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

    // Lấy nhanh Role hiện tại an toàn (Nếu chưa đăng nhập trả về std::nullopt)
    std::optional<UserRole> getCurrentRole() const {
        if (!isLoggedIn() || !m_currentAccount) return std::nullopt;
        return m_currentAccount->getRole();
    }

    // Các hàm Tiện ích (Helper Functions) kiểm tra vai trò an toàn cho tầng UI & Service
    bool isAdmin() const {
        auto r = getCurrentRole();
        return r.has_value() && r.value() == UserRole::Admin;
    }

    bool isDoctor() const {
        auto r = getCurrentRole();
        return r.has_value() && r.value() == UserRole::Doctor;
    }

    bool isNurse() const {
        auto r = getCurrentRole();
        return r.has_value() && r.value() == UserRole::Nurse;
    }

    bool isReceptionist() const {
        auto r = getCurrentRole();
        return r.has_value() && r.value() == UserRole::Receptionist;
    }

    bool isPharmacist() const {
        auto r = getCurrentRole();
        return r.has_value() && r.value() == UserRole::Pharmacist;
    }

signals:
    // Tín hiệu phát đi mỗi khi Đăng nhập hoặc Đăng xuất thành công
    void sessionChanged(std::shared_ptr<IAuthenticatable> newAccount);

private:
    UserSession() = default;
    ~UserSession() override = default;

    std::shared_ptr<IAuthenticatable> m_currentAccount = nullptr;
};