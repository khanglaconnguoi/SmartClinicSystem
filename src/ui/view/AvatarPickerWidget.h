#pragma once

#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QPaintEvent>

class AvatarPickerWidget : public QLabel {
    Q_OBJECT
public:
    explicit AvatarPickerWidget(QWidget *parent = nullptr);
    ~AvatarPickerWidget() override = default;

    QPixmap getAvatarPixmap() const;
    void setAvatarPixmap(const QPixmap &pixmap);

signals:
    void avatarChanged();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateDefaultText();

    QPixmap m_avatarPixmap;
    bool m_hasCustomAvatar;
};
