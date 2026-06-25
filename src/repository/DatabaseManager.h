#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariantList>

class DatabaseManager {
private:
    DatabaseManager();
    ~DatabaseManager() { if (m_db.isOpen()) m_db.close(); }

    bool initializeDatabase();
    bool createTables();

    QSqlDatabase m_db;

public:
    static DatabaseManager& getInstance(){
        static DatabaseManager instance;
        return instance;
    }

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool        executeQuery(const QString& sql, const QVariantList& params = {});
    QSqlQuery   selectQuery(const QString& sql, const QVariantList& params = {});
    
    bool        beginTransaction() { 
        if (!m_db.transaction()) {
            return false;
        } 
        return true;
    }
    
    bool        commitTransaction() { 
        if (!m_db.commit()) {
            m_db.rollback();
            return false;
        }
        return true;
    }

    bool rollbackTransaction() { return m_db.rollback(); }
};