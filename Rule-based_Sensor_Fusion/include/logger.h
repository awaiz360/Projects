#ifndef TRACKER_LOGGER_H_
#define TRACKER_LOGGER_H_

#include <fstream>
#include <iostream>
#include <string>

namespace tracker {

class Logger {
 public:
  enum Level { FATAL, ERROR, WARNING, INFO, DEBUG };

  static Logger& GetInstance() {
    static Logger instance;
    return instance;
  }

  void Log(Level level, const std::string& message) {
    if (level <= logLevel_) {
      std::cerr << "[" << GetLevelString(level) << "] " << message << std::endl;

      if (level == FATAL) {
        std::ofstream logFile("fatal_log.txt", std::ios_base::app);
        logFile << "[" << GetLevelString(level) << "] " << message << std::endl;
        logFile.close();  // Close the log file immediately to ensure data is
                          // written

        // Terminate the application (optional)
        std::terminate();
      }
    }
  }

 private:
  Logger() : logLevel_(Level::INFO) {}  // Set default log level

  std::string GetLevelString(Level level) const {
    switch (level) {
      case FATAL:
        return "FATAL";
      case ERROR:
        return "ERROR";
      case WARNING:
        return "WARNING";
      case INFO:
        return "INFO";
      case DEBUG:
        return "DEBUG";
    }
    return "UNKNOWN";
  }

  Level logLevel_;
};
}  // namespace tracker

#endif  // TRACKER_LOGGER_H_
