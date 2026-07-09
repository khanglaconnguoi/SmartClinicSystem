#pragma once

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariantList>
#include <optional>

class DatabaseManager {
private:
    // 3. Đưa constructor và destructor vào private
    DatabaseManager();
    ~DatabaseManager() { if (m_db.isOpen()) m_db.close(); }

    // Hàm khởi tạo database và tạo bảng
    bool initializeDatabase();
    bool createTables();

    QSqlDatabase m_db;

public:
    // 1. Điểm truy cập toàn cục duy nhất (Dấu hiệu của Singleton Pattern)
    static DatabaseManager& getInstance(){
        static DatabaseManager instance;
        return instance;
    }

    // 2. Xóa tính năng copy để đảm bảo chỉ có 1 Object duy nhất tồn tại
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    struct AppointmentRecord {
        int appointmentId;
        int patientId;
        QString doctorId;
        QString appointmentDate;
        QString startTime;
        QString endTime;
        QString status;
        QString reason;
        QString notes;
        QString patientName;
        QString patientCode;
        QString roomNumber;
    };

    struct PatientRecord {
        int patientId;
        QString patientCode;
        QString fullName;
        QString phone;
    };

    QList<AppointmentRecord> getDoctorAppointments(const QString &doctorId, const QString &date = "");
    bool updateAppointmentStatus(int appointmentId, const QString &status);
    std::optional<PatientRecord> getPatientByPhoneOrCitizenId(const QString &phone, const QString &citizenId);
    bool createAppointment(int patientId, const QString &doctorCode, int createdBy, const QString &date, const QString &startTime, const QString &reason);

    QSqlQuery   executeQuery(const QString& sql, const QVariantList& params = {});
    QSqlQuery   selectQuery(const QString& sql, const QVariantList& params = {});
    bool        beginTransaction() { 
                if (!m_db.transaction()) {
                    //qDebug() << "Không thể mở transaction:" << m_db.lastError().text();
                    return false;
                } 
                return true;
    }
    bool        commitTransaction() { 
                if (!m_db.commit()) {
                    //qDebug() << "Ghi dữ liệu thất bại:" << m_db.lastError().text();
                    m_db.rollback();
                    return false;
                }
                return true;
    }
    bool        rollbackTransaction() { return m_db.rollback(); }
};
