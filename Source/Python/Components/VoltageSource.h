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

#pragma once

#include "Python/Component.h"

namespace DPsim {
namespace Python {
namespace Components {

	extern const char* DocVoltageSource;

	template<class C>
	PyObject* VoltageSource(PyObject* self, PyObject* args)
	{
		const char *name;
		int src, dest;
		Py_complex initVoltage;

		if (!PyArg_ParseTuple(args, "siiD", &name, &src, &dest, &initVoltage))
			return nullptr;

		Component *pyComp = PyObject_New(Component, &DPsim::Python::ComponentType);
		Component::init(pyComp);
		pyComp->comp = std::make_shared<C>(name, src, dest, CPS::Complex(initVoltage.real, initVoltage.imag));

		return (PyObject*) pyComp;
	}
}
}
}
