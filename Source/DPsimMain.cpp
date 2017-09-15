#include <Python.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "CIMReader.h"
#include "PyComponent.h"
#include "PySimulation.h"
#include "Simulation.h"
#include "ShmemInterface.h"

using namespace DPsim;

void usage() {
	std::cerr << "usage: DPsolver [OPTIONS] [PYTHON_FILE...]" << std::endl
	  << "Possible options:" << std::endl
	  << "  -b/--batch:               don't show an interactive prompt after reading files" << std::endl
	  << "  -h/--help:                show this help and exit" << std::endl
	  << "  -i/--interface OBJ_NAME:  prefix for the names of the shmem objects used for communication (default: /dpsim)" << std::endl
	  << "  -n/--node NODE_ID:        RDF id of the node where the interfacing voltage/current source should be placed" << std::endl
	  << "  -r/--realtime:            enable realtime simulation " << std::endl
	  << "  -s/--split INDEX:         index of this instance for distributed simulation (0 or 1)" << std::endl
	  << "Remaining arguments are treated as Python files and executed in order." << std::endl;
}

bool parseFloat(const char *s, double *d) {
	char *end;
	*d = std::strtod(s, &end);
	return (end != s && !*end);
}

bool parseInt(const char *s, int *i) {
	char *end;
	*i = strtol(s, &end, 0);
	return (end != s && !*end);
}

static PyMethodDef pyModuleMethods[] = {
	{"load_cim", pyLoadCim, METH_VARARGS, "Load a network from CIM file(s)."},
	{0}
};

static PyModuleDef dpsimModule = {
	PyModuleDef_HEAD_INIT, "dpsim", NULL, -1, pyModuleMethods,
	NULL, NULL, NULL, NULL
};

static PyObject* PyInit_dpsim(void) {
	PyObject* m;

	if (PyType_Ready(&PyComponentType) < 0)
		return nullptr;
	if (PyType_Ready(&PySimulationType) < 0)
		return nullptr;

	m = PyModule_Create(&dpsimModule);
	if (!m)
		return nullptr;

	Py_INCREF(&PySimulationType);
	PyModule_AddObject(m, "Simulation", (PyObject*) &PySimulationType);
	Py_INCREF(&PyComponentType);
	PyModule_AddObject(m, "Component", (PyObject*) &PyComponentType);
	return m;
}

// TODO: that many platform-dependent ifdefs inside main are kind of ugly
int cimMain(int argc, const char* argv[]) {
	bool rt = false, batch = false;
	int i, split = -1;
	std::string interfaceBase = "/dpsim";
	std::string splitNode = "";
	std::string outName, inName, logName("log.txt"), llogName("lvector.csv"), rlogName("rvector.csv");
#ifdef __linux__
	ShmemInterface *intf = nullptr;
#endif

	// Parse arguments
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--batch")) {
			batch = true;
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			usage();
			return 0;
		} else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--interface")) {
			if (i == argc-1) {
				std::cerr << "Missing argument for -i/--interface; see 'DPsim --help' for usage" << std::endl;
				return 1;
			}
			if (argv[++i][0] != '/') {
				std::cerr << "Shmem interface object name must start with a '/'" << std::endl;
				return 1;
			}
			interfaceBase = std::string(argv[i]);
		} else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--node")) {
			if (i == argc-1) {
				std::cerr << "Missing argument for -n/--node; see 'DPsim --help' for usage" << std::endl;
				return 1;
			}
			splitNode = std::string(argv[++i]);
		} else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--realtime")) {
			rt = true;
		} else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--split")) {
			if (i == argc-1) {
				std::cerr << "Missing argument for -s/--split; see 'DPsim --help' for usage" << std::endl;
				return 1;
			}
			if (!parseInt(argv[++i], &split) || split < 0 || split > 1) {
				std::cerr << "Invalid setting " << argv[i] << " for the split index" << std::endl;
				return 1;
			}
		} else if (argv[i][0] == '-') {
			std::cerr << "Unknown option " << argv[i] << " ; see 'DPsim --help' for usage" << std::endl;
			return 1;
		} else {
			// remaining arguments treated as input files
			break;
		}
	}
#ifndef __linux__
	if (split >= 0 || splitNode.length() != 0) {
		std::cerr << "Distributed simulation not supported on this platform" << std::endl;
		return 1;
	} else if (rt) {
		std::cerr << "Realtime simulation not supported on this platform" << std::endl;
		return 1;
	}
#endif
	if (split >= 0 || splitNode.length() != 0 || rt) {
		std::cerr << "Realtime and distributed simulation currently not supported in combination with Python" << std::endl;
		return 1;
	}

	// TODO: RT / shmem interface with python
	/*
#ifdef __linux__
	// TODO: this is a simple, pretty much fixed setup. Make this more flexible / configurable
	if (split >= 0) {
		int node = reader.mapTopologicalNode(splitNode);
		if (node < 0) {
			std::cerr << "Invalid / missing split node" << std::endl;
			return 1;
		}
		if (split == 0) {
			outName = interfaceBase + ".0.out";
			inName = interfaceBase + ".0.in";
			intf = new ShmemInterface(outName.c_str(), inName.c_str());
			ExternalVoltageSource *evs = new ExternalVoltageSource("v_int", node, 0, 0, 0, reader.getNumVoltageSources()+1);
			intf->registerVoltageSource(evs, 0, 1);
			intf->registerExportedCurrent(evs, 0, 1);
			components.push_back(evs);
			// TODO make log names configurable
			logName = "cim0.log";
			llogName = "lvector-cim0.csv";
			rlogName = "rvector-cim0.csv";
		} else {
			outName = interfaceBase + ".1.out";
			inName = interfaceBase + ".1.in";
			intf = new ShmemInterface(outName.c_str(), inName.c_str());
			ExternalCurrentSource *ecs = new ExternalCurrentSource("i_int", node, 0, 0, 0);
			intf->registerCurrentSource(ecs, 0, 1);
			intf->registerExportedVoltage(node, 0, 0, 1);
			components.push_back(ecs);
			logName = "cim1.log";
			llogName = "lvector-cim1.csv";
			rlogName = "rvector-cim1.csv";
		}
	}
#endif
	*/
	
	PyImport_AppendInittab("dpsim", &PyInit_dpsim);
	Py_Initialize();
	for (; i < argc; i++) {
		FILE* f = fopen(argv[i], "r");
		if (!f) {
			std::cerr << "Failed to open ";
			std::perror(argv[i]);
			return 1;
		}
		PyRun_SimpleFile(f, argv[i]);
		fclose(f);
	}

	if (!batch) {
		while (std::cin.good() && Py_IsInitialized()) {
			std::cout << "> ";
			std::string line;
			std::getline(std::cin, line);
			PyRun_SimpleString(line.c_str());
		}
	}

	/*
#ifdef __linux__
		if (intf)
			sim.addExternalInterface(intf);

		if (rt) {
			sim.runRT(RTTimerFD, true, log, llog, rlog);
		} else {
			while (sim.step(log, llog, rlog))
				sim.increaseByTimeStep();
		}

		if (intf)
			delete intf;
#endif
	*/
	return 0;
}

int main(int argc, const char* argv[]) {
	return cimMain(argc, argv);
}
