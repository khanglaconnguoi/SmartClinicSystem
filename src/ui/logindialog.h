#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QAction>
#include <memory>         // Để dùng std::shared_ptr

// Khai báo trước (Forward declaration) lớp AuthService để không cần nạp chồng chéo thư viện
class AuthService; 

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    // Sửa hàm khởi tạo: Nhận con trỏ quản lý lớp AuthService
    explicit LoginDialog(std::shared_ptr<AuthService> authService, QWidget *parent = nullptr);
    ~LoginDialog();

    // BỔ SUNG: Hàm tiện ích công khai giúp dọn dẹp các ô nhập liệu khi cần thiết (ví dụ lúc logout)
    void clearFields();

protected:
    void paintEvent(QPaintEvent *event) override;
    // BỔ SUNG: Cho phép kéo giữ cửa sổ di chuyển khắp màn hình vì đã ẩn khung viền mặc định của Windows
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    // BỔ SUNG: Slot xử lý sự kiện đăng nhập tập trung (dùng cho cả click chuột và nhấn Enter)
    void handleLogin();

private:
    // Lưu trữ thực thể dịch vụ xác thực tài khoản
    std::shared_ptr<AuthService> m_authService; 

    // BỔ SUNG: Biến hỗ trợ việc di chuyển cửa sổ phẳng frameless
    QPoint m_dragPosition;

    // Các biến giao diện đồ họa giữ nguyên vẹn
    QLabel *lblLeftLogo;
    QLabel *lblSlogan;
    QLineEdit *txtAccount;
    QLineEdit *txtPassword;
    QPushButton *btnLogin;
    QPushButton *btnForgot;
    QPushButton *btnClose;
    QWidget *rightContainer;
    QLabel *lblHello;
    QLabel *lblSubDetails;
    QLabel *lblDoctor;
};

#endif // LOGINDIALOG_H