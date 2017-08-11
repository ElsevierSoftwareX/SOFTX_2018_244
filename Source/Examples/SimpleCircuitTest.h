#ifndef SIMPLECIRCUIT_H
#define SIMPLECIRCUIT_H

#include "../MathLibrary.h"

namespace DPsim {
	void RXLineResLoad();
	void VarFreqRXLineResLoad(Real timeStep, Real finalTime, Real freqStep, Real loadStep, Real rampTime);
	void RXLineResLoadEMT();
	void VarFreqRXLineResLoadEMT(Real timeStep, Real finalTime, Real freqStep, Real loadStep, Real rampTime);
	void runDpEmtVarFreqStudy();
	void RXLineResLoadStatic();
	void RTExample();
};

#endif
