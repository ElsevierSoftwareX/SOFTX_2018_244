/** Linear Resistor (EMT)
 *
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

#include "LinearResistorEMT.h"

using namespace DPsim;

LinearResistorEMT::LinearResistorEMT(String name, Int src, Int dest, Real resistance) : BaseComponent(name, src, dest) {
	this->mResistance = resistance;
	attrMap["resistance"] = {AttrReal, &this->mResistance};
}

void LinearResistorEMT::applySystemMatrixStamp(SystemModel& system) {
	this->mConductance = 1.0 / mResistance;
	// Set diagonal entries
	if (mNode1 >= 0) {
		system.addRealToSystemMatrix(mNode1, mNode1, mConductance);
	}
	if (mNode2 >= 0) {
		system.addRealToSystemMatrix(mNode2, mNode2, mConductance);
	}
	// Set off diagonal entries
	if (mNode1 >= 0 && mNode2 >= 0) {
		system.addRealToSystemMatrix(mNode1, mNode2, -mConductance);
		system.addRealToSystemMatrix(mNode2, mNode1, -mConductance);
	}
}


