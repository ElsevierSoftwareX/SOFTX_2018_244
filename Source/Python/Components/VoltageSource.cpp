/** Python binding for ExternalVoltageSource
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

#include "Python/Components/VoltageSource.h"

const char *DPsim::Python::Components::DocVoltageSource =
"VoltageSource(name, node1, node2, initial_voltage)\n"
"Construct a new external voltage source.\n"
"\n"
"An external voltage source is pretty much the same as a normal ideal voltage "
"source, but its voltage value can be controlled from external programs by "
"registering it with an `Interface`.\n"
"\n"
":param initial_current: The voltage of this source in the first timestep (as a complex value).\n"
":returns: A new `Component` representing this voltage source.\n";
