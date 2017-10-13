/** Logger
 *
 * @file
 * @author Markus Mirz <mmirz@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * DPsim
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#pragma once

#include <iostream>
#include <fstream>

#include "MathLibrary.h"

namespace DPsim {

	enum class LogLevel { NONE, ERROR, WARN, INFO };

	class Logger {
	private:
		std::ofstream mLogFile;
		LogLevel mLogLevel;

		static std::ostringstream nullStream;
		static std::ostream& getNullStream();

	public:
		Logger();
		Logger(std::string filename, LogLevel level = LogLevel::INFO);
		~Logger();

		std::ostream& Log(LogLevel level = LogLevel::INFO);
		void LogDataLine(double time, Matrix& data);
		void LogDataLine(double time, double data);
	};
}

