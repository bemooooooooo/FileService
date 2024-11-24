#pragma once

#include <drogon/HttpFilter.h>
#include "../utils/JWTUtils.hpp"

class JWTAuthFilter : public drogon::HttpFilter<JWTAuthFilter> {
public:
    virtual void doFilter(const drogon::HttpRequestPtr& req,
                         drogon::FilterCallback&& fcb,
                         drogon::FilterChainCallback&& fccb) override;
};
