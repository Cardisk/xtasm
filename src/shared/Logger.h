#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>

// Implementing a Logger as a singleton.
class Logger {
    public:
        // Used to log things correctly.
        enum LogType {
            WARN,
            ERR,
            INFO,
            DBG,
        };

        // Deleting copy c'tor.
        explicit Logger(const Logger &other) = delete;
        // Destructor.
        ~Logger() { delete logger; }

        // Used to get a Logger reference.
        static Logger &get_logger() {
            if (!logger) logger = new Logger();
            return *logger;
        }

        // Used to log things to the user.
        void log(LogType type, std::string &msg) {
            switch (type) {
                case LogType::DBG:
                    std::cout << "[DEBUG] " << msg << std::endl;
                    break;
                case LogType::INFO:
                    std::cout << "[INFO] " << msg << std::endl;
                    break;
                case LogType::WARN:
                    std::cerr << "[WARNING] " << msg << std::endl;
                    break;
                case LogType::ERR:
                    std::cerr << "[ERROR] " << msg << std::endl;
                    break;
            }
        }
    private:
        // Default c'tor.
        Logger() = default;
        // shared instance.
        inline static Logger *logger = nullptr;
};

#endif // LOGGER_H
