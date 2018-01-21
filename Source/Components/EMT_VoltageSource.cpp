/** Ideal voltage source EMT
*
* @author Markus Mirz <mmirz@eonerc.rwth-aachen.de>
* @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
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

#include "EMT_VoltageSource.h"

using namespace DPsim;

Components::EMT::VoltageSource::VoltageSource(String name, Int node1, Int node2, Real voltageAmp, Real voltagePhase,
	Logger::Level loglevel)	: Component(name, node1, node2, loglevel) {
	mVoltageAmp = voltageAmp;
	mVoltagePhase = voltagePhase;
	mNumVirtualNodes = 1;
	mVirtualNodes = { 0 };
	mVoltage = mVoltageAmp * cos(mVoltagePhase);
	attrMap["voltage"] = { Attribute::Real, &mVoltage };
}

Components::EMT::VoltageSource::VoltageSource(String name, Int node1, Int node2, Complex voltage,
	Logger::Level loglevel) : VoltageSource(name, node1, node2, std::abs(voltage), std::arg(voltage), loglevel) {
}

void Components::EMT::VoltageSource::applySystemMatrixStamp(SystemModel& system) {
	if (mNode1 >= 0) {
		mLog.Log(Logger::Level::DEBUG) << "Add " << -1 << " to " << mNode1 << "," << mVirtualNodes[0] << std::endl;
		system.addRealToSystemMatrix(mNode1, mVirtualNodes[0], -1);
		mLog.Log(Logger::Level::DEBUG) << "Add " << -1 << " to " << mVirtualNodes[0] << "," << mNode1 << std::endl;
		system.addRealToSystemMatrix(mVirtualNodes[0], mNode1, -1);
	}

	if (mNode2 >= 0) {
		mLog.Log(Logger::Level::DEBUG) << "Add " << 1 << " to " << mNode2 << "," << mVirtualNodes[0] << std::endl;
		system.addRealToSystemMatrix(mNode2, mVirtualNodes[0], 1);
		mLog.Log(Logger::Level::DEBUG) << "Add " << 1 << " to " << mVirtualNodes[0] << "," << mNode2 << std::endl;
		system.addRealToSystemMatrix(mVirtualNodes[0], mNode2, 1);
	}
}

void Components::EMT::VoltageSource::applyRightSideVectorStamp(SystemModel& system) {
	mLog.Log(Logger::Level::DEBUG) << "Add " << mVoltage << " to right side " << mVirtualNodes[0] << std::endl;
	system.addRealToRightSideVector(mVirtualNodes[0], mVoltage);
}

void Components::EMT::VoltageSource::step(SystemModel& system, Real time) {
	system.addRealToRightSideVector(mVirtualNodes[0], mVoltage);
}

Complex Components::EMT::VoltageSource::getCurrent(const SystemModel& system) {
	Complex actualcurrent = Complex(system.getRealFromLeftSideVector(mVirtualNodes[0]), 0);
	return Complex(system.getRealFromLeftSideVector(mVirtualNodes[0]),0);
}
