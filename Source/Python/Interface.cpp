/** Python Interface
 *
 * @file
 * @author Georg Reinke <georg.reinke@rwth-aachen.de>
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

#include <dpsim/Config.h>

#ifdef WITH_SHMEM
  #include <cps/Interface.h>
  #include <cps/AttributeList.h>
#endif

#include <dpsim/Python/Interface.h>
#include <dpsim/Python/Component.h>

using namespace DPsim;

PyObject* Python::Interface::newfunc(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Python::Interface *self;

	self = (Python::Interface*) type->tp_alloc(type, 0);
	if (self) {
#ifdef WITH_SHMEM
		using SharedIntfPtr = std::shared_ptr<CPS::Interface>;

		new (&self->intf) SharedIntfPtr();
#endif
	}

	return (PyObject*) self;
}

void Python::Interface::dealloc(Python::Interface* self)
{
#ifdef WITH_SHMEM
		using SharedIntfPtr = std::shared_ptr<CPS::Interface>;

		self->intf.~SharedIntfPtr();
#endif

	Py_TYPE(self)->tp_free((PyObject*) self);
}

const char* Python::Interface::docRegisterControlledAttribute =
"import(comp, attr, real_idx, imag_idx)\n"
"Register a source with this interface, causing it to use values received from "
"this interface as its current or voltage value.\n"
"\n"
":param comp: The ``Component`` whose attribute we want to modify.\n"
":param attr:\n"
":param real_idx: Index of the real part of the current or voltage.\n"
":param imag_idx: Index of the imaginary part of the current or voltage.\n";
PyObject* Python::Interface::registerControlledAttribute(Interface* self, PyObject* args, PyObject *kwargs)
{
#ifdef WITH_SHMEM
	PyObject *pyObj;
	int idx;
	const char *attrName;
	double gain;
	int mode;

	const char *kwlist[] = {"component", "attribute", "idx", "mode", "gain", nullptr};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Osi|ds", (char **) kwlist, &pyObj, &attrName, &idx, &gain, &mode))
		return nullptr;

	CPS::AttributeList::Ptr attrList;
	if (PyObject_TypeCheck(pyObj, &Python::Component::type)) {
		auto *pyComp = (Component*) pyObj;

		attrList = std::dynamic_pointer_cast<CPS::AttributeList>(pyComp->comp);
	}
	else if (PyObject_TypeCheck(pyObj, &Python::Node<CPS::Real>::type)) {
		auto *pyNode = (Node<CPS::Real> *) pyObj;

		attrList = std::dynamic_pointer_cast<CPS::AttributeList>(pyNode->node);
	}
	else if (PyObject_TypeCheck(pyObj, &Python::Node<CPS::Complex>::type)) {
		auto *pyNode = (Node<CPS::Complex> *) pyObj;

		attrList = std::dynamic_pointer_cast<CPS::AttributeList>(pyNode->node);
	}
	else {
		PyErr_SetString(PyExc_TypeError, "First argument must be a Component or a Node");
		return nullptr;
	}

	try {
		switch (mode) {
			case 0: { // Real Attribute

				CPS::Attribute<CPS::Real>::Ptr a = attrList->findAttribute<CPS::Real>(attrName);

				self->intf->addImport(a, gain, idx);
				break;
			}

			case 1: { // Complex Attribute: real & imag
				auto a = attrList->findAttribute<CPS::Complex>(attrName);

				self->intf->addImport(a, gain, idx, idx + 1);
				break;
			}

			case 2: { // Complex Attribute: mag & phase
				auto a = attrList->findAttribute<CPS::Complex>(attrName);

				std::function<void(CPS::Real)> setMag = [a](CPS::Real r) {
					CPS::Complex z = a->get();
					a->set(CPS::Complex(r, z.imag()));
				};

				std::function<void(CPS::Real)> setPhas = [a](CPS::Real i) {
					CPS::Complex z = a->get();
					a->set(CPS::Complex(z.real(), i));
				};

				self->intf->addImport(setMag,  idx);
				self->intf->addImport(setPhas, idx+1);
				break;
			}

			default:
				PyErr_SetString(PyExc_TypeError, "Invalid mode");
				return nullptr;
		}
	}
	catch (const CPS::InvalidAttributeException &exp) {
		PyErr_SetString(PyExc_TypeError, "First argument must be a writable attribute");
		return nullptr;
	}

	Py_INCREF(Py_None);
	return Py_None;
#else
	PyErr_SetString(PyExc_NotImplementedError, "not implemented on this platform");
	return nullptr;
#endif
}

const char* Python::Interface::docRegisterExportedAttribute =
"export(comp, attr, real_idx, imag_idx)\n"
"Register a voltage between two nodes to be written to this interface after every timestep.\n"
"\n"
":param comp:\n"
":param attr:\n"
":param real_idx: Index where the real part of the voltage is written.\n"
":param imag_idx: Index where the imaginary part of the voltage is written.\n";
PyObject* Python::Interface::registerExportedAttribute(Interface* self, PyObject* args, PyObject* kwargs)
{
#ifdef WITH_SHMEM
	PyObject* pyObj;
	int idx;
	const char *attrName;
	double gain = 1;
	int mode = 0;

	const char *kwlist[] = {"component", "attribute", "idx", "mode", "gain", nullptr};

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Osi|id", (char **) kwlist, &pyObj, &attrName, &idx, &mode, &gain))
		return nullptr;

	CPS::AttributeList::Ptr attrList;
	if (PyObject_TypeCheck(pyObj, &Python::Component::type)) {
		auto *pyComp = (Component*) pyObj;

		attrList = std::dynamic_pointer_cast<CPS::AttributeList>(pyComp->comp);
	}
	else if (PyObject_TypeCheck(pyObj, &Python::Node<CPS::Real>::type)) {
		auto *pyNode = (Node<CPS::Real> *) pyObj;

		attrList = std::dynamic_pointer_cast<CPS::AttributeList>(pyNode->node);
	}
	else if (PyObject_TypeCheck(pyObj, &Python::Node<CPS::Complex>::type)) {
		auto *pyNode = (Node<CPS::Complex> *) pyObj;

		attrList = std::dynamic_pointer_cast<CPS::AttributeList>(pyNode->node);
	}
	else {
		PyErr_SetString(PyExc_TypeError, "First argument must be a Component or a Node");
		return nullptr;
	}

	try {
		switch (mode) {
			case 0: { // Real Attribute

				CPS::Attribute<CPS::Real>::Ptr realAttr = attrList->findAttribute<CPS::Real>(attrName);

				self->intf->addExport(realAttr, gain, idx);
				break;
			}

			case 1: { // Complex Attribute: real & imag
				auto a = attrList->findAttribute<CPS::Complex>(attrName);

				self->intf->addExport(a, gain, idx, idx + 1);
				break;
			}

			case 2: { // Complex Attribute: mag & phase
				auto a = attrList->findAttribute<CPS::Complex>(attrName);

				std::function<CPS::Real()> getMag = [a](){ return std::abs(a->get()); };
				std::function<CPS::Real()> getPhas = [a](){ return std::arg(a->get()); };

				self->intf->addExport(getMag,  idx);
				self->intf->addExport(getPhas, idx+1);
				break;
			}

			default:
				PyErr_SetString(PyExc_TypeError, "Invalid mode");
				return nullptr;
		}
	}
	catch (const CPS::InvalidAttributeException &exp) {
		PyErr_SetString(PyExc_TypeError, "First argument must be a writable attribute");
		return nullptr;
	}

	Py_INCREF(Py_None);
	return Py_None;
#else
	PyErr_SetString(PyExc_NotImplementedError, "not implemented on this platform");
	return nullptr;
#endif
}

const char* Python::Interface::docOpen =
"open_interface(wname, rname, queuelen=512, samplelen=64, polling=False)\n"
"Opens a set of shared memory regions to use as an interface for communication. The communication type / format of VILLASNode's shmem node-type is used; see its documentation for more information on the internals.\n"
"\n"
"For this interface type, the indices passed to the methods specify the indices "
"in the array of values that is passed in each timestep. The sending program "
"must be configured to use the same indices as well.\n"
"\n"
":param wname: Name of the POSIX shared memory object where values are written. "
"Must start with a ``/``.\n"
":param rname: Name of the POSIX shared memory object where values are read from. "
"Must start with a ``/``.\n"
":param queuelen: Length of the outgoing queue as an integer number of samples.\n"
":param samplelen: Number of values that are passed as a \"sample\" each timestep. "
"Must be greater or equal to the maximum index passed to the export methods.\n"
":param polling: If True, no POSIX CV will be used to signal writes to the "
"interface, meaning that polling will have to be used. This may increase "
"performance at the cost of wasted CPU time.\n"
":returns: A new `Interface` object.\n";
PyObject* Python::OpenInterface(PyObject *self, PyObject *args, PyObject *kwds)
{
#ifdef WITH_SHMEM
	static const char *kwlist[] = {"wname", "rname", "queuelen", "samplelen", "polling", nullptr};
	CPS::Interface::Config conf;
	const char *wname, *rname;

	conf.queuelen = 512;
	conf.samplelen = 64;
	conf.polling = 0;
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss|iib", (char **) kwlist,
		&wname, &rname, &conf.queuelen, &conf.samplelen, &conf.polling)) {
		return nullptr;
	}

	Python::Interface *pyIntf = PyObject_New(Python::Interface, &Python::Interface::type);
	pyIntf->intf = CPS::Interface::make(wname, rname, &conf);
	return (PyObject*) pyIntf;
#else
	PyErr_SetString(PyExc_NotImplementedError, "not implemented on this platform");
	return nullptr;
#endif
}

PyMethodDef Python::Interface::methods[] = {
	{"export_attribute", (PyCFunction) Python::Interface::registerExportedAttribute, METH_VARARGS | METH_KEYWORDS, Python::Interface::docRegisterExportedAttribute},
	{"import_attribute", (PyCFunction) Python::Interface::registerControlledAttribute, METH_VARARGS, Python::Interface::docRegisterControlledAttribute},
	{0},
};

const char* Python::Interface::doc =
"A connection to an external program, using which simulation data is exchanged.\n"
"\n"
"Currently, only an interface using POSIX shared memory is implemented (see `open_interface`).\n";
PyTypeObject Python::Interface::type = {
	PyVarObject_HEAD_INIT(nullptr, 0)
	"dpsim.Interface",                       /* tp_name */
	sizeof(Python::Interface),               /* tp_basicsize */
	0,                                       /* tp_itemsize */
	(destructor)Python::Interface::dealloc,  /* tp_dealloc */
	0,                                       /* tp_print */
	0,                                       /* tp_getattr */
	0,                                       /* tp_setattr */
	0,                                       /* tp_reserved */
	0,                                       /* tp_repr */
	0,                                       /* tp_as_number */
	0,                                       /* tp_as_sequence */
	0,                                       /* tp_as_mapping */
	0,                                       /* tp_hash  */
	0,                                       /* tp_call */
	0,                                       /* tp_str */
	0,                                       /* tp_getattro */
	0,                                       /* tp_setattro */
	0,                                       /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,/* tp_flags */
	Python::Interface::doc,                  /* tp_doc */
	0,                                       /* tp_traverse */
	0,                                       /* tp_clear */
	0,                                       /* tp_richcompare */
	0,                                       /* tp_weaklistoffset */
	0,                                       /* tp_iter */
	0,                                       /* tp_iternext */
	Python::Interface::methods,              /* tp_methods */
	0,                                       /* tp_members */
	0,                                       /* tp_getset */
	0,                                       /* tp_base */
	0,                                       /* tp_dict */
	0,                                       /* tp_descr_get */
	0,                                       /* tp_descr_set */
	0,                                       /* tp_dictoffset */
	0,                                       /* tp_init */
	0,                                       /* tp_alloc */
	Python::Interface::newfunc               /* tp_new */
};
