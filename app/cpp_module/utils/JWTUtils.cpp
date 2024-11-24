#include "JWTUtils.hpp"
#include <chrono>

const std::string JWTUtils::secret_key = "your-secret-key-here"; // In production, this should be loaded from environment variables

std::string JWTUtils::generateToken(const std::string& user_id) {
    auto token = jwt::create()
        .set_issuer("file_service")
        .set_type("JWS")
        .set_issued_at(std::chrono::system_clock::now())
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{token_expire_hours})
        .set_payload_claim("user_id", jwt::claim(user_id))
        .sign(jwt::algorithm::hs256{secret_key});
    
    return token;
}

bool JWTUtils::verifyToken(const std::string& token, std::string& user_id) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_key})
            .with_issuer("file_service");
        
        verifier.verify(decoded);
        user_id = decoded.get_payload_claim("user_id").as_string();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string JWTUtils::refreshToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_key})
            .with_issuer("file_service");
        
        verifier.verify(decoded);
        
        std::string user_id = decoded.get_payload_claim("user_id").as_string();
        return generateToken(user_id);
    } catch (const std::exception&) {
        return "";
    }
}
