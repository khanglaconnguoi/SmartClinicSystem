#include "SystemUser.h"

UserRole roleFromString(const QString& roleStr){
    if(roleStr == "ADMIN")          return UserRole::Admin;
    if(roleStr == "DOCTOR")         return UserRole::Doctor;
    if(roleStr == "NURSE")          return UserRole::Nurse;
    if(roleStr == "RECEPTIONIST")   return UserRole::Receptionist;
}

QString roleToString(UserRole role){
    switch(role) {
        case UserRole::Admin:         return "ADMIN";
        case UserRole::Doctor:        return "DOCTOR";
        case UserRole::Nurse:         return "NURSE";
        case UserRole::Receptionist:  return "RECEPTIONIST";
    }
}