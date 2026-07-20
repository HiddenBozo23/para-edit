#pragma once

#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace para {

// designed to be used as a singleton class via macros

class Logger {
   public:
    enum LogLevel : uint32_t {
        debug = 1 << 0,
        info = 1 << 1,
        warning = 1 << 2,
        error = 1 << 3,
        fatal = 1 << 4
    };

    struct Log {
        std::string timestamp;
        std::string file;
        int line;
        LogLevel level;
        std::string msg;
    };

    static void LogMessage(LogLevel lvl, const std::string& msg,
                           const char* file = "", int line = 0) {
        m_GetInstance().m_LogMessage(lvl, msg, file, line);
    }

    // lock on mutex so that if another thread results in a resize,
    // the getLogs thread does not use a bad m_logs reference
    // returns a copy of the logs vector
    static std::vector<Log> GetLogs() {
        std::lock_guard<std::mutex> lock(m_GetInstance().m_mutex);
        return m_GetInstance().m_logs;
    }

    static void ClearLogs() {
        m_GetInstance().m_logs.clear();
    }

   private:
    Logger() {};

    static Logger& m_GetInstance() {
        static Logger instance;
        return instance;
    }

    void m_LogMessage(LogLevel lvl, const std::string& msg,
                      const char* file, int line) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_logs.push_back({m_GetTimestamp(), file, line, lvl, msg});
    }

    static std::string m_GetTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &now);
#else
        localtime_r(&now, &tm);
#endif
        char buf[16];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
        return std::string(buf);
    }

    std::vector<Log> m_logs{};
    std::mutex m_mutex{};
};

}  // namespace para

// MACROS for ease of use with string streams (<< operator)
#define LOG_DEBUG(msg)                                                                          \
    do {                                                                                        \
        std::ostringstream oss;                                                                 \
        oss << msg;                                                                             \
        para::Logger::LogMessage(para::Logger::LogLevel::debug, oss.str(), __FILE__, __LINE__); \
    } while (0)

#define LOG_INFO(msg)                                                                          \
    do {                                                                                       \
        std::ostringstream oss;                                                                \
        oss << msg;                                                                            \
        para::Logger::LogMessage(para::Logger::LogLevel::info, oss.str(), __FILE__, __LINE__); \
    } while (0)

#define LOG_WARNING(msg)                                                                          \
    do {                                                                                          \
        std::ostringstream oss;                                                                   \
        oss << msg;                                                                               \
        para::Logger::LogMessage(para::Logger::LogLevel::warning, oss.str(), __FILE__, __LINE__); \
    } while (0)

#define LOG_ERROR(msg)                                                                          \
    do {                                                                                        \
        std::ostringstream oss;                                                                 \
        oss << msg;                                                                             \
        para::Logger::LogMessage(para::Logger::LogLevel::error, oss.str(), __FILE__, __LINE__); \
    } while (0)

#define LOG_FATAL(msg)                                                                          \
    do {                                                                                        \
        std::ostringstream oss;                                                                 \
        oss << msg;                                                                             \
        para::Logger::LogMessage(para::Logger::LogLevel::fatal, oss.str(), __FILE__, __LINE__); \
    } while (0)
