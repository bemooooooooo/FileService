#pragma once

#include <jwt-cpp/jwt.h>
#include <string>

class JWTUtils {
private:
    static const std::string secret_key;
    static const int token_expire_hours = 24;

public:
    static std::string generateToken(const std::string& user_id);
    static bool verifyToken(const std::string& token, std::string& user_id);
    static std::string refreshToken(const std::string& token);
};
