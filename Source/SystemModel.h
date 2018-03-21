/** A simulation model
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

#include <iostream>
#include <vector>

#include "cps/Source/Definitions.h"
#include "cps/Source/MathUtils.h"
#include "cps/Source/Logger.h"

namespace DPsim {

	enum class SimulationType { DP, EMT };
	enum class NumericalMethod { Euler, Trapezoidal_flux, Trapezoidal_current };

	class SystemModel {

	private:
		/// Simulation mType
		SimulationType mSimType;
		///Numerical method
		NumericalMethod mNumMethod;
		/// Number of nodes
		Int mNumNodes;
		/// Index offset for imaginary part
		Int mCompOffset;
		/// Angular frequency of the phasor
		Real mSystemOmega;
		/// Simulation time step
		Real mTimeStep;
		/// Number of ideal Voltage Sources
		Int mNumIdealVS;

		/// LU decomposition of system matrix A
		Eigen::PartialPivLU<Matrix> mLuFactored;
		/// LU decomposition of system matrix A
		std::vector<Eigen::PartialPivLU<Matrix> > mLuFactoredVector;
		/// System matrix A that is modified by matrix stamps
		Matrix mSystemMatrix;
		/// System matrices list for swtiching events
		std::vector<Matrix> mSystemMatrixVector;
		/// Vector of known quantities
		Matrix mRightSideVector;
		/// Vector of unknown quantities
		Matrix mLeftSideVector;

	public:
		SystemModel() { }
		void initialize(Int numNodes);
		void createEmptySystemMatrix();
		void addSystemMatrix();
		void InitializeRightSideVector(Matrix& rightSideVector) { mRightSideVector = rightSideVector; }
		void InitializeLeftSideVector(Matrix& leftSideVector) { mLeftSideVector = leftSideVector; }
		void switchSystemMatrix(UInt systemMatrixIndex);
		void addRealToSystemMatrix(Matrix::Index row, Matrix::Index column, Real value);
		void addCompToSystemMatrix(Matrix::Index row, Matrix::Index column, Complex value);
		void setCompSystemMatrixElement(Matrix::Index row, Matrix::Index column, Complex value);
		void addCompToRightSideVector(Matrix::Index row, Complex value);
		void addRealToRightSideVector(Matrix::Index row, Real value);
		void setRightSideVectorToZero();
		void solve();
		void updateLuFactored();

		Matrix& getCurrentSystemMatrix() { return mSystemMatrix; }
		const Matrix& getLUdecomp() { return mLuFactored.matrixLU(); }
		Matrix& getRightSideVector() { return mRightSideVector; }
		Matrix& getLeftSideVector() { return mLeftSideVector; }
		Real getTimeStep() const { return mTimeStep; }
		Real getOmega() const { return mSystemOmega; }
		Int getCompOffset() const { return mCompOffset; }
		Real getRealFromLeftSideVector(Matrix::Index row) const { return mLeftSideVector(row, 0); }
		Complex getCompFromLeftSideVector(Matrix::Index row) const { return Complex(mLeftSideVector(row, 0), mLeftSideVector(row + mCompOffset, 0)); }
		SimulationType getSimType() const { return mSimType; }
		Int getNumNodes() const { return mNumNodes; }
		Int getNumIdealVS() const { return mNumIdealVS; }
		NumericalMethod getNumMethod() const { return mNumMethod; }

		void setSimType(SimulationType simType) { mSimType = simType; }
		void setTimeStep(Real timeStep) { mTimeStep = timeStep; }
		void setOmega(Real omega) { mSystemOmega = omega; }
		void setSystemMatrixElement(Matrix::Index row, Matrix::Index column, Real value) { mSystemMatrix(row, column) = value; }
		void setCompSystemMatrixElement(Matrix::Index row, Matrix::Index column, Real reValue, Real imValue);
		void setNumMethod(NumericalMethod numMethod) { mNumMethod = numMethod; }
	};
}
