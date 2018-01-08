/** Turbine Governor
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

#include "Base.h"

namespace DPsim {
namespace Components {

	/// Synchronous generator model
	/// If parInPerUnit is not set, the parameters have to be given with their respective stator or rotor
	/// referred values. The calculation to per unit is performed in the initialization.
	/// The case where parInPerUnit is not set will be implemented later.
	/// parameter names include underscores and typical variables names found in literature instead of
	/// descriptive names in order to shorten formulas and increase the readability

	class TurbineGovernor {

	protected:
		// ### Steam Turbine Parameters ####

		/// Time constant of main inlet volume and steam chest
		Real mTa;
		/// Time constant of reheater
		Real mTb;
		/// Time constant of cross over piping and LP inlet volumes
		Real mTc;
		/// Fraction of total turbine power generated by HP section
		Real mFa;
		/// Fraction of total turbine power generated by IP section
		Real mFb;
		/// Fraction of total turbine power generated by LP section
		Real mFc;

		// ### Regulator ####
		/// Main droop
		Real mK;

		// ### Speed Relay and servo motor ####
		/// Time constant of speed relay
		Real mTsr;
		/// Time constant of servo motor
		Real mTsm;
		/// Opening rate limit
		Real mLc1 = 0.1;
		/// Closing rate limit
		Real mLc2 = -1;

		// ### Vaariables ###
		/// Mechanical Torque in pu (output of steam turbine)
		Real mTm = 0;
		/// Valve position
		Real mVcv = 0;
		/// Valve position changing rate
		Real mpVcv = 0;
		/// Boiler pressure
		Real mPb = 1;
		/// Speed reference
		Real mOmRef = 1;
		/// Speed
		Real mOm = 0;
		/// Power Reference
		Real mPmRef;
		/// Input of speed realy
		Real Psr_in = 0;
		/// Input of servor motor
		Real Psm_in = 0;

		Real AuxVar = 0;

		bool mLogActive;
		Logger* mLog;

	public:
		TurbineGovernor() { };
		~TurbineGovernor() { };

		/// Initializes exciter parameters
		TurbineGovernor(Real Ta, Real Tb, Real Tc, Real Fa, Real Fb, Real Fc, Real K, Real Tsr, Real Tsm);

		void init(Real PmRef, Real Tm_init);
		/// Performs an step to update field voltage value
		Real step(Real mOm, Real mOmRef, Real PmRef, Real dt);
	};
}
}
