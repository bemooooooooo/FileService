#include <iostream>
#include "drogon/HttpAppFramework.h"
#include "controllers/FileController.h"

using namespace drogon;
int main(){
    app().addListener("127.0.0.1",8012).run();
    return 0;
}