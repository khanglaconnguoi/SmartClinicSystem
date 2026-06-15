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