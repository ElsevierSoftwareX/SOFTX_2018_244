/** SynGenDPBalancedResLoad Example
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
using namespace CPS::DP::Ph3;

int main(int argc, char* argv[]) {
	// Define simulation parameters
	Real timeStep = 0.00005; //initial: 0.00005
	Real finalTime = 0.1;
	String simName = "DP_SynchronGenerator_dq_ThreePhFault";
	// Define machine parameters in per unit
	Real nomPower = 555e6;
	Real nomPhPhVoltRMS = 24e3;
	Real nomFreq = 60;
	Real nomFieldCurr = 1300;
	Int poleNum = 2;
	Real H = 3.7;
	Real Rs = 0.003;
	Real Ll = 0.15;
	Real Lmd = 1.6599;
	Real Lmq = 1.61;
	Real Rfd = 0.0006;
	Real Llfd = 0.1648;
	Real Rkd = 0.0284;
	Real Llkd = 0.1713;
	Real Rkq1 = 0.0062;
	Real Llkq1 = 0.7252;
	Real Rkq2 = 0.0237;
	Real Llkq2 = 0.125;
	// Initialization parameters
	Real initActivePower = 300e6;
	Real initReactivePower = 0;
	Real initTerminalVolt = 24000 / sqrt(3) * sqrt(2);
	Real initVoltAngle = -PI / 2;
	Real fieldVoltage = 7.0821;
	Real mechPower = 300e6;
	// Define grid parameters
	Real Rload = 1.92;
	Real BreakerOpen = 1e6;
	Real BreakerClosed = 1e-6;

	// Nodes
	std::vector<Complex> initVoltN1 = std::vector<Complex>({
		Complex(initTerminalVolt * cos(initVoltAngle), initTerminalVolt * sin(initVoltAngle)),
		Complex(initTerminalVolt * cos(initVoltAngle - 2 * PI / 3), initTerminalVolt * sin(initVoltAngle - 2 * PI / 3)),
		Complex(initTerminalVolt * cos(initVoltAngle + 2 * PI / 3), initTerminalVolt * sin(initVoltAngle + 2 * PI / 3)) });
	auto n1 = Node::make("n1", PhaseType::ABC, initVoltN1);

	// Components
	auto gen = Ph3::SynchronGeneratorDQ::make("DP_SynGen_dq_ThreePhFault_SynGen");
	gen->setFundamentalParametersPU(nomPower, nomPhPhVoltRMS, nomFreq, poleNum, nomFieldCurr,
		Rs, Ll, Lmd, Lmq, Rfd, Llfd, Rkd, Llkd, Rkq1, Llkq1, Rkq2, Llkq2, H,
		initActivePower, initReactivePower, initTerminalVolt, initVoltAngle, fieldVoltage, mechPower); //no real initialization

	auto res = Ph3::SeriesResistor::make("R_load");
	res->setParameters(Rload);

	auto fault = Ph3::SeriesSwitch::make("Br_fault");
	fault->setParameters(BreakerOpen, BreakerClosed);
	fault->open();

	// Connections
	gen->connect({n1});
	res->connect({Node::GND, n1});
	fault->connect({Node::GND, n1});

	// System
	auto sys = SystemTopology(60, SystemNodeList{n1}, SystemComponentList{gen, res, fault});

	//logger:
	auto logger = DataLogger::make(simName);
	/*
	logger->addAttribute("i", gen->attribute("i_intf"));
	//logger->addAttribute("v1", n1->attribute("v"));
	logger->addAttribute("theta", gen->attribute("theta"));*/
	// Simulation
	/*Simulation sim(simName, sys, timeStep, finalTime,
		Domain::DP, Solver::Type::MNA, Logger::Level::INFO); //Domain::DP, Solver::Type::MNA, Logger::Level::INFO*/

	std::shared_ptr<ODESolver>  ode_solver;
	ode_solver= std::make_shared<ODESolver>("DP_SynGen_dq_ThreePhFault_SynGen_SOLVER", gen, timeStep, 0);

	std::vector<std::shared_ptr<ODESolver> > ode_comps;
	ode_comps.push_back(ode_solver); // for each ODE-component

	Sim_ODE sim(simName, sys, timeStep, finalTime, ode_comps, Domain::DP, Solver::Type::MNA, Logger::Level::INFO);
	sim.addLogger(logger);

	// Events
	auto sw1 = SwitchEvent::make(0.05, fault, true);
	sim.addEvent(sw1);

	//gen->print_States();
	sim.run();
	//gen->print_States();
	return 0;
}
