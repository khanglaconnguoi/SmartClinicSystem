
#ifndef PROFILE_H
#define PROFILE_H

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class ProfileWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileWidget(QWidget *parent = nullptr);
    ~ProfileWidget() = default;

private slots:
    void onEditClicked();

private:
    QWidget* createTopBar();
    QWidget* createLeftPanel();
    QWidget* createRightPanel();

    QWidget* createInfoCard(const QString &title, const QList<QPair<QString, QString>> &items);
    QWidget* createSecurityCard();

    // Các thành phần cần quản lý con trỏ để cập nhật dữ liệu
    QLabel *lblAvatar;
    QLabel *lblStatus;
    QLabel *lblName;
    QLabel *lblRole;
    QLabel *lblStaffCode;
    QLabel *lblStaffId;
    QLabel *lblShift;

    QPushButton *btnEdit;
    QPushButton *btnExport;
};

#endif // PROFILE_H