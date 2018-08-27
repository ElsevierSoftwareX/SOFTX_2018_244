/** Python binding for Inductors.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
 *
 * CPowerSystems
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

#include "Python/Components/Inductor.h"

const char *DPsim::Python::Components::DocInductor =
"Inductor(name, node1, node2, inductance)\n"
"Construct a new inductor.\n"
"\n"
"Attributes: ``inductance``.\n"
"\n"
":param inductance: Inductance in Henry.\n"
":returns: A new `Component` representing this inductor.\n";

template<>
PyObject* DPsim::Python::Components::Inductor<CPS::EMT::Ph1::Inductor>(PyObject* self, PyObject* args)
{
    const char *name;
    double inductance;

    if (!PyArg_ParseTuple(args, "sd", &name, &inductance))
        return nullptr;

    try {
        Component *pyComp = PyObject_New(Component, &DPsim::Python::ComponentType);
        Component::init(pyComp);

        std::shared_ptr<CPS::EMT::Ph1::Inductor> cps_comp = CPS::EMT::Ph1::Inductor::make(name);
        cps_comp->setParameters(inductance);
        pyComp->comp = cps_comp;

        return (PyObject*) pyComp;
    } catch (...) {
        return nullptr;
    }
}

template<>
PyObject* DPsim::Python::Components::Inductor<CPS::DP::Ph1::Inductor>(PyObject* self, PyObject* args)
{
    const char *name;
    double inductance;

    if (!PyArg_ParseTuple(args, "sd", &name, &inductance))
        return nullptr;

    try {
        Component *pyComp = PyObject_New(Component, &DPsim::Python::ComponentType);
        Component::init(pyComp);

        std::shared_ptr<CPS::DP::Ph1::Inductor> cps_comp = CPS::DP::Ph1::Inductor::make(name);
        cps_comp->setParameters(inductance);
        pyComp->comp = cps_comp;

        return (PyObject*) pyComp;
    } catch (...) {
        return nullptr;
    }
}
