#include "VoltSourceResFreq.h"

using namespace DPsim;

VoltSourceResFreq::VoltSourceResFreq(std::string name, int src, int dest, Real voltage, Real phase, Real resistance, Real omegaSource, Real switchTime) : BaseComponent(src, dest) {
	mName = name;
	mResistance = resistance;
	mConductance = 1. / resistance;
	mVoltageAmp = voltage;
	mVoltagePhase = phase;
	mSwitchTime = switchTime;
	mOmegaSource = omegaSource;
	mVoltageDiffr = mVoltageAmp*cos(mVoltagePhase);
	mVoltageDiffi = mVoltageAmp*sin(mVoltagePhase);
	mCurrentr = mVoltageDiffr / mResistance;
	mCurrenti = mVoltageDiffi / mResistance;
}

void VoltSourceResFreq::applySystemMatrixStamp(DPSMatrix& g, int compOffset, Real om, Real dt) {
	// Apply matrix stamp for equivalent resistance
	if (mNode1 >= 0) {
		g(mNode1, mNode1) = g(mNode1, mNode1) + mConductance;
		g(compOffset + mNode1, compOffset + mNode1) = g(compOffset + mNode1, compOffset + mNode1) + mConductance;
	}

	if (mNode2 >= 0) {
		g(mNode2, mNode2) = g(mNode2, mNode2) + mConductance;
		g(compOffset + mNode2, compOffset + mNode2) = g(compOffset + mNode2, compOffset + mNode2) + mConductance;
	}

	if (mNode1 >= 0 && mNode2 >= 0) {
		g(mNode1, mNode2) = g(mNode1, mNode2) - mConductance;
		g(compOffset + mNode1, compOffset + mNode2) = g(compOffset + mNode1, compOffset + mNode2) - mConductance;

		g(mNode2, mNode1) = g(mNode2, mNode1) - mConductance;
		g(compOffset + mNode2, compOffset + mNode1) = g(compOffset + mNode2, compOffset + mNode1) - mConductance;
	}
}

void VoltSourceResFreq::applyRightSideVectorStamp(DPSMatrix& j, int compOffset, Real om, Real dt) {
	// Apply matrix stamp for equivalent current source
	if (mNode1 >= 0) {
		j(mNode1, 0) = j(mNode1, 0) + mCurrentr;
		j(mNode1 + compOffset, 0) = j(compOffset + mNode1, 0) + mCurrenti;
	}

	if (mNode2 >= 0) {
		j(mNode2, 0) = j(mNode2, 0) - mCurrentr;
		j(mNode2 + compOffset, 0) = j(compOffset + mNode2, 0) - mCurrenti;
	}
}


void VoltSourceResFreq::step(DPSMatrix& g, DPSMatrix& j, int compOffset, Real om, Real dt, Real t) {
	if (t >= mSwitchTime) {
		Real fadeInOut = sin(2*PI*10*(t - mSwitchTime)) * mOmegaSource * (t - mSwitchTime);
		mVoltageDiffr = mVoltageAmp*cos(mVoltagePhase + fadeInOut);
		mVoltageDiffi = mVoltageAmp*sin(mVoltagePhase + fadeInOut);
		mCurrentr = mVoltageDiffr / mResistance;
		mCurrenti = mVoltageDiffi / mResistance;
	}

	if (mNode1 >= 0) {
		j(mNode1, 0) = j(mNode1, 0) + mCurrentr;
		j(mNode1 + compOffset, 0) = j(mNode1 + compOffset, 0) + mCurrenti;
	}

	if (mNode2 >= 0) {
		j(mNode2, 0) = j(mNode2, 0) - mCurrentr;
		j(mNode2 + compOffset, 0) = j(mNode2 + compOffset, 0) - mCurrenti;
	}
}
