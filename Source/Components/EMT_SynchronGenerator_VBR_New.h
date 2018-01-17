﻿/** Voltage behind reactance (EMT)
*
* @file
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

#include "Base_SynchronGenerator.h"
#include "Exciter.h"
#include "TurbineGovernor.h"
#include "EMT_VoltageSource.h"
#include "EMT_Resistor.h"


namespace DPsim {
		namespace Components {
				namespace EMT {

						/// Synchronous generator model
						/// If parInPerUnit is not set, the parameters have to be given with their respective stator or rotor
						/// referred values. The calculation to per unit is performed in the initialization.
						/// The case where parInPerUnit is not set will be implemented later.
						/// parameter names include underscores and typical variables names found in literature instead of
						/// descriptive names in order to shorten formulas and increase the readability

						class VoltageBehindReactanceEMTNew : public SynchronGeneratorBase, public SharedFactory<VoltageBehindReactanceEMTNew> {
						protected:

								/// Exciter Model
								Exciter mExciter;
								/// Determine if Exciter is activated
								bool WithExciter = false;

								/// Governor Model
								TurbineGovernor mTurbineGovernor;
								/// Determine if Turbine and Governor are activated
								bool WithTurbineGovernor = false;
							
								/// d dynamic inductance
								Real mDLmd;
								/// q dynamic inductance
								Real mDLmq;

								/// Auxiliar inductance
								Real mLa;
								/// Auxiliar inductance
								Real mLb;

								/// Dynamic d voltage
								Real mDVq;
								/// Dynamic q voltage
								Real mDVd;
								/// Dynamic voltage phase a
								Real mDVa;
								/// Dynamic voltage phase b
								Real mDVb;
								/// Dynamic voltage phase c
								Real mDVc;

								/// Interface voltage phase a
								Real mVa;
								/// Interface voltage phase b
								Real mVb;
								/// Interface voltage phase c
								Real mVc;

								/// Interface curent phase a
								Real mIa;
								/// Interface curent phase b
								Real mIb;
								/// Interface curent phase c
								Real mIc;

								/// Phase currents in pu
								Matrix mIabc = Matrix::Zero(3, 1);
								///Phase Voltages in pu
								Matrix mVabc = Matrix::Zero(3, 1);
								/// Subtransient voltage in pu
								Matrix mDVabc = Matrix::Zero(3, 1);

								/// Magnetizing flux linkage in q axis
								Real mPsimq;
								/// Magnetizing flux linkage in d axis
								Real mPsimd;

								/// Dq stator current vector
								Matrix mDqStatorCurrents = Matrix::Zero(2, 1);
								/// Q axis stator current of  from last time step
								Real mIq_hist;
								/// D axis stator current of  from last time step
								Real mId_hist;

								// ### Useful Matrices ###
								/// inductance matrix
								Matrix mDInductanceMat = Matrix::Zero(3, 3);
	
								/// Q axis Rotor flux 
								Matrix mPsikq1kq2 = Matrix::Zero(2, 1);
								/// D axis rotor flux
								Matrix mPsifdkd = Matrix::Zero(2, 1);
								/// Equivalent Stator Conductance Matrix
								Matrix mConductanceMat = Matrix::Zero(3, 3);
								/// Equivalent Stator Current Source
								Matrix mISourceEq = Matrix::Zero(3, 1);
								/// Dynamic Voltage Vector
								Matrix mDVqd = Matrix::Zero(2, 1);
								/// Equivalent VBR Stator Resistance
								Matrix R_eq_vbr = Matrix::Zero(3, 3);
								/// Equivalent VBR Stator Voltage Source
								Matrix E_eq_vbr = Matrix::Zero(3, 1);
								/// Park Transformation Matrix
								Matrix mKrs_teta = Matrix::Zero(3, 3);
								/// Inverse Park Transformation Matrix
								Matrix mKrs_teta_inv = Matrix::Zero(3, 3);


								/// Auxiliar variables
								Real c21_omega;
								Real c22_omega;
								Real c13_omega;
								Real c14_omega;
								Matrix K1a = Matrix::Zero(2, 2);
								Matrix K1b = Matrix::Zero(2, 1);
								Matrix K1 = Matrix::Zero(2, 1);
								Matrix K2a = Matrix::Zero(2, 2);
								Matrix K2b = Matrix::Zero(2, 1);
								Matrix K2 = Matrix::Zero(2, 1);
								Matrix H_qdr = Matrix::Zero(3, 1);
								Matrix h_qdr;
								Matrix K = Matrix::Zero(3, 3);
								Matrix mEsh_vbr = Matrix::Zero(3, 1);
								Matrix E_r_vbr = Matrix::Zero(3, 1);
								Matrix K1K2 = Matrix::Zero(2, 2);

								/// Auxiliar constants
								Real c11;
								Real c12;
								Real c23;
								Real c24;
								Real c15;
								Real c25;
								Real c26;

								Real b11;
								Real b12;
								Real b13;
								Real b21;
								Real b22;
								Real b23;
								Real b31;
								Real b32;
								Real b33;
								Real b41;
								Real b42;
								Real b43;

								Real E1_1d;
								Real E2_1d;
		
								Matrix Ea = Matrix::Zero(2, 2);
								Matrix E1b = Matrix::Zero(2, 1);
								Matrix E1 = Matrix::Zero(2, 2);
								Matrix Fa = Matrix::Zero(2, 2);
								Matrix F1b = Matrix::Zero(2, 1);
								Matrix F1 = Matrix::Zero(2, 2);
								Matrix E2b = Matrix::Zero(2, 2);
								Matrix E2 = Matrix::Zero(2, 2);
								Matrix F2b = Matrix::Zero(2, 2);
								Matrix F2 = Matrix::Zero(2, 2);
								Matrix F3b = Matrix::Zero(2, 1);
								Matrix F3 = Matrix::Zero(2, 2);
								Matrix C26 = Matrix::Zero(2, 1);


						public:
								//VoltageBehindReactanceEMTNew() { };
								~VoltageBehindReactanceEMTNew();

								/// Initializes the per unit or stator referred machine parameters with the machine parameters given in per unit or
								/// stator referred parameters depending on the setting of parameter type.
								/// The initialization mode depends on the setting of state type.
								VoltageBehindReactanceEMTNew(String name, Int node1, Int node2, Int node3,
										Real nomPower, Real nomVolt, Real nomFreq, Int poleNumber, Real nomFieldCur,
										Real Rs, Real Ll, Real Lmd, Real Lmd0, Real Lmq, Real Lmq0,
										Real Rfd, Real Llfd, Real Rkd, Real Llkd,
										Real Rkq1, Real Llkq1, Real Rkq2, Real Llkq2,
										Real inertia, Logger::Level logLevel = Logger::Level::NONE);

								/// Function to initialize Exciter
								void AddExciter(Real Ta, Real Ka, Real Te, Real Ke, Real Tf, Real Kf, Real Tr, Real Lad, Real Rfd);
								/// Function to initialize Governor and Turbine
								void AddGovernor(Real Ta, Real Tb, Real Tc, Real Fa, Real Fb, Real Fc, Real K, Real Tsr, Real Tsm, Real Tm_init, Real PmRef);

								/// Initializes states in per unit or stator referred variables depending on the setting of the state type.
								/// Function parameters have to be given in real units.
								void initialize(Real om, Real dt,
										Real initActivePower, Real initReactivePower, Real initTerminalVolt, Real initVoltAngle, Real initFieldVoltage, Real initMechPower);

								/// Performs an Euler forward step with the state space model of a synchronous generator
								/// to calculate the flux and current from the voltage vector.
								void step(SystemModel& system, Real time);

								/// Performs an Euler forward step with the state space model of a synchronous generator
								/// to calculate the flux and current from the voltage vector in per unit.
								void stepInPerUnit(Real om, Real dt, Real time, NumericalMethod numMethod);

								/// Retrieves calculated voltage from simulation for next step
								void postStep(SystemModel& system);

								/// Park transform as described in Krause
								Matrix parkTransform(Real theta, Real a, Real b, Real c);

								/// Inverse Park transform as described in Krause
								Matrix inverseParkTransform(Real theta, Real q, Real d, Real zero);

								/// Calculate inductance Matrix L and its derivative
								void CalculateL();
								void CalculateAuxiliarConstants(Real dt);
								void CalculateAuxiliarVariables();

								//Matrix& getRotorFluxes() { return mRotorFlux; }
								Matrix& getDqStatorCurrents() { return mDqStatorCurrents; }
								Real getElectricalTorque() { return mElecTorque*mBase_T; }
								Real getRotationalSpeed() { return mOmMech*mBase_OmMech; }
								Real getRotorPosition() { return mThetaMech; }
								Matrix& getStatorCurrents() { return mIabc; }

								void init(Real om, Real dt) { }
								void applySystemMatrixStamp(SystemModel& system) { }
								void applyRightSideVectorStamp(SystemModel& system) { }

						};
}
}
}

