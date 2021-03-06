/** Python node
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2018, Institute for Automation of Complex Power Systems, EONERC
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

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif

#include <cps/Node.h>

namespace DPsim {
namespace Python {

	template<typename VarType>
	struct Node {
		PyObject_HEAD

		typename CPS::Node<VarType>::Ptr node;

		// The Python API has no notion of C++ classes and methods, so the methods
		// that can be called from Python are static.
		//
		// Helper methods for memory management / initialization etc.
		static PyObject* newfunc(PyTypeObject *type, PyObject *args, PyObject *kwds);
		static int init(Node<VarType> *self, PyObject *args, PyObject *kwds);
		static void dealloc(Node<VarType> *self);

		static PyObject * gnd(PyObject *self, PyObject *args);

		static const char *name;
		static const char *doc;
		static const char *docGND;
		static PyMethodDef methods[];
		static PyTypeObject type;

		static PyObject *Py_GND;

		static typename CPS::Node<VarType>::List fromPython(PyObject* list);
	};

	template<typename VarType>
	PyObject *Node<VarType>::Py_GND = NULL;

	CPS::TopologicalNode::List nodesFromPython(PyObject* list);
}
}
