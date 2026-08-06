#include "BillingRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QDate>
#include <QSqlError>
#include <QtDebug>

int BillingRepository::countInvoicesByPrefix(const QString &prefix) {
    DatabaseManager &db = DatabaseManager::getInstance();
    QString sql = "SELECT COUNT(*) FROM invoices WHERE invoice_code LIKE ?";
    QSqlQuery query = db.selectQuery(sql, {prefix + "%"});

    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}


bool BillingRepository::insertInvoice(const InvoiceInsertDTO &dto) {
    DatabaseManager &db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) {
        return false;
    }

    QString sql = R"(
        INSERT INTO invoices (
            invoice_code, patient_id, record_id, patient_type,
            consultation_fee, medication_fee, total_amount, status, issued_date
        )
        VALUES (?,?,?,?,?,?,?,?,?)
    )";

    QVariantList params;
    params << dto.invoiceCode
           << dto.patientId
           << (dto.recordId.has_value() ? QVariant(dto.recordId.value()) : QVariant(QVariant::Int))
           << dto.patientType
           << dto.consultationFee
           << dto.medicationFee
           << dto.totalAmount
           << dto.status
           << dto.issuedDate;

    QSqlQuery query(db.database());
    if (!query.prepare(sql)) {
        db.rollbackTransaction();
        return false;
    }
    for (const QVariant &param : params) {
        query.addBindValue(param);
    }
    if (!query.exec()) {
        db.rollbackTransaction();
        return false;
    }

    int invoiceId = query.lastInsertId().toInt();
    if (invoiceId <= 0) {
        db.rollbackTransaction();
        return false;
    }

    QString itemSql = R"(
        INSERT INTO invoice_items (
            invoice_id, item_type, description, quantity, unit_price, subtotal
        )
        VALUES (?,?,?,?,?,?)
    )";

    for (const InvoiceItemDTO &item : dto.items) {
        QVariantList itemParams;
        itemParams << invoiceId
                   << item.itemType
                   << item.description
                   << item.quantity
                   << item.unitPrice
                   << item.subtotal;

        if (!db.executeQuery(itemSql, itemParams).isActive()) {
            db.rollbackTransaction();
            return false;
        }
    }

    if (!db.commitTransaction()) {
        return false;
    }

    return true;
}

bool BillingRepository::updateInvoice(const InvoiceUpdateDTO &dto) {
    DatabaseManager &db = DatabaseManager::getInstance();
    if (!db.beginTransaction()) {
        return false;
    }

    QString sql = R"(
        UPDATE invoices
        SET consultation_fee = ?, medication_fee = ?, total_amount = ?
        WHERE invoice_id = ?
    )";

    QVariantList params;
    params << dto.consultationFee
           << dto.medicationFee
           << dto.totalAmount
           << dto.invoiceId;

    if (!db.executeQuery(sql, params).isActive()) {
        db.rollbackTransaction();
        return false;
    }

    // Replace items
    QString delItemSql = "DELETE FROM invoice_items WHERE invoice_id = ?";
    if (!db.executeQuery(delItemSql, {dto.invoiceId}).isActive()) {
        db.rollbackTransaction();
        return false;
    }

    QString itemSql = R"(
        INSERT INTO invoice_items (
            invoice_id, item_type, description, quantity, unit_price, subtotal
        )
        VALUES (?,?,?,?,?,?)
    )";

    for (const InvoiceItemDTO &item : dto.items) {
        QVariantList itemParams;
        itemParams << dto.invoiceId
                   << item.itemType
                   << item.description
                   << item.quantity
                   << item.unitPrice
                   << item.subtotal;

        if (!db.executeQuery(itemSql, itemParams).isActive()) {
            db.rollbackTransaction();
            return false;
        }
    }

    return db.commitTransaction();
}

bool BillingRepository::cancelInvoice(int invoiceId) {
    DatabaseManager &db = DatabaseManager::getInstance();
    QString sql = "UPDATE invoices SET status = 'CANCELLED' WHERE invoice_id = ?";
    return db.executeQuery(sql, {invoiceId}).isActive();
}

bool BillingRepository::collectPayment(int invoiceId, double finalAmount) {
    DatabaseManager &db = DatabaseManager::getInstance();
    QString sql;
    QVariantList params;
    QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");
    if (finalAmount >= 0.0) {
        sql = "UPDATE invoices SET status = 'PAID', paid_date = ?, total_amount = ? WHERE invoice_id = ?";
        params << todayStr << finalAmount << invoiceId;
    } else {
        sql = "UPDATE invoices SET status = 'PAID', paid_date = ? WHERE invoice_id = ?";
        params << todayStr << invoiceId;
    }
    return db.executeQuery(sql, params).isActive();
}

std::optional<InvoiceResultDTO> BillingRepository::getInvoiceByRecordId(int recordId) {
    DatabaseManager &db = DatabaseManager::getInstance();
    QString sql = "SELECT * FROM invoices WHERE record_id = ?";
    QSqlQuery query = db.selectQuery(sql, {recordId});

    if (!query.next()) {
        return std::nullopt;
    }

    InvoiceResultDTO dto;
    dto.invoiceId = query.value("invoice_id").toInt();
    dto.invoiceCode = query.value("invoice_code").toString();
    dto.patientId = query.value("patient_id").toInt();
    
    QVariant recId = query.value("record_id");
    dto.recordId = recId.isNull() ? std::nullopt : std::make_optional(recId.toInt());
    
    dto.patientType = patientTypeFromEn(query.value("patient_type").toString());
    dto.consultationFee = query.value("consultation_fee").toDouble();
    dto.medicationFee = query.value("medication_fee").toDouble();
    dto.totalAmount = query.value("total_amount").toDouble();
    dto.status = query.value("status").toString();
    dto.issuedDate = query.value("issued_date").toDate();
    
    QVariant paidDate = query.value("paid_date");
    dto.paidDate = paidDate.isNull() ? std::nullopt : std::make_optional(paidDate.toDate());

    QString itemSql = "SELECT * FROM invoice_items WHERE invoice_id = ?";
    QSqlQuery itemQuery = db.selectQuery(itemSql, {dto.invoiceId});
    
    while (itemQuery.next()) {
        InvoiceItemDTO item;
        item.itemType = itemQuery.value("item_type").toString();
        item.description = itemQuery.value("description").toString();
        item.quantity = itemQuery.value("quantity").toInt();
        item.unitPrice = itemQuery.value("unit_price").toDouble();
        item.subtotal = itemQuery.value("subtotal").toDouble();
        dto.items.append(item);
    }

    return dto;
}

QList<InvoiceResultDTO> BillingRepository::getInvoicesByPatientId(int patientId) {
    DatabaseManager &db = DatabaseManager::getInstance();
    QString sql = "SELECT * FROM invoices WHERE patient_id = ? ORDER BY issued_date DESC";
    QSqlQuery query = db.selectQuery(sql, {patientId});

    QList<InvoiceResultDTO> results;
    
    while (query.next()) {
        InvoiceResultDTO dto;
        dto.invoiceId = query.value("invoice_id").toInt();
        dto.invoiceCode = query.value("invoice_code").toString();
        dto.patientId = query.value("patient_id").toInt();
        
        QVariant recId = query.value("record_id");
        dto.recordId = recId.isNull() ? std::nullopt : std::make_optional(recId.toInt());
        
        dto.patientType = patientTypeFromEn(query.value("patient_type").toString());
        dto.consultationFee = query.value("consultation_fee").toDouble();
        dto.medicationFee = query.value("medication_fee").toDouble();
        dto.totalAmount = query.value("total_amount").toDouble();
        dto.status = query.value("status").toString();
        dto.issuedDate = query.value("issued_date").toDate();
        
        QVariant paidDate = query.value("paid_date");
        dto.paidDate = paidDate.isNull() ? std::nullopt : std::make_optional(paidDate.toDate());

        QString itemSql = "SELECT * FROM invoice_items WHERE invoice_id = ?";
        QSqlQuery itemQuery = db.selectQuery(itemSql, {dto.invoiceId});
        
        while (itemQuery.next()) {
            InvoiceItemDTO item;
            item.itemType = itemQuery.value("item_type").toString();
            item.description = itemQuery.value("description").toString();
            item.quantity = itemQuery.value("quantity").toInt();
            item.unitPrice = itemQuery.value("unit_price").toDouble();
            item.subtotal = itemQuery.value("subtotal").toDouble();
            dto.items.append(item);
        }
        
        results.append(dto);
    }

    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// Search
// ─────────────────────────────────────────────────────────────────────────────

QString BillingRepository::buildSearchWhereClause(
    const InvoiceSearchCriteria &criteria,
    QVariantList &outParams) const {
  QStringList conditions;

  // searchKey: LIKE trên invoice_code.
  if (!criteria.searchKey.trimmed().isEmpty()) {
    conditions << "i.invoice_code LIKE ?";
    QString escaped = criteria.searchKey.trimmed();
    escaped.replace('%', "\\%").replace('_', "\\_");
    outParams << "%" + escaped + "%";
  }

  if (criteria.patientId != -1) {
    conditions << "i.patient_id = ?";
    outParams << criteria.patientId;
  }

  if (!criteria.status.trimmed().isEmpty()) {
    conditions << "i.status = ?";
    outParams << criteria.status.trimmed().toUpper();
  }

  if (criteria.patientType.has_value()) {
    conditions << "i.patient_type = ?";
    outParams << patientTypeToEn(criteria.patientType.value());
  }

  if (criteria.fromDate.has_value() && criteria.fromDate->isValid()) {
    conditions << "i.issued_date >= ?";
    outParams << criteria.fromDate->toString("yyyy-MM-dd");
  }

  if (criteria.toDate.has_value() && criteria.toDate->isValid()) {
    conditions << "i.issued_date <= ?";
    outParams << criteria.toDate->toString("yyyy-MM-dd");
  }

  if (conditions.isEmpty())
    return "1 = 1";

  return conditions.join(" AND ");
}

QList<InvoiceSummaryDTO> BillingRepository::searchInvoices(
    const InvoiceSearchCriteria &criteria) {
  QVariantList params;
  QString whereClause = buildSearchWhereClause(criteria, params);

  const QString sql = QString(R"(
    SELECT i.invoice_id, i.invoice_code, i.patient_id, i.record_id,
           i.patient_type, i.total_amount, i.status,
           i.issued_date, i.paid_date
    FROM   invoices i
    WHERE  %1
    ORDER  BY i.issued_date DESC
    LIMIT  ? OFFSET ?
  )").arg(whereClause);

  params << criteria.limit << criteria.offset;

  QList<InvoiceSummaryDTO> results;
  DatabaseManager &db = DatabaseManager::getInstance();
  QSqlQuery query = db.selectQuery(sql, params);

  if (!query.isActive()) {
    qWarning() << "BillingRepository::searchInvoices - Query thất bại";
    return results;
  }

  while (query.next()) {
    InvoiceSummaryDTO dto;
    dto.invoiceId   = query.value("invoice_id").toInt();
    dto.invoiceCode = query.value("invoice_code").toString();
    dto.patientId   = query.value("patient_id").toInt();

    QVariant recId  = query.value("record_id");
    dto.recordId    = recId.isNull() ? std::nullopt : std::make_optional(recId.toInt());

    dto.patientType  = patientTypeFromEn(query.value("patient_type").toString());
    dto.totalAmount  = query.value("total_amount").toDouble();
    dto.status       = query.value("status").toString();
    dto.issuedDate   = query.value("issued_date").toDate();

    QVariant paidDate = query.value("paid_date");
    dto.paidDate = paidDate.isNull() ? std::nullopt : std::make_optional(paidDate.toDate());

    results.append(dto);
  }
  return results;
}

int BillingRepository::countSearchResults(const InvoiceSearchCriteria &criteria) const {
  QVariantList params;
  QString whereClause = buildSearchWhereClause(criteria, params);

  const QString sql = QString(R"(
    SELECT COUNT(*)
    FROM   invoices i
    WHERE  %1
  )").arg(whereClause);

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (query.next())
    return query.value(0).toInt();
  return 0;
}

PagedResult<InvoiceSummaryDTO> BillingRepository::searchInvoicesPaged(
    const InvoiceSearchCriteria &criteria) const {
  PagedResult<InvoiceSummaryDTO> result;
  result.page = qMax(1, criteria.page);
  result.pageSize = criteria.pageSize;

  result.totalCount = countSearchResults(criteria);

  QVariantList params;
  QString whereClause = buildSearchWhereClause(criteria, params);

  QString sql = QString(R"(
    SELECT i.invoice_id, i.invoice_code, i.patient_id, i.record_id,
           i.patient_type, i.total_amount, i.status,
           i.issued_date, i.paid_date
    FROM   invoices i
    WHERE  %1
    ORDER  BY i.issued_date DESC
  )").arg(whereClause);

  if (criteria.pageSize > 0) {
    int offset = (result.page - 1) * criteria.pageSize;
    sql += " LIMIT ? OFFSET ?";
    params << criteria.pageSize << offset;
  }

  QSqlQuery query = DatabaseManager::getInstance().selectQuery(sql, params);
  if (!query.isActive()) {
    qWarning() << "BillingRepository::searchInvoicesPaged - Query thất bại";
    return result;
  }

  while (query.next()) {
    InvoiceSummaryDTO dto;
    dto.invoiceId   = query.value("invoice_id").toInt();
    dto.invoiceCode = query.value("invoice_code").toString();
    dto.patientId   = query.value("patient_id").toInt();

    QVariant recId  = query.value("record_id");
    dto.recordId    = recId.isNull() ? std::nullopt : std::make_optional(recId.toInt());

    dto.patientType  = patientTypeFromEn(query.value("patient_type").toString());
    dto.totalAmount  = query.value("total_amount").toDouble();
    dto.status       = query.value("status").toString();
    dto.issuedDate   = query.value("issued_date").toDate();

    QVariant paidDate = query.value("paid_date");
    dto.paidDate = paidDate.isNull() ? std::nullopt : std::make_optional(paidDate.toDate());

    result.items.append(dto);
  }

  return result;
}

