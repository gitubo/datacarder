#include "../../include/logger/Logger.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace opencmd {
    Logger& Logger::getInstance() {
        static Logger instance;
        return instance;
    }

    Logger::Level Logger::getSeverity() {
        return this->severity;
    }

    void Logger::setSeverity(Level severity) {
        this->severity = severity;
    }

    void Logger::log(const std::string& message, Level level) {
        if (static_cast<unsigned int>(level) < static_cast<unsigned int>(this->severity)) return;

        auto now_seconds = std::chrono::system_clock::now();
        auto now_millis = std::chrono::time_point_cast<std::chrono::microseconds>(now_seconds);
        auto value = now_millis.time_since_epoch().count();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now_seconds);
        auto now_localtime = std::localtime(&now_time);
        std::ostringstream ss;
        ss << std::put_time(now_localtime, "[%Y-%m-%d %H:%M:%S.") << std::setfill('0') << std::setw(6) << (value % 1000000) << "] [";

        switch (level) {
            case Level::DEBUG:
                ss << "debug";
                break;
            case Level::INFO:
                ss << "info";
                break;
            case Level::WARNING:
                ss << "warning";
                break;
            case Level::ERROR:
                ss << "error";
                break;
            case Level::CRITICAL:
                ss << "critical";
                break;
            default:
                ss << "unknown log level";
        }

        ss << "] " << message << std::endl;

        std::cout << ss.str();
    }

    void Logger::debug(const std::string& message) {
        this->log(message, Level::DEBUG);
    }

    void Logger::info(const std::string& message) {
        this->log(message, Level::INFO);
    }

    void Logger::warning(const std::string& message) {
        this->log(message, Level::WARNING);
    }

    void Logger::error(const std::string& message) {
        this->log(message, Level::ERROR);
    }

    void Logger::critical(const std::string& message) {
        this->log(message, Level::CRITICAL);
    }
}