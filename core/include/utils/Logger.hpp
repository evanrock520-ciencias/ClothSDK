#pragma once

#include <string>

namespace ClothSDK {

class Logger {
public:

    static void info(const std::string&);
    static void warn(const std::string&);
    static void error(const std::string&);

private:

    static void log(const std::string& level, const std::string& color, const std::string& message);
};

}