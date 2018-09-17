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

#include "DPsim.h"

using namespace DPsim;
using namespace CPS::DP;
using namespace CPS::DP::Ph1;

int main(int argc, char* argv[]) {
	// Nodes
	auto n1 = Node::make("n1");
	auto n2 = Node::make("n2");

	// Components
	auto cs = CurrentSource::make("cs");
	cs->setParameters(10);
	cs->setNodes(Node::List{ Node::GND, n1 });
	auto r1 = Resistor::make("r_1");
	r1->setParameters(1);
	r1->setNodes(Node::List{ n1, Node::GND });
	auto c1 = Capacitor::make("c_1");
	c1->setParameters(0.001);
	c1->setNodes(Node::List{ n1, n2 });
	auto l1 = Inductor::make("l_1");
	l1->setParameters(0.001);
	l1->setNodes(Node::List{ n2, Node::GND });
	auto r2 = Resistor::make("r_2");
	r2->setParameters(1);
	r2->setNodes(Node::List{ n2, Node::GND });

	// Define system topology
	auto sys = SystemTopology(50, SystemNodeList{n1, n2}, SystemComponentList{cs, r1, c1, l1, r2});
		
	// Define simulation scenario
	Real timeStep = 0.0001;
	Real finalTime = 0.1;
	String simName = "DP_CS_R2CL";

	Simulation sim(simName, sys, timeStep, finalTime);
	sim.run();

	return 0;
}
