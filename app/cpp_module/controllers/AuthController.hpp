#pragma once

#include <drogon/HttpController.h>
#include "../utils/JWTUtils.hpp"

using namespace drogon;

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthController::login, "/api/auth/login", Post);
    ADD_METHOD_TO(AuthController::refresh, "/api/auth/refresh", Post);
    METHOD_LIST_END

    void login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void refresh(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
};
