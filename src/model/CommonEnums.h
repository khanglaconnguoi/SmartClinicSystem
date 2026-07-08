#pragma once
#include <QString>

enum class Gender { Male, Female, Other };

inline QString genderToString(Gender gender) {
    switch(gender) {
        case Gender::Male:      return "MALE";
        case Gender::Female:    return "FEMALE";
        case Gender::Other:     return "OTHER";
    }
    return "OTHER";
}

inline Gender genderFromString(const QString& genderStr) {
    if(genderStr == "MALE")     return Gender::Male;
    if(genderStr == "FEMALE")   return Gender::Female;
    return Gender::Other;
}


enum class UserRole { Admin, Doctor, Nurse, Receptionist };

inline QString roleToString(UserRole role){
    switch(role) {
        case UserRole::Admin:         return "ADMIN";
        case UserRole::Doctor:        return "DOCTOR";
        case UserRole::Nurse:         return "NURSE";
        case UserRole::Receptionist:  return "RECEPTIONIST";
    }
}

inline UserRole roleFromString(const QString& roleStr){
    if(roleStr == "ADMIN")          return UserRole::Admin;
    if(roleStr == "DOCTOR")         return UserRole::Doctor;
    if(roleStr == "NURSE")          return UserRole::Nurse;
    if(roleStr == "RECEPTIONIST")   return UserRole::Receptionist;
}


