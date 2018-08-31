/** CIM Test
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

#include <iostream>
#include <list>

#include "DPsim.h"

using namespace DPsim;
using namespace CPS;
using namespace CPS;
using namespace CPS::DP::Ph1;
using namespace CPS::Signal;

int main(int argc, char *argv[]) {

	CommandLineArgs args(argc, argv);

	// Specify CIM files
#ifdef _WIN32
	String path("Examples\\CIM\\WSCC-09_Neplan_RX\\");
#elif defined(__linux__) || defined(__APPLE__)
	String path("Examples/CIM/WSCC-09_Neplan_RX/");
#endif

	std::list<String> filenames = {
		path + "WSCC-09_Neplan_RX_DI.xml",
		path + "WSCC-09_Neplan_RX_EQ.xml",
		path + "WSCC-09_Neplan_RX_SV.xml",
		path + "WSCC-09_Neplan_RX_TP.xml"
	};

	String simName = "Shmem_WSCC-9bus_Ctrl";

	CIM::Reader reader(simName, Logger::Level::INFO, Logger::Level::INFO);
	SystemTopology sys = reader.loadCIM(60, filenames);

	// Extend system with controllable load (Profile)
	auto load_profile = PQLoadCS::make("load_cs_profile", Node::List{sys.mNodes[6]}, 0, 0, 230000, Logger::Level::INFO);
	sys.mComponents.push_back(load_profile);

	// Extend system with controllable load
	auto load = PQLoadCS::make("load_cs", Node::List{sys.mNodes[3]}, 0, 0, 230000, Logger::Level::INFO);
	sys.mComponents.push_back(load);

	// Controllers and filter
	std::vector<Real> coefficients_profile = std::vector(2000, 1./2000);
	std::vector<Real> coefficients = std::vector(100, 1./100);

	auto filtP_profile = FIRFilter::make("filter_p_profile", coefficients_profile, Logger::Level::INFO);
	filtP_profile->setPriority(1);
	filtP_profile->initialize(0.);
	filtP_profile->setConnection(load_profile->findAttribute<Real>("active_power"));
	filtP_profile->findAttribute<Real>("input")->set(0.);
	sys.mComponents.push_back(filtP_profile);

	auto filtP = FIRFilter::make("filter_p", coefficients, Logger::Level::INFO);
	filtP->setPriority(1);
	filtP->initialize(0.);
	filtP->setConnection(load->findAttribute<Real>("active_power"));
	filtP->findAttribute<Real>("input")->set(0.);
	sys.mComponents.push_back(filtP);

	RealTimeSimulation sim(simName, sys, args.timeStep, args.duration, args.solver.domain, args.solver.type, args.logLevel, true);

	// Create shmem interface
	Interface::Config conf;
	conf.samplelen = 64;
	conf.queuelen = 1024;
	conf.polling = false;
	String in  = "/villas-dpsim1";
	String out = "/dpsim1-villas";
	Interface intf(out, in, &conf);

	// Register exportable node voltages
	for (auto n : sys.mNodes) {
		UInt i;
		if (sscanf(n->name().c_str(), "BUS%u", &i) != 1) {
			std::cerr << "Failed to determine bus no of bus: " << n->name() << std::endl;
			continue;
		}

		i--;

		auto v = n->findAttribute<Complex>("voltage");

		std::cout << "Signal << " << (i*2)+0 << ": Mag " << n->name() << std::endl;
		std::cout << "Signal << " << (i*2)+1 << ": Phas " << n->name() << std::endl;

		std::function<Real()> getMag = [v](){ return std::abs(v->get()); };
		std::function<Real()> getPhas = [v](){ return std::arg(v->get()); };

		intf.addExport(getMag,  (i*2)+0);
		intf.addExport(getPhas, (i*2)+1);
	}

	// Register controllable load
	//intf.addImport(load->findAttribute<Real>("active_power"), 1.0, 0);
	intf.addImport(filtP->findAttribute<Real>("input"), 1.0, 0);
	intf.addImport(filtP_profile->findAttribute<Real>("input"), 20e8, 1);

	sim.addInterface(&intf, false, false);
	sim.run();

	return 0;
}
