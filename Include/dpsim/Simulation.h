/*********************************************************************************
* @file
* @author Markus Mirz <mmirz@eonerc.rwth-aachen.de>
* @copyright 2017-2018, Institute for Automation of Complex Power Systems, EONERC
*
* CPowerSystems
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
#include <vector>
#include <list>
#include <cstdint>

#include <dpsim/Config.h>
#include <dpsim/MNASolver.h>
//#include <dpsim/DAESolver.h>
#include <cps/Definitions.h>
#include <cps/PowerComponent.h>
#include <cps/Logger.h>
#include <cps/SystemTopology.h>
#include <cps/Node.h>

#ifdef WITH_SHMEM
  #include <cps/Interface.h>
#endif

namespace DPsim {

	class Simulation : public CPS::AttributeList {
	public:
		enum class Event : std::uint32_t {
			Started = 1,
			Stopped = 2,
			Finished = 3,
			Overrun = 4,
			Paused = 5,
			Resumed = 6
		};

	protected:
		/// Simulation logger
		CPS::Logger mLog;
		/// Simulation name
		String mName;
		/// Final time of the simulation
		Real mFinalTime;
		/// Time variable that is incremented at every step
		Real mTime = 0;
		/// Number of step which have been executed for this simulation.
		Int mTimeStepCount = 0;
		/// Simulation log level
		CPS::Logger::Level mLogLevel;
		///
		Solver::Type mSolverType;
		///
		std::shared_ptr<Solver> mSolver;
		/// Pipe for asynchronous inter-process communication (IPC) to the Python world
		int mPipe[2];

#ifdef WITH_SHMEM
		struct InterfaceMapping {
			/// A pointer to the external interface
			CPS::Interface *interface;
			///
			bool sync;
			bool syncStart;
		};

		/// Vector of Interfaces
		std::vector<InterfaceMapping> mInterfaces;
		/// Interfaces are initialized
		bool mInit = false;
#endif

	public:
		/// Creates system matrix according to
		Simulation(String name,
			Real timeStep, Real finalTime,
			CPS::Domain domain = CPS::Domain::DP,
			Solver::Type solverType = Solver::Type::MNA,
			CPS::Logger::Level logLevel = CPS::Logger::Level::INFO);
		/// Creates system matrix according to System topology
		Simulation(String name, CPS::SystemTopology system,
			Real timeStep, Real finalTime,
			CPS::Domain domain = CPS::Domain::DP,
			Solver::Type solverType = Solver::Type::MNA,
			CPS::Logger::Level logLevel = CPS::Logger::Level::INFO,
			Bool steadyStateInit = false);
		///
		virtual ~Simulation();

		/// Run simulation until total time is elapsed.
		void run();
		/// Solve system A * x = z for x and current time
		Real step();

		///
		void setSwitchTime(Real switchTime, Int systemIndex);
#ifdef WITH_SHMEM
		///
		void addInterface(CPS::Interface *eint, bool sync, bool syncStart) {
			mInterfaces.push_back({eint, sync, syncStart});
		}

		void addInterface(CPS::Interface *eint, bool sync = true) {
			mInterfaces.push_back({eint, sync, sync});
		}
#endif
		///
		void addSystemTopology(CPS::SystemTopology system);
		///
		void setLogDownsamplingRate(Int divider) {}

		// #### Getter ####
		String name() const { return mName; }
		Real time() const { return mTime; }
		Real finalTime() const { return mFinalTime; }
		Int timeStepCount() const { return mTimeStepCount; }
		int eventFD(Int flags = -1, Int coalesce = 1);

		/// Sends a notification to other processes / Python
		void sendEvent(enum Event evt);
	};

}
