#include "AvatarPickerWidget.h"
#include <QFileDialog>
#include <QPainter>
#include <QPainterPath>
#include <QMimeData>
#include <QFileInfo>
#include <QMessageBox>

AvatarPickerWidget::AvatarPickerWidget(QWidget *parent)
    : QLabel(parent), m_hasCustomAvatar(false) {
    setFixedSize(140, 140);
    setAcceptDrops(true);
    setCursor(Qt::PointingHandCursor);
    setAlignment(Qt::AlignCenter);
    setStyleSheet("QLabel { background: transparent; border: none; }");
    setAttribute(Qt::WA_TranslucentBackground, true);
    
    // Set a tooltip to guide the user
    setToolTip("Nhấn hoặc kéo thả ảnh vào đây để chọn Avatar");
    
    // Initial style
    updateDefaultText();
}

QPixmap AvatarPickerWidget::getAvatarPixmap() const {
    return m_hasCustomAvatar ? m_avatarPixmap : QPixmap();
}

void AvatarPickerWidget::setAvatarPixmap(const QPixmap &pixmap) {
    if (!pixmap.isNull()) {
        m_avatarPixmap = pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_hasCustomAvatar = true;
        setText("");
    } else {
        m_avatarPixmap = QPixmap();
        m_hasCustomAvatar = false;
        updateDefaultText();
    }
    update();
    emit avatarChanged();
}

void AvatarPickerWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (!isEnabled()) {
        event->ignore();
        return;
    }
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QFileInfo fileInfo(urls.first().toLocalFile());
            QString suffix = fileInfo.suffix().toLower();
            if (suffix == "png" || suffix == "jpg" || suffix == "jpeg" || suffix == "bmp") {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void AvatarPickerWidget::dropEvent(QDropEvent *event) {
    if (!isEnabled()) {
        event->ignore();
        return;
    }
    if (event->mimeData()->hasUrls()) {
        QString filePath = event->mimeData()->urls().first().toLocalFile();
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            setAvatarPixmap(pixmap);
        } else {
            QMessageBox::warning(this, "Lỗi", "Không thể tải ảnh. Vui lòng chọn tệp hợp lệ.");
        }
    }
}

void AvatarPickerWidget::mousePressEvent(QMouseEvent *event) {
    if (!isEnabled()) {
        return;
    }
    if (event->button() == Qt::LeftButton) {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            "Chọn Avatar",
            QString(),
            "Images (*.png *.jpg *.jpeg *.bmp)"
        );

        if (!filePath.isEmpty()) {
            QPixmap pixmap(filePath);
            if (!pixmap.isNull()) {
                setAvatarPixmap(pixmap);
            } else {
                QMessageBox::warning(this, "Lỗi", "Không thể tải ảnh. Vui lòng chọn tệp hợp lệ.");
            }
        }
    }
    QLabel::mousePressEvent(event);
}

void AvatarPickerWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    QRect r = this->rect().adjusted(1, 1, -1, -1);
    QPainterPath clipPath;
    clipPath.addEllipse(r);
    
    painter.save();
    painter.setClipPath(clipPath);
    
    if (m_hasCustomAvatar && !m_avatarPixmap.isNull()) {
        QPixmap scaled = m_avatarPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    } else {
        // Draw background
        painter.fillPath(clipPath, QColor("#EFF6FF")); // Light blue
        
        QString t = text();
        if (t.isEmpty() || t == "Chọn ảnh") {
            painter.setPen(QPen(QColor("#93C5FD"), 1.5, Qt::DashLine));
            painter.drawEllipse(r);
            painter.setPen(QColor("#3B82F6"));
            QFont f = painter.font();
            f.setPointSize(11);
            f.setBold(true);
            painter.setFont(f);
            painter.drawText(r, Qt::AlignCenter, "Chọn ảnh");
        } else {
            // Draw initials
            painter.setPen(QColor("#1D4ED8"));
            QFont f = painter.font();
            f.setPointSize(width() / 3);
            f.setBold(true);
            painter.setFont(f);
            painter.drawText(r, Qt::AlignCenter, t);
        }
    }

    if (isEnabled()) {
        QRect overlayRect(0, height() - 34, width(), 34);
        painter.fillRect(overlayRect, QColor(15, 23, 42, 175)); // Dark slate glassmorphism
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(overlayRect, Qt::AlignCenter, "Đổi ảnh");
    }

    painter.restore();

    // Outer border
    painter.setPen(QPen(isEnabled() ? QColor("#3B82F6") : QColor("#E2E8F0"), isEnabled() ? 2.5 : 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(r);
}

void AvatarPickerWidget::updateDefaultText() {
    setText("Chọn ảnh");
    setStyleSheet("QLabel { background: transparent; border: none; color: #3B82F6; font-size: 13px; font-weight: bold; }");
}
