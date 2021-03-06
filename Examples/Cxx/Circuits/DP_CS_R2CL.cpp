/** Reference Circuits
 *
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

#include <DPsim.h>

using namespace DPsim;
using namespace CPS::DP;
using namespace CPS::DP::Ph1;

int main(int argc, char* argv[]) {
	// Define simulation scenario
	Real timeStep = 0.0001;
	Real finalTime = 0.1;
	String simName = "DP_CS_R2CL";

	// Nodes
	auto n1 = Node::make("n1");
	auto n2 = Node::make("n2");

	// Components
	auto cs = CurrentSource::make("cs");
	cs->setParameters(10);
	auto r1 = Resistor::make("r_1");
	r1->setParameters(1);
	auto c1 = Capacitor::make("c_1");
	c1->setParameters(0.001);
	auto l1 = Inductor::make("l_1");
	l1->setParameters(0.001);
	auto r2 = Resistor::make("r_2");
	r2->setParameters(1);

	// Topology
	cs->connect({ Node::GND, n1 });
	r1->connect({ n1, Node::GND });
	c1->connect({ n1, n2 });
	l1->connect({ n2, Node::GND });
	r2->connect({ n2, Node::GND });

	// Define system topology
	auto sys = SystemTopology(50, SystemNodeList{n1, n2}, SystemComponentList{cs, r1, c1, l1, r2});

	// Logging
	auto logger = DataLogger::make(simName);
	logger->addAttribute("v1", n1->attribute("v"));
	logger->addAttribute("v2", n2->attribute("v"));
	logger->addAttribute("i12", cs->attribute("i_intf"));
	logger->addAttribute("i34", c1->attribute("i_intf"));

	Simulation sim(simName, sys, timeStep, finalTime);
	sim.addLogger(logger);

	sim.run();

	return 0;
}
