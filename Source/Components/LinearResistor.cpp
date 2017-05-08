#include "LinearResistor.h"

using namespace DPsim;

LinearResistor::LinearResistor(std::string name, int src, int dest, Real resistance) : BaseComponent(name, src, dest) {
	this->mResistance = resistance;
	this->mConductance = 1.0 / resistance;
}	
		
void LinearResistor::applySystemMatrixStamp(SystemModel& system) {

	// Set diagonal entries
	if (mNode1 >= 0) {
		system.addCompToSystemMatrix(mNode1, mNode1, mConductance, 0);
	}	
	if (mNode2 >= 0) {
		system.addCompToSystemMatrix(mNode2, mNode2, mConductance, 0);
	}
	// Set off diagonal entries
	if (mNode1 >= 0 && mNode2 >= 0) {
		system.addCompToSystemMatrix(mNode1, mNode2, -mConductance, 0);		
		system.addCompToSystemMatrix(mNode2, mNode1, -mConductance, 0);
	}
}

Complex LinearResistor::getCurrent(SystemModel& model) {
	Real realVolt = 0, imagVolt = 0;
	if (mNode1 >= 0) {
		realVolt += model.getRealFromLeftSideVector(mNode1);
		imagVolt += model.getImagFromLeftSideVector(mNode2);
	}
	if (mNode2 >= 0) {
		realVolt -= model.getRealFromLeftSideVector(mNode1);
		imagVolt -= model.getImagFromLeftSideVector(mNode2);
	}
	Complex curr(realVolt*mConductance, imagVolt*mConductance);
	return curr;
}
