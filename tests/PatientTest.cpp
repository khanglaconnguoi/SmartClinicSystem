#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "model/OutPatient.h"
#include "model/MedicalRecord.h"
#include "repository/DatabaseManager.h"
#include "repository/PatientRepository.h"
#include "service/PatientService.h"

class PatientTest : public QObject {
    Q_OBJECT

private:
    std::shared_ptr<PatientService> m_service;

private slots:
    void initTestCase() {
        // Khởi tạo DatabaseManager sẽ tự tạo database và tables
        DatabaseManager::getInstance();
        
        // Đảm bảo bảng trống trước khi test (tuỳ chọn)
        DatabaseManager::getInstance().executeQuery("DELETE FROM medical_records");
        DatabaseManager::getInstance().executeQuery("DELETE FROM patients");

        auto repo = std::make_shared<PatientRepository>();
        m_service = std::make_shared<PatientService>(repo);
    }

    void testGeneratePatientCode() {
        QString code = Patient::generatePatientCode();
        QVERIFY(code.startsWith("BN-"));
        QCOMPARE(code.length(), 11); // "BN-" (3) + 8 chars = 11
    }

    void testHasAllergy() {
        OutPatient p;
        p.setAllergies("Penicillin, Aspirin");
        QVERIFY(p.hasAllergy("Penicillin"));
        QVERIFY(p.hasAllergy("Aspirin"));
        QVERIFY(!p.hasAllergy("Paracetamol"));
    }

    void testPatientServiceCRUD() {
        auto p = std::make_shared<OutPatient>();
        p->setFullName("Nguyen Van Test");
        p->setPhoneNumber("0123456789");
        p->setMedicalHistory("Cao huyết áp, Tiểu đường");
        p->setCitizenId("079012345678");
        
        // Test Add
        bool added = m_service->addPatient(p);
        QVERIFY(added);
        QVERIFY(p->id() > 0);
        QVERIFY(!p->patientCode().isEmpty());

        // Test Retrieve
        auto retrieved = m_service->getPatient(p->id());
        QVERIFY(retrieved != nullptr);
        QCOMPARE(retrieved->fullName(), QString("Nguyen Van Test"));
        QCOMPARE(retrieved->medicalHistory(), QString("Cao huyết áp, Tiểu đường"));
        QCOMPARE(retrieved->citizenId(), QString("079012345678"));

        // Test Search by Citizen ID
        PatientSearchCriteria criteria;
        criteria.citizenId = "079012345678";
        auto results = m_service->searchPatients(criteria);
        QVERIFY(results.size() >= 1);
        QCOMPARE(results[0]->citizenId(), QString("079012345678"));
    }

    void testMedicalRecord() {
        // Lấy một bệnh nhân để test
        auto patients = m_service->getAllPatients();
        QVERIFY(!patients.empty());
        int pid = patients[0]->id();

        MedicalRecord record(pid, 1);
        record.setClinicalNotes("Bệnh nhân kêu đau đầu");
        record.setTestResults("Đường huyết: 6.5 mmol/L");
        
        // Test Add Medical Record
        bool added = m_service->addMedicalRecord(record);
        QVERIFY(added);

        // Test Retrieve
        auto records = m_service->getMedicalRecords(pid);
        QVERIFY(records.size() >= 1);
        QCOMPARE(records[0].getTestResults(), QString("Đường huyết: 6.5 mmol/L"));
    }
};

QTEST_MAIN(PatientTest)
#include "PatientTest.moc"
