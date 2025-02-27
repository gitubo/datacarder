#pragma once

#include <chrono>
#include <fstream>
#include <string>

namespace opencmd {
    class Logger {
	 	public:
			enum class Level {
	   			DEBUG,
	   			INFO,
	   			WARNING,
	   			ERROR,
	   			CRITICAL
	   		};

			static Logger& getInstance();

   			Level getSeverity();
			void setSeverity(Level severity);

	  		void log(const std::string& message, Level level);
	  		void debug(const std::string& message);
	  		void info(const std::string& message);
	  		void warning(const std::string& message);
	  		void error(const std::string& message);
	  		void critical(const std::string& message);

		private:
	   		Logger() = default;
			Logger(const Logger&) = delete;
	  		Logger& operator=(const Logger&) = delete;
			Level severity = Level::DEBUG;
    };
}
