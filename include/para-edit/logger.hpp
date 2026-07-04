#pragma once

#include <string>
#include <chrono>   // used for timestamping logs
#include <vector>
#include <sstream>
#include <mutex>
                

// designed to be used as a singleton class via macros

class Logger 
{
public:
    enum class LogLevel {
        debug,
        info,
        warning,
        error,
        fatal
    };

    struct Log {
        std::chrono::system_clock::time_point time;
        std::string file;
        int line;
        LogLevel level;
        std::string msg;
    };

    static void LogMessage( LogLevel lvl, const std::string& msg, 
                            const char* file = "", int line = 0 ) {
        m_GetInstance().m_LogMessage(lvl, msg, file, line);
    }

    // lock on mutex so that if another thread results in a resize,
    // the getLogs thread does not use a bad m_logs reference
    // returns a copy of the logs vector
    static std::vector<Log> GetLogs() {
        std::lock_guard<std::mutex> lock(m_GetInstance().m_mutex);
        return m_GetInstance().m_logs;
    }

private:
    Logger() {};

    static Logger& m_GetInstance() {
        static Logger instance;
        return instance;
    }

    void m_LogMessage( LogLevel lvl, const std::string& msg, 
                            const char* file, int line) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto t = std::chrono::system_clock::now();
        m_logs.push_back({ t, file, line, lvl, msg });
    }

    std::vector<Log> m_logs{};
    std::mutex m_mutex{};
};


// MACROS for ease of use with string streams (<< operator)
#define LOG_DEBUG(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::LogMessage(Looger::LogLevel::debug, oss.str(), __FILE__, __LINE__); \
    } while (0)

#define LOG_INFO(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::LogMessage(Logger::LogLevel::info, oss.str(), __FILE__, __LINE__); \
    } while (0)

#define LOG_WARNING(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::LogMessage(Logger::LogLevel::warning, oss.str(), __FILE__, __LINE__); \
    } while (0)

#define LOG_ERROR(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::LogMessage(Logger::LogLevel::error, oss.str(), __FILE__, __LINE__); \
    } while (0)

#define LOG_FATAL(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Logger::LogMessage(Logger::LogLevel::fatal, oss.str(), __FILE__, __LINE__); \
    } while (0)
