#include "SystemUser.h"
#include "ext/bcrypt.h"

bool SystemUser::verifyPassword(const QString& plainPassword) const {
    return bcrypt::validatePassword(plainPassword.toStdString(), this->getPasswordHash().toStdString());
}