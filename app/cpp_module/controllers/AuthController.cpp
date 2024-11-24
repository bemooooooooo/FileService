#include "AuthController.hpp"
#include <json/json.h>

void AuthController::login(const HttpRequestPtr& req, 
                         std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json) {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value("Invalid JSON"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    std::string username = (*json)["username"].asString();
    std::string password = (*json)["password"].asString();

    // Here you would typically validate against your database
    // This is a simplified example
    if (username == "admin" && password == "password") {
        std::string token = JWTUtils::generateToken(username);
        
        Json::Value result;
        result["token"] = token;
        result["type"] = "Bearer";
        
        auto resp = HttpResponse::newHttpJsonResponse(result);
        callback(resp);
    } else {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value("Invalid credentials"));
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
    }
}

void AuthController::refresh(const HttpRequestPtr& req, 
                           std::function<void(const HttpResponsePtr&)>&& callback) {
    std::string auth_header = req->getHeader("Authorization");
    if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value("No token provided"));
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
        return;
    }

    std::string token = auth_header.substr(7);
    std::string new_token = JWTUtils::refreshToken(token);
    
    if (new_token.empty()) {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value("Invalid token"));
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
        return;
    }

    Json::Value result;
    result["token"] = new_token;
    result["type"] = "Bearer";
    
    auto resp = HttpResponse::newHttpJsonResponse(result);
    callback(resp);
}
