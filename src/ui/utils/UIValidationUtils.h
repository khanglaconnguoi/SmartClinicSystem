#pragma once

#include <QLineEdit>
#include <QString>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QVariant>

namespace UIValidationUtils {

    inline void applyFieldValidationStyle(QLineEdit* lineEdit, const QString& errorMessage) {
        if (!lineEdit) return;

        // Store original style if not already stored
        if (lineEdit->property("originalStyle").isNull()) {
            QString currentStyle = lineEdit->styleSheet();
            if (currentStyle.trimmed().isEmpty()) {
                currentStyle = "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; background-color: #FFFFFF; color: #111827; padding: 6px 10px; font-size: 13px; min-height: 30px; }";
            }
            lineEdit->setProperty("originalStyle", currentStyle);
        }

        if (!errorMessage.isEmpty()) {
            // Apply error style (matching Profile.cpp red border/background but with 6px border-radius for Admin dialogs)
            lineEdit->setStyleSheet("QLineEdit { border: 1px solid #FF3B30; border-radius: 6px; background-color: #FFE5E5; padding: 6px 10px; font-size: 13px; color: #111827; min-height: 30px; }");
            lineEdit->setToolTip(errorMessage);
        } else {
            // Restore original style
            QString orig = lineEdit->property("originalStyle").toString();
            if (orig.trimmed().isEmpty()) {
                orig = "QLineEdit { border: 1px solid #D1D5DB; border-radius: 6px; background-color: #FFFFFF; color: #111827; padding: 6px 10px; font-size: 13px; min-height: 30px; }";
            }
            lineEdit->setStyleSheet(orig);
            lineEdit->setToolTip("");
        }
    }

    inline void applyTableFieldValidationStyle(QLineEdit* lineEdit, const QString& errorMessage) {
        if (!lineEdit) return;

        // Store original style if not already stored
        if (lineEdit->property("originalStyle").isNull()) {
            QString currentStyle = lineEdit->styleSheet();
            if (currentStyle.trimmed().isEmpty()) {
                currentStyle = "QLineEdit { border: 1px solid #cbd5e1; border-radius: 4px; padding: 2px 6px; background-color: #ffffff; color: #0f172a; min-height: 24px; }";
            }
            lineEdit->setProperty("originalStyle", currentStyle);
        }

        if (!errorMessage.isEmpty()) {
            lineEdit->setStyleSheet("QLineEdit { border: 1px solid #FF3B30; border-radius: 4px; background-color: #FFE5E5; padding: 2px 6px; color: #0f172a; min-height: 24px; }");
            lineEdit->setToolTip(errorMessage);
        } else {
            // Restore original style
            QString orig = lineEdit->property("originalStyle").toString();
            if (orig.trimmed().isEmpty()) {
                orig = "QLineEdit { border: 1px solid #cbd5e1; border-radius: 4px; padding: 2px 6px; background-color: #ffffff; color: #0f172a; min-height: 24px; }";
            }
            lineEdit->setStyleSheet(orig);
            lineEdit->setToolTip("");
        }
    }

    inline void attachPrimitiveValidators(QLineEdit* txtCitizenId, QLineEdit* txtPhone) {
        if (txtCitizenId) {
            // 12 digits max, only numbers
            txtCitizenId->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\d{0,12}$"), txtCitizenId));
        }
        if (txtPhone) {
            // 10-11 digits max, starting with 0
            txtPhone->setValidator(new QRegularExpressionValidator(QRegularExpression("^0\\d{0,10}$"), txtPhone));
        }
    }

} // namespace UIValidationUtils
