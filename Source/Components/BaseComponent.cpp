/** Base component
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
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

#include <cxxabi.h>
#include <typeinfo>

#include "BaseComponent.h"

using namespace DPsim;

std::string BaseComponent::getType()
{
	int status = 1;
	const char *unmangled, *mangled;

	mangled = typeid(*this).name();
	unmangled = abi::__cxa_demangle(mangled, NULL, NULL, &status);

	if (status)
		return mangled;
	else {
		std::string ret(unmangled);

		free((void *) unmangled);

		if (ret.find("DPsim::") == 0)
			return ret.substr(7);
		else
			return ret;
	}
}
