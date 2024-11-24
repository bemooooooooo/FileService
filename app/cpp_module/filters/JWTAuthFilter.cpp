#include "JWTAuthFilter.hpp"
#include <json/json.h>

void JWTAuthFilter::doFilter(const drogon::HttpRequestPtr& req,
                           drogon::FilterCallback&& fcb,
                           drogon::FilterChainCallback&& fccb) {
    std::string auth_header = req->getHeader("Authorization");
    
    if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        Json::Value result;
        result["message"] = "No token provided";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(drogon::k401Unauthorized);
        fcb(resp);
        return;
    }

    std::string token = auth_header.substr(7);
    std::string user_id;
    
    if (!JWTUtils::verifyToken(token, user_id)) {
        Json::Value result;
        result["message"] = "Invalid token";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
        resp->setStatusCode(drogon::k401Unauthorized);
        fcb(resp);
        return;
    }

    // Add user_id to request attributes for later use
    req->addAttribute("user_id", user_id);
    fccb();
}
