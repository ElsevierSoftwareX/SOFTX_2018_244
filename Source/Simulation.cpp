/**
 * @author Markus Mirz <mmirz@eonerc.rwth-aachen.de>
 * @copyright 2017-2018, Institute for Automation of Complex Power Systems, EONERC
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

#include <dpsim/Simulation.h>
#include <dpsim/MNASolver.h>

#ifdef WITH_CIM
  #include <cps/CIM/Reader.h>
#endif

#ifdef WITH_SUNDIALS
  #include <dpsim/DAESolver.h>
#endif

using namespace CPS;
using namespace DPsim;

Simulation::Simulation(String name,
	Real timeStep, Real finalTime,
	Domain domain, Solver::Type solverType,
	Logger::Level logLevel) :
	mLog(name, logLevel),
	mName(name),
	mFinalTime(finalTime),
	mTimeStep(timeStep),
	mLogLevel(logLevel)
{
	addAttribute<String>("name", &mName, Flags::read);
	addAttribute<Real>("final_time", &mFinalTime, Flags::read);
}

Simulation::Simulation(String name, SystemTopology system,
	Real timeStep, Real finalTime,
	Domain domain, Solver::Type solverType,
	Logger::Level logLevel,
	Bool steadyStateInit) :
	Simulation(name, timeStep, finalTime,
		domain, solverType, logLevel) {

	switch (solverType) {
	case Solver::Type::MNA:
		if (domain == Domain::DP)
			mSolver = std::make_shared<MnaSolver<Complex>>(name, system, timeStep,
				domain, logLevel, steadyStateInit);
		else
			mSolver = std::make_shared<MnaSolver<Real>>(name, system, timeStep,
				domain, logLevel, steadyStateInit);
		break;

#ifdef WITH_SUNDIALS
	case Solver::Type::DAE:
		mSolver = std::make_shared<DAESolver>(name, system, timeStep, 0.0);
		break;
#endif /* WITH_SUNDIALS */

	default:
		throw UnsupportedSolverException();
	}
}

Simulation::~Simulation() {

}

void Simulation::sync()
{
#ifdef WITH_SHMEM
	// We send initial state over all interfaces
	for (auto ifm : mInterfaces) {
		ifm.interface->writeValues();
	}

	std::cout << Logger::prefix() << "Waiting for start synchronization on " << mInterfaces.size() << " interfaces" << std::endl;

	// Blocking wait for interfaces
	for (auto ifm : mInterfaces) {
		ifm.interface->readValues(ifm.syncStart);
	}

	std::cout << Logger::prefix() << "Synchronized simulation start with remotes" << std::endl;
#endif
}

void Simulation::run() {
	mLog.info() << "Opening interfaces." << std::endl;

#ifdef WITH_SHMEM
	for (auto ifm : mInterfaces)
		ifm.interface->open();
#endif

	sync();

	mLog.info() << "Start simulation." << std::endl;

	while (mTime < mFinalTime) {
		step();
	}

#ifdef WITH_SHMEM
	for (auto ifm : mInterfaces)
		ifm.interface->close();
#endif

	for (auto lg : mLoggers)
		lg.logger->flush();

	mLog.info() << "Simulation finished." << std::endl;
}

Real Simulation::step() {
	Real nextTime;

#ifdef WITH_SHMEM
	for (auto ifm : mInterfaces) {
		if (mTimeStepCount % ifm.downsampling == 0)
			ifm.interface->readValues(ifm.sync);
	}
#endif

	mEvents.handleEvents(mTime);

	nextTime = mSolver->step(mTime);
	mSolver->log(mTime);

#ifdef WITH_SHMEM
	for (auto ifm : mInterfaces) {
		if (mTimeStepCount % ifm.downsampling == 0)
			ifm.interface->writeValues();
	}
#endif

	for (auto lg : mLoggers) {
		if (mTimeStepCount % lg.downsampling == 0) {
			lg.logger->log(mTime);
        }
	}

	mTime = nextTime;
	mTimeStepCount++;

	return mTime;
}
