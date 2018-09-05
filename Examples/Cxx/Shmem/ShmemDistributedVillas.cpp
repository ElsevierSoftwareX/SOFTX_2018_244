/** Example of shared memory interface
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

#include <DPsim.h>

using namespace DPsim;
using namespace CPS::DP;
using namespace CPS::DP::Ph1;

int main(int argc, char *argv[])
{
	SystemComponentList comps, comps2;
	SystemNodeList nodes;

	CPS::Interface::Config conf;
	conf.samplelen = 4;
	conf.queuelen = 1024;
	conf.polling = true;

	if (argc < 2) {
		std::cerr << "Not enough arguments (either 0 or 1 for the test number)" << std::endl;
		std::exit(1);
	}

	String in, out;

	if (String(argv[1]) == "0") {
		in  = "/villas0-in";
		out = "/villas0-out";
	}
	else if (String(argv[1]) == "1") {
		in  = "/villas1-in";
		out = "/villas1-out";
	}

	auto intf = CPS::Interface(in, out, &conf);

	if (String(argv[1]) == "0") {
		// Nodes
		auto n1 = Node::make("n1");
		auto n2 = Node::make("n2");
		auto n3 = Node::make("n3");

		// Components
		auto evs = VoltageSource::make("v_t");
		auto vs = VoltageSourceNorton::make("v_s");
		auto l1 = Inductor::make("l_1");
		auto r1 = Resistor::make("r_1");

		// Topology
		evs->connect({ Node::GND, n3 });
		vs->connect({ Node::GND, n1 });
		l1->connect({ n1, n2 });
		r1->connect({ n2, n3 });

		// Parameters
		evs->setParameters(Complex(0, 0));
		vs->setParameters(Complex(10000, 0), 1);
		l1->setParameters(0.1);
		r1->setParameters(1);

		comps = SystemComponentList{evs, vs, l1, r1};
		nodes = SystemNodeList{Node::GND, n1, n2, n3};

		intf.addImport(evs->findAttribute<Complex>("voltage_ref"), 1.0, 0, 1);
		intf.addExport(evs->findAttribute<Complex>("comp_current"), 1.0, 0, 1);
	}
	else if (String(argv[1]) == "1") {
		// Nodes
		auto n4 = Node::make("n4");

		// Components
		auto ecs = CurrentSource::make("v_s");
		auto r2A = Resistor::make("r_2");
		auto r2B = Resistor::make("r_2");

		// Topology
		ecs->connect({ Node::GND, n4 });
		r2A->connect({ Node::GND, n4 });
		r2B->connect({ Node::GND, n4 });

		// Parameters
		ecs->setParameters(Complex(0, 0));
		r2A->setParameters(10);
		r2B->setParameters(8);

		comps = SystemComponentList{ecs, r2A};
		comps2 = SystemComponentList{ecs, r2B};
		nodes = SystemNodeList{Node::GND, n4};

		intf.addImport(ecs->findAttribute<Complex>("current_ref"), 1.0, 0, 1);
		intf.addExport(ecs->findAttribute<Complex>("comp_voltage"), 1.0, 0, 1);
	}
	else {
		std::cerr << "invalid test number" << std::endl;
		std::exit(1);
	}

	String simName = "ShmemDistributed";
	Real timeStep = 0.001;

	auto sys1 = SystemTopology(50, nodes, comps);

	auto sim = RealTimeSimulation(simName + argv[1], sys1, timeStep, 20);
	sim.addInterface(&intf);

	if (String(argv[1]) == "1") {
		auto sys2 = SystemTopology(50, comps2);

		sim.addSystemTopology(sys2);
		sim.setSwitchTime(10, 1);
	}

	sim.run();

	return 0;
}
