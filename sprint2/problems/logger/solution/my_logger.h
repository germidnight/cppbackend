#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
    auto GetTime() const {
        if (manual_ts_) {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    auto GetTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        return std::put_time(std::localtime(&t_c), "%F %T");
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const {
        const auto now = GetTime();
        const auto t_c_ = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&t_c_), "%Y_%m_%d");
        return oss.str();
    }

    void ChangeLogFile(std::chrono::system_clock::time_point ts) {
        log_file_.close();
        log_file_.open(log_file_prefix + GetFileTimeStamp() + log_file_extension, std::ios::app);
    }

    template <typename T0, typename... Ts>
    void LogImpl(const T0& v0, const Ts&... vs) {
        log_file_ << v0;
        /* оператор ","" вычисляет свои аргументы слева направо и возвращает результат вычисления второго аргумента */
        (..., (log_file_ << vs));
    }

    Logger() {
        log_file_.open(log_file_prefix + GetFileTimeStamp() + log_file_extension, std::ios::app);
    }
    Logger(const Logger&) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;

public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    // Выведите в поток все аргументы.
    template <class... Ts>
    void Log(const Ts&... args) {
        std::lock_guard g(mutex_);
        log_file_ << GetTimeStamp() << ": "sv;
        if constexpr (sizeof...(args) != 0) {
            LogImpl(args...);
        }
        log_file_ << std::endl;
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        std::lock_guard g(mutex_);
        manual_ts_ = ts; // сначала установить метку времени чтобы получить корректное имя файла
        ChangeLogFile(ts);
    }

private:
    const std::string log_file_prefix = "/var/log/sample_log_";
    const std::string log_file_extension = ".log";
    std::mutex mutex_;
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
    std::ofstream log_file_;
};
