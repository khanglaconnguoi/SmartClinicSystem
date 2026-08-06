#include "AvatarPickerWidget.h"
#include <QFileDialog>
#include <QPainter>
#include <QPainterPath>
#include <QMimeData>
#include <QFileInfo>
#include <QMessageBox>

AvatarPickerWidget::AvatarPickerWidget(QWidget *parent)
    : QLabel(parent), m_hasCustomAvatar(false) {
    setFixedSize(120, 120);
    setAcceptDrops(true);
    setCursor(Qt::PointingHandCursor);
    setAlignment(Qt::AlignCenter);
    
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
    
    // Draw circular background/border
    QRect rect = this->rect();
    QPainterPath path;
    path.addEllipse(rect);
    
    painter.setClipPath(path);
    
    if (m_hasCustomAvatar && !m_avatarPixmap.isNull()) {
        QPixmap scaled = m_avatarPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;
        int y = (height() - scaled.height()) / 2;
        painter.drawPixmap(x, y, scaled);
    } else {
        // Draw placeholder background
        painter.fillPath(path, QColor("#EFF6FF")); // Light blue
        painter.setPen(QPen(QColor("#BFDBFE"), 2, Qt::DashLine));
        painter.drawPath(path);
        
        // Let standard QLabel draw the text
        QLabel::paintEvent(event);
    }
}

void AvatarPickerWidget::updateDefaultText() {
    setText("Chọn ảnh");
    setStyleSheet("QLabel { color: #6B7280; font-size: 13px; font-weight: bold; }");
}
