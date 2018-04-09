/** Reference Circuits
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

#include "DPsim.h"

using namespace DPsim;
using namespace CPS::Components::DP;

int main(int argc, char* argv[]) {
	Real timeStep = 0.00005;
	Real omega = 2.0*M_PI*50.0;
	Real finalTime = 0.2;
	String simName = "DP_IdealVS_R_1";

	Component::List comps = {
		VoltageSource::make("v_in", GND, 0, Complex(10, 0)),
		Resistor::make("r_1", 0, GND, 1)
	};

	Simulation sim(simName, comps, omega, timeStep, finalTime);

	sim.run();

	return 0;
}
