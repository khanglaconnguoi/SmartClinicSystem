/**
 * @file    StaffService.h
 * @brief   Business Logic Layer cho Staff
 *
 * VALIDATE STRATEGY:
 *   - Moi field co 1 ham validate rieng (public static) -> UI goi real-time khi editingFinished()
 *   - Validate format truoc, validate uniqueness (can DB) sau
 *   - Aggregate validators (private) gom toan bo truoc khi goi Repo
 *   - Tat ca ham validate tra ve QString: "" = hop le, co gia tri = thong bao loi
 *
 * HIRE FLOW:
 *   UI switch(role) -> goi dung ham hireNew*()
 *   Service: validate -> generateStaffCode -> hashPassword -> build InsertDTO -> Repo
 *
 * UPDATE FLOW:
 *   UI tao StaffUpdateInputDTO -> goi update*()
 *   Service: validate (co excludeStaffId cho unique check) -> build UpdateDTO -> Repo
 */

#pragma once
#include <QList>
#include <QString>
#include <memory>

#include "repository/StaffRepository.h"
#include "repository/DatabaseManager.h"

struct ResetPasswordResult {
    bool result;
    QString newPassword;
};

struct StaffHireResult {
    QString errorMessage;
    QString staffCode;
    QString plainPassword;
};

class StaffService {
private:
    std::shared_ptr<StaffRepository> m_staffRepository;

    // =================================================================
    // AGGREGATE VALIDATORS (private)
    // Gom tat ca field validate truoc khi thuc hien INSERT/UPDATE
    // Goi lan luot tung ham validate don le (co tac dung nhu checklist)
    // =================================================================

    // Validation helpers — trả về chuỗi lỗi rỗng nếu hợp lệ
    QString validateStaffBaseInput(const StaffInputDTO& staff, int staffId = -1);
    QString validateDoctorInput(const DoctorInputDTO& doctor, int staffId = -1);
    QString validateNurseInput(const NurseInputDTO& nurse, int staffId = -1);
    QString validatePharmacistInput(const PharmacistInputDTO& pharmacist, int staffId = -1);

    QString generateStaffCode(UserRole role) const;
    QString generateRandomPassword() const;

    // =================================================================
    // NORMALIZE (private static)
    // Trim all QString fields in-place — called BEFORE validation
    // Insert flow: operate on InputDTO
    // Update flow: operate on UpdateDTO
    // =================================================================
    static void normalizeStaffInput(StaffInputDTO& dto);
    static void normalizeDoctorInput(DoctorInputDTO& dto);
    static void normalizeNurseInput(NurseInputDTO& dto);
    static void normalizePharmacistInput(PharmacistInputDTO& dto);

    // =================================================================
    // MAPPING HELPERS (private static) — Insert flow only
    // Convert a normalized InputDTO into a repo-level InsertDTO.
    // No trimming here — assumes input is already normalized.
    // =================================================================
    static StaffInsertDTO mapStaffToInsertDTO(
            const StaffInputDTO& input,
            const QString& staffCode,
            const QString& passwordHash,
            UserRole role);
    static DoctorInsertDTO mapDoctorToInsertDTO(
            const DoctorInputDTO& input, const QString& staffCode, const QString& passwordHash);
    static NurseInsertDTO mapNurseToInsertDTO(
            const NurseInputDTO& input, const QString& staffCode, const QString& passwordHash);
    static PharmacistInsertDTO mapPharmacistToInsertDTO(
            const PharmacistInputDTO& input, const QString& staffCode, const QString& passwordHash);

    static StaffUpdateDTO mapStaffToUpdateDTO(const StaffInputDTO& dto, int staffId);
    static DoctorUpdateDTO mapDoctorToUpdateDTO(const DoctorInputDTO& dto, int staffId);
    static NurseUpdateDTO mapNurseToUpdateDTO(const NurseInputDTO& dto, int staffId);
    static PharmacistUpdateDTO mapPharmacistToUpdateDTO(const PharmacistInputDTO& dto, int staffId);
   public:
    explicit StaffService(std::shared_ptr<StaffRepository> staffRepository) 
        : m_staffRepository(staffRepository) {}

    // =================================================================
    // FORMAT VALIDATORS — public static
    // Chỉ chứa các hàm validate chuyên biệt cho staff, 
    // các hàm validate thông dụng khác thì dùng bên namespace Validation
    // UI goi real-time tren tung QLineEdit::editingFinished()
    // Khong can DB, khong co side effect.
    // Tra ve: "" = hop le | chuoi loi = khong hop le
    // =================================================================
 
    // -- Giu nguyen (khong sua implementation) ──────────────────────
    
    
    // -- Field chung cho moi role ────────────────────────────────────
    static QString validatePlainPassword(const QString& plainPassword);
    static QString validateDateOfBirth(const QDate& dateOfBirth);
    //static QString validateShift(const QString& shift);
 
    // -- Field dac thu Doctor ────────────────────────────────────────
    static QString validateLicenseNumber(const QString& licenseNumber);
    static QString validateExperienceYears(int experienceYears);
    static QString validateConsultationFee(int consultationFee);
 
    // -- Field dac thu Nurse ─────────────────────────────────────────
    static QString validateNurseLevel(const QString& nurseLevel);

    // -- Field dac thu Pharmacist ────────────────────────────────────
    //static QString validatePharmacySection(const QString& pharmacySection);
 
    // =================================================================
    // UNIQUENESS VALIDATORS — public non-static (can DB)
    // UI goi sau khi format da hop le (editingFinished + format OK).
    // excludeStaffId: dung cho UPDATE — bo qua ban ghi hien tai
    //                 de khong bao "trung" chinh minh.
    //                 Khi INSERT: de mac dinh -1 (khong loai tru ai)
    // =================================================================

    QString validateCitizenIdUnique(const QString& citizenId, int excludeStaffId = -1) const;
    QString validatePhoneNumberUnique(const QString& phoneNumber, int excludeStaffId = -1) const;
    QString validateEmailUnique(const QString& email, int excludeStaffId = -1) const;
    QString validateLicenseNumberUnique(const QString& licenseNumber, int excludeStaffId = -1) const;


    std::unique_ptr<StaffProfileDTO> getOwnProfile(int staffId) const {
        return m_staffRepository->findProfileById(staffId);
    }

    std::unique_ptr<StaffPublicProfileDTO> getPublicProfile(const SystemUser& user) const {
        return user.toPublicProfile();  
    }


    StaffHireResult hireNewDoctor(DoctorInputDTO doctor);
    StaffHireResult hireNewNurse(NurseInputDTO nurse);
    // StaffHireResult hireNewReceptionist(ReceptionistInputDTO receptionist);
    StaffHireResult hireNewPharmacist(PharmacistInputDTO pharmacist);

    QString editStaffBaseInformation(StaffInputDTO staffInformation, int staffId);
    QString editDoctorInformation(DoctorInputDTO doctorInformation, int staffId);
    QString editNurseInformation(NurseInputDTO nurseInformation, int staffId);
    QString editPharmacistInformation(PharmacistInputDTO pharmacistInformation, int staffId);




    bool deactivateStaff(int staffId) {
        return m_staffRepository->deactivate(staffId);
    }
    
    bool reactivateStaff(int staffId) {
        return m_staffRepository->reactivate(staffId);
    }

    
    PagedResult<std::shared_ptr<SystemUser>> searchStaffPaged(const StaffSearchCriteria& criteria) const;
    PagedResult<std::shared_ptr<SystemUser>> searchDoctorsPaged(DoctorSearchCriteria criteria) const;
    PagedResult<std::shared_ptr<SystemUser>> searchNursesPaged(NurseSearchCriteria criteria) const;
    PagedResult<std::shared_ptr<SystemUser>> searchReceptionistsPaged(ReceptionistSearchCriteria criteria) const;
    PagedResult<std::shared_ptr<SystemUser>> searchPharmacistsPaged(PharmacistSearchCriteria criteria) const;



    /*
    QList<std::shared_ptr<SystemUser>> searchDoctors(
        QString searchKey,    
        QString specialty,                  
        int     departmentId,          
        QString shift,                     
        bool onlyActive,
        bool includeDeleted
    ) const;

    QList<std::shared_ptr<SystemUser>> searchNurses(
        QString searchKey,
        int departmentId = -1,
        bool onlyActive = true,
        bool includeDeleted = false
    ) const;

    QList<std::shared_ptr<SystemUser>> searchReceptionists(
        QString searchKey,
        bool onlyActive = true,
        bool includeDeleted = false
    ) const;
    */


    bool changePassword(int staffId, const QString& plainPassword);
    ResetPasswordResult resetPassword(int staffId);
};