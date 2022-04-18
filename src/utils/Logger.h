//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_LOGGER_H
#define RAMPACK_LOGGER_H

#include <ostream>
#include <utility>

/**
 * @brief A simple class for logging with log type info and date.
 * @details The behaviour can be best seen in the test: LoggerTest.cpp.
 */

class Logger {
public:
    enum LogType {
        ERROR,
        WARN,
        INFO,
        VERBOSE,
        DEBUG
    };

private:
    std::vector<std::reference_wrapper<std::ostream>> outs{};
    std::vector<bool> afterNewlines = {true};
    LogType currentLogType = INFO;
    std::vector<LogType> maxLogTypes = {INFO};
    std::string additionalText;

    Logger &changeLogType(LogType newLogType) {
        if (this->currentLogType == newLogType)
            return *this;

        this->currentLogType = newLogType;

        for (std::size_t i{}; i < this->outs.size(); i++) {
            std::vector<bool>::reference afterNewline = this->afterNewlines[i];
            auto &out = this->outs[i].get();

            if (!afterNewline) {
                out << std::endl;
                afterNewline = true;
            }
        }

        return *this;
    }

    [[nodiscard]] std::string logTypeText() const {
        switch (this->currentLogType) {
            case INFO:
                return "   INFO";
            case WARN:
                return "   WARN";
            case ERROR:
                return "  ERROR";
            case VERBOSE:
                return "VERBOSE";
            case DEBUG:
                return "  DEBUG";
        }
        return "";
    }

protected:
    /**
     * @brief This method can be overriden for a custom timestamp.
     * @details Default is year-month-day hour:minute:second.
     */
    [[nodiscard]] virtual std::string currentDateTime() const {
        time_t     now = time(nullptr);
        struct tm  tstruct{};
        char       buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

        return buf;
    }

public:
    using StreamManipulator = std::ostream&(*)(std::ostream&);

    explicit Logger(std::ostream &out) : outs{out} { }

    /**
     * @brief If set to non-empty, after [log type] [date and time] there will be [@a additionalText_].
     */
    void setAdditionalText(std::string additionalText_) { this->additionalText = std::move(additionalText_); }

    [[nodiscard]] const std::string &getAdditionalText() const { return this->additionalText; }

    /**
     * @brief Sets the log type with maximal verbosity level, which should be displayed.
     * @brief Verbosity levels are given by LogType in an ascending order, with LogType::ERROR being the least verbose
     * and LogType::DEBUG being the most verbose. Default value is LogType::INFO.
     */
    void setVerbosityLevel(LogType maxLogType_) {
        for (auto &maxLogType : this->maxLogTypes)
            maxLogType = maxLogType_;
    }

    operator std::ostream&() { return this->outs.front(); }
    std::ostream &raw() { return this->outs.front(); }

    Logger &info() { return this->changeLogType(INFO); }
    Logger &warn() { return this->changeLogType(WARN); }
    Logger &error() { return this->changeLogType(ERROR); }
    Logger &verbose() { return this->changeLogType(VERBOSE); }
    Logger &debug() { return this->changeLogType(DEBUG); }

    template<typename T>
    Logger &operator<<(T &&t) {
        for (std::size_t i{}; i < this->outs.size(); i++) {
            const auto &maxLogType = this->maxLogTypes[i];
            std::vector<bool>::reference afterNewline = this->afterNewlines[i];
            auto &out = this->outs[i].get();

            if (this->currentLogType > maxLogType)
                continue;

            if (afterNewline) {
                out << "[" << this->logTypeText() << "] [" << this->currentDateTime() << "] ";
                if (!this->additionalText.empty())
                    out << "[" << this->additionalText << "] ";
            }

            afterNewline = false;
            out << std::forward<T>(t);
        }

        return *this;
    }

    Logger &operator<<(StreamManipulator manipulator) {
        for (std::size_t i{}; i < this->outs.size(); i++) {
            const auto &maxLogType = this->maxLogTypes[i];
            std::vector<bool>::reference afterNewline = this->afterNewlines[i];
            auto &out = this->outs[i].get();

            if (this->currentLogType > maxLogType)
                continue;

            if (manipulator == static_cast<StreamManipulator>(std::endl))
                afterNewline = true;

            manipulator(out);
        }

        return *this;
    }
};

#endif //RAMPACK_LOGGER_H
