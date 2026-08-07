#ifndef GUI_TEST_HELPER_H
#define GUI_TEST_HELPER_H

#include <QtTest/QTest>
#include <QApplication>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QMessageBox>
#include <QTimer>
#include <QSignalSpy>
#include <QWidget>
#include <QString>
#include "../TestHelper.h"

namespace GuiTestHelper {

    /**
     * @brief Tự động đóng bất kỳ QMessageBox nào hiển thị trong khoảng delayMs.
     */
    inline void autoDismissMessageBox(int delayMs = 500) {
        QTimer::singleShot(delayMs, []() {
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                if (auto mb = qobject_cast<QMessageBox*>(widget)) {
                    mb->close();
                }
            }
        });
    }

    /**
     * @brief Tìm và gõ văn bản vào QLineEdit theo objectName hoặc index.
     */
    inline bool typeText(QWidget *parent, const QString &text, int index = 0) {
        auto lineEdits = parent->findChildren<QLineEdit*>();
        if (index >= 0 && index < lineEdits.size()) {
            lineEdits[index]->clear();
            QTest::keyClicks(lineEdits[index], text);
            return true;
        }
        return false;
    }

    /**
     * @brief Click vào QPushButton theo text hiển thị.
     */
    inline bool clickButtonByText(QWidget *parent, const QString &buttonText) {
        auto buttons = parent->findChildren<QPushButton*>();
        for (auto btn : buttons) {
            if (btn->text().contains(buttonText, Qt::CaseInsensitive)) {
                QTest::mouseClick(btn, Qt::LeftButton);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Chọn item trong QComboBox theo text hiển thị.
     */
    inline bool selectComboBoxByText(QComboBox *combo, const QString &text) {
        if (!combo) return false;
        int idx = combo->findText(text, Qt::MatchContains);
        if (idx >= 0) {
            combo->setCurrentIndex(idx);
            return true;
        }
        return false;
    }

} // namespace GuiTestHelper

#endif // GUI_TEST_HELPER_H
