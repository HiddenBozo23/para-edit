#include <iostream>

#include "para-edit/logger.hpp"

int main() {
    LOG_INFO("this is a message");

    std::cout << Logger::GetLogs()[0].msg << std::endl;
}