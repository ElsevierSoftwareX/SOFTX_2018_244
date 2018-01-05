/** Current source
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

#include "DP_CurrentSource_Ideal.h"

using namespace DPsim;

Component::DP::CurrentSourceIdeal::CurrentSourceIdeal(String name, Int src, Int dest, Complex current)
	: Base(name, src, dest)
{
	mCurrent = current;

	attrMap["current"] = { Attribute::Complex, &mCurrent };
};

void Component::DP::CurrentSourceIdeal::applyRightSideVectorStamp(SystemModel& system)
{
	if (mNode1 >= 0) {
		system.addCompToRightSideVector(mNode1, mCurrent.real(), mCurrent.imag());
	}
	if (mNode2 >= 0) {
		system.addCompToRightSideVector(mNode2, -mCurrent.real(), mCurrent.imag());
	}
}

void Component::DP::CurrentSourceIdeal::step(SystemModel& system, Real time)
{
	applyRightSideVectorStamp(system);
}

Complex Component::DP::CurrentSourceIdeal::getCurrent(SystemModel &system)
{
	return mCurrent;
}

void Component::DP::CurrentSourceIdeal::setCurrent(Complex current)
{
	mCurrent = current;
}

