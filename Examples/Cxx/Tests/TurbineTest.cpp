﻿/** Synchron Generator Tests
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


#include "Components/TurbineGovernor.h"

using namespace DPsim;

int main(int argc, char* argv[])
{
	// Define Object for saving data on a file
	Logger TurbineOut("TurbineOutput_DPsim.csv");

	// Define machine parameters in per unit
	Real nomPower = 555e6;

	// Turbine
	Real Ta_t = 0.3;
	Real Fa = 0.3;
	Real Tb = 7;
	Real Fb = 0.3;
	Real Tc = 0.2;
	Real Fc = 0.4;
	Real Tsr = 0.1;
	Real Tsm = 0.3;
	Real Kg = 20;

	Real initActivePower = 300e6;

	DPsim::Components::TurbineGovernor mTurbineGovernor;

	mTurbineGovernor = DPsim::Components::TurbineGovernor(Ta_t, Tb, Tc, Fa, Fb, Fc, Kg, Tsr, Tsm);

	Real OmRef = 1;
	Real PmRef = 300e6/555e6;

	std::string line;
	Real Om;
	std::ifstream omega ("omega.csv");
	Real dt = 0.00005;
	Real t = 0;
	Real Pm = PmRef;

	mTurbineGovernor.initialize(PmRef, initActivePower / nomPower);

	while (getline(omega, line))
	{
		t = t + dt;
		Om = std::stod(line);
		std::cout << Om << '\n';
		Pm = mTurbineGovernor.step(Om, OmRef, PmRef, dt);
		TurbineOut.LogDataLine(t,Pm);
	}

	return 0;
}
