#ifndef LOGGER_HH
#define LOGGER_HH

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/fmt/ostr.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>

// Singleton Logger class that will log to both the console and a log file
class Logger {
public:
    static void info(const std::string& message) {
        getInstance().logger_->info(message);
    }

    static void debug(const std::string& message) {
        getInstance().logger_->debug(message);
    }

    static void warn(const std::string& message) {
        getInstance().logger_->warn(message);
    }

    static void error(const std::string& message) {
        getInstance().logger_->error(message);
    }

    static void critical(const std::string& message) {
        getInstance().logger_->critical(message);
    }

    static void log(spdlog::level::level_enum level, const std::string& message) {
        getInstance().logger_->log(level, message);
    }

    static void log_timestamp(std::string label) {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();

        std::string timestamp = ss.str();
        getInstance().logger_->trace("{}: {}", label, timestamp);
    }

    // function to enable trace logging. this will also log various timestamps
    static void enable_trace(){
        getInstance().logger_->set_level(spdlog::level::trace);
        getInstance().logger_->flush_on(spdlog::level::trace);
    }

private:
    Logger() {
        // Create log directory if it doesn't exist
        std::filesystem::create_directories("./../logs");

        // Generate timestamped log file name
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d_%H-%M-%S");
        std::string timestamp = ss.str();
        std::string log_filename = "./../logs/log_" + timestamp + ".txt";

        // Create a multi-sink logger (console + file)
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_filename, true);

        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

        std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };

        logger_ = std::make_shared<spdlog::logger>("multi_sink", begin(sinks), end(sinks));
        spdlog::register_logger(logger_);

        logger_->set_level(spdlog::level::debug); // Set to debug to log everything but trace
        logger_->flush_on(spdlog::level::debug);  // Flush on every log
    }

    ~Logger() {
        spdlog::drop_all(); // Cleanup the logger
    }

    std::shared_ptr<spdlog::logger> logger_;

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

#endif // LOGGER_HH