#pragma once
#pragma once
#include <fmt/format.h>
#include <string>
#include <iostream>
#include <mutex>
#include <ctime>

enum class LOG_LEVEL {
    FATAL,    // Critical errors that terminate the application
    ERROR,    // Errors that need attention
    WARN,  // Potential issues
    INFO,     // General information
    TRACE   // Detailed debug information
};

class Logger {
public:
    // Log category to group log messages
    class LogCategory {
    public:
        explicit LogCategory(std::string name) : name_(std::move(name)) {}
        
        template<typename... Args>
        void Log(LOG_LEVEL verbosity, fmt::format_string<Args...> format, Args&&... args) const {
            Logger::Get().LogImpl(name_, verbosity, format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void FATAL(fmt::format_string<Args...> format, Args&&... args) const {
            Log(LOG_LEVEL::FATAL, format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void ERROR(fmt::format_string<Args...> format, Args&&... args) const {
            Log(LOG_LEVEL::ERROR, format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void WARN(fmt::format_string<Args...> format, Args&&... args) const {
            Log(LOG_LEVEL::WARN, format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void INFO(fmt::format_string<Args...> format, Args&&... args) const {
            Log(LOG_LEVEL::INFO, format, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void TRACE(fmt::format_string<Args...> format, Args&&... args) const {
            Log(LOG_LEVEL::TRACE, format, std::forward<Args>(args)...);
        }

    private:
        std::string name_;
    };

    // Singleton access
    static Logger& Get() {
        static Logger instance;
        return instance;
    }

    // Set minimum verbosity level for output
    void SetMinVerbosity(LOG_LEVEL verbosity) {
        std::lock_guard<std::mutex> lock(mutex_);
        minVerbosity_ = verbosity;
    }

private:
    // TODO: Make configurable from ini file, to enable ''
    Logger() : minVerbosity_(LOG_LEVEL::INFO) {}

    template<typename... Args>
    void LogImpl(const std::string& category, LOG_LEVEL verbosity, 
                 fmt::format_string<Args...> format, Args&&... args) const {
        if (verbosity > minVerbosity_) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        // Get current time
        std::time_t now = std::time(nullptr);
        std::string timeStr = std::ctime(&now);
        timeStr.pop_back(); // Remove trailing newline

        // Format the message
        std::string verbosityStr = VerbosityToString(verbosity);
        std::string message = fmt::format("[{}] [{}] [{}]: {}", 
            timeStr, category, verbosityStr, fmt::format(format, std::forward<Args>(args)...));

        // Output to console with color coding
        OutputToConsole(verbosity, message);

        // If fatal, throw to mimic UE_LOG's crash behavior
        if (verbosity == LOG_LEVEL::FATAL) {
            throw std::runtime_error(message);
        }
    }

    static std::string VerbosityToString(LOG_LEVEL verbosity) {
        switch (verbosity) {
            case LOG_LEVEL::FATAL:   return "FATAL";
            case LOG_LEVEL::ERROR:   return "ERROR";
            case LOG_LEVEL::WARN: return "WARN";
            case LOG_LEVEL::INFO:    return "INFO";
            case LOG_LEVEL::TRACE: return "TRACE";
            default:                    return "UNKNOWN";
        }
    }

    static void OutputToConsole(LOG_LEVEL verbosity, const std::string& message) {
        // ANSI color codes
        std::string color;
        switch (verbosity) {
            case LOG_LEVEL::FATAL:   color = "\033[31m"; break; // Red
            case LOG_LEVEL::ERROR:   color = "\033[31m"; break; // Red
            case LOG_LEVEL::WARN: color = "\033[33m"; break; // Yellow
            case LOG_LEVEL::INFO:    color = "\033[32m"; break; // Green
            case LOG_LEVEL::TRACE: color = "\033[36m"; break; // Cyan
            default:                    color = "\033[0m";  break; // Default
        }

        std::cout << color << message << "\033[0m" << std::endl;
    }

    mutable std::mutex mutex_;
    LOG_LEVEL minVerbosity_;
};

#define DECLARE_LOG_CATEGORY(CategoryName) \
    static const Logger::LogCategory CategoryName(#CategoryName);

#define OE_LOG(Category, Verbosity, Format, ...) \
    Category.Verbosity(Format, ##__VA_ARGS__);
