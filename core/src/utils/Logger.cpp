// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "utils/Logger.hpp"
#include <iostream>

namespace ClothSDK {

void Logger::info(const std::string& message) {
    log("INFO", "\033[32m", message);
}

void Logger::warn(const std::string& message) {
    log("WARN", "\033[33m", message);
}

void Logger::error(const std::string& message) {
    log("ERROR", "\033[31m", message);
}

void Logger::log(const std::string& level, const std::string& color, const std::string& message) {
    std::cout   << color                
                << "[ClothSDK]"         
                << "[" << level << "] " 
                << message              
                << "\033[0m"          
                << std::endl;
}
}