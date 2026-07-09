#include "StringSanitize.h"

QString StringSanitize::collapseSpaces(const QString& rawString) {
    QStringList words = rawString.trimmed().split(' ', Qt::SkipEmptyParts);
    return words.join(" ");
}