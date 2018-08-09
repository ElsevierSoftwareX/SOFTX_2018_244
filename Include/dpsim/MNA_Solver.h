/** Simulation
 *
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
#include <list>

#include "Solver.h"
#ifdef WITH_CIM
#include "cps/CIM/Reader.h"
#endif /* WITH_CIM */

namespace DPsim {
	/// Simulation class which uses Modified Nodal Analysis (MNA).
	template <typename VarType>
	class MnaSolver : public Solver {
	protected:
		// General simulation settings
		/// System time step is constant for MNA solver
		Real mTimeStep;
		/// Simulation domain, which can be dynamic phasor (DP) or EMT
		CPS::Domain mDomain;
		/// Number of nodes
		UInt mNumNodes = 0;
		/// Number of network nodes
		UInt mNumNetNodes = 0;
		/// Number of virtual nodes
		UInt mNumVirtualNodes = 0;
		/// Number of simulation nodes
		UInt mNumSimNodes = 0;
		/// Number of simulation network nodes
		UInt mNumNetSimNodes = 0;
		/// Number of simulation virtual nodes
		UInt mNumVirtualSimNodes = 0;
		/// Flag to activate power flow based initialization.
		/// If this is false, all voltages are initialized with zero.
		Bool mPowerflowInitialization;
		/// System list
		CPS::SystemTopology mSystem;
		/// 
		typename CPS::Node<VarType>::List mNodes;
		/// 
		typename CPS::PowerComponent<VarType>::List mPowerComponents;
		/// 
		CPS::SignalComponent::List mSignalComponents;
		
		// #### MNA specific attributes ####
		/// System matrix A that is modified by matrix stamps
		UInt mSystemIndex = 0;
		/// System matrices list for swtiching events
		std::vector<Matrix> mSystemMatrices;
		/// LU decomposition of system matrix A
		std::vector<CPS::LUFactorized> mLuFactorizations;
		/// Vector of known quantities
		Matrix mRightSideVector;
		/// Vector of unknown quantities
		Matrix mLeftSideVector;
		/// Switch to trigger steady-state initialization
		Bool mSteadyStateInit = false;

		// #### Attributes related to switching ####
		/// Index of the next switching event
		UInt mSwitchTimeIndex = 0;
		/// Vector of switch times
		std::vector<SwitchConfiguration> mSwitchEvents;

		// #### Attributes related to logging ####
		/// Last simulation time step when log was updated
		Int mLastLogTimeStep = 0;
		/// Down sampling rate for log
		Int mDownSampleRate = 1;
		/// Simulation log level
		CPS::Logger::Level mLogLevel;
		/// Simulation logger
		CPS::Logger mLog;
		/// Left side vector logger
		CPS::Logger mLeftVectorLog;
		/// Right side vector logger
		CPS::Logger mRightVectorLog;

		/// TODO: check that every system matrix has the same dimensions
		void initialize(CPS::SystemTopology system) {
			mLog.info() << "#### Start Initialization ####" << std::endl;
			mSystem = system;

			// We need to differentiate between power and signal components and
			// ground nodes should be ignored.
			IdentifyTopologyObjects();

			// These steps complete the network information.
			createVirtualNodes();
			assignSimNodes();					

			// For the power components the step order should not be important
			// but signal components need to be executed following the connections.
			sortExecutionPriority();

			// The system topology is prepared and we create the MNA matrices.
			createEmptyVectors();
			createEmptySystemMatrix();

			// TODO: Move to base solver class?
			// This intialization according to power flow information is not MNA specific.
			mLog.info() << "Initialize power flow" << std::endl;
			for (auto comp : mPowerComponents)
				comp->initializePowerflow(mSystem.mSystemFrequency);

			// This steady state initialization is MNA specific and runs a simulation 
			// before the actual simulation executed by the user.
			if (mSteadyStateInit && mDomain == CPS::Domain::DP) {
				mLog.info() << "Run steady-state initialization." << std::endl;
				steadyStateInitialization();
			}

			// Now, initialize the components for the actual simulation.			
			for (auto comp : mSignalComponents) {
				comp->initialize();
			}
			for (auto comp : mPowerComponents) {
				comp->mnaInitialize(mSystem.mSystemOmega, mTimeStep);
				comp->mnaApplyRightSideVectorStamp(mRightSideVector);
				comp->mnaApplySystemMatrixStamp(mSystemMatrices[0]);
			}
						
			// Compute LU-factorization for system matrix
			mLuFactorizations.push_back(Eigen::PartialPivLU<Matrix>(mSystemMatrices[0]));
		}

		/// Identify Nodes and PowerComponents and SignalComponents
		void IdentifyTopologyObjects() {			
			for (auto baseNode : mSystem.mNodes) {	
				// Add nodes to the list and ignore ground nodes.
				if (!baseNode->isGround()) {			
					auto node = std::dynamic_pointer_cast< CPS::Node<VarType> >(baseNode);
					mNodes.push_back( node );	
				}
			}
			
			for (UInt i = 0; i < mNodes.size(); i++) {
				mLog.info() << "Found node " << mNodes[i]->getName() << std::endl;
			}

			for (auto comp : mSystem.mComponents) {
				if ( typename CPS::PowerComponent<VarType>::Ptr powercomp = std::dynamic_pointer_cast< CPS::PowerComponent<VarType> >(comp) ) {
					mPowerComponents.push_back(powercomp);		
				}
				else if ( CPS::SignalComponent::Ptr signalcomp = std::dynamic_pointer_cast< CPS::SignalComponent >(comp) )	{	
					mSignalComponents.push_back(signalcomp);	
				}
			}
		}

		void sortExecutionPriority() {			
			// Sort SignalComponents according to execution priority.
			// Components with a higher priority number should come first.
			std::sort(mSignalComponents.begin(), mSignalComponents.end(), [](const auto& lhs, const auto& rhs) {
				return lhs->getPriority() > rhs->getPriority();
			});
		}

		/// Assign simulation node index according to index in the vector.
		void assignSimNodes() {			
			UInt simNodeIdx = 0;
			for (UInt idx = 0; idx < mNodes.size(); idx++) {
				mNodes[idx]->setSimNode(0, simNodeIdx);
				simNodeIdx++;
				if (mNodes[idx]->getPhaseType() == CPS::PhaseType::ABC) {
					mNodes[idx]->setSimNode(1, simNodeIdx);
					simNodeIdx++;
					mNodes[idx]->setSimNode(2, simNodeIdx);
					simNodeIdx++;
				}
				if (idx == mNumNetNodes-1) mNumNetSimNodes = simNodeIdx;
			}	
			// Total number of network nodes is simNodeIdx + 1
			mNumSimNodes = simNodeIdx;	
			mNumVirtualSimNodes = mNumSimNodes - mNumNetSimNodes;

			mLog.info() << "Number of network simulation nodes: " << mNumNetSimNodes << std::endl;
			mLog.info() << "Number of simulation nodes: " << mNumSimNodes << std::endl;
		}

		/// Creates virtual nodes inside components.
		/// The MNA algorithm handles these nodes in the same way as network nodes.
		void createVirtualNodes() {
			// We have not added virtual nodes yet so the list has only network nodes
			mNumNetNodes = mNodes.size();
			// virtual nodes are placed after network nodes
			UInt virtualNode = mNumNetNodes - 1;
			// Check if component requires virtual node and if so set one
			for (auto comp : mPowerComponents) {
				if (comp->hasVirtualNodes()) {
					for (UInt node = 0; node < comp->getVirtualNodesNum(); node++) {
						virtualNode++;
						mNodes.push_back(std::make_shared<CPS::Node<VarType>>(virtualNode));
						comp->setVirtualNodeAt(mNodes[virtualNode], node);
						mLog.info() << "Created virtual node" << node << " = " << virtualNode
							<< " for " << comp->getName() << std::endl;
					}
				}
			}
			// Update node number to create matrices and vectors
			mNumNodes = mNodes.size();
			mNumVirtualNodes = mNumNodes - mNumNetNodes;

			mLog.info() << "Number of network nodes: " << mNumNetNodes << std::endl;
			mLog.info() << "Number of nodes: " << mNumNodes << std::endl;
		}

		// TODO: check if this works with AC sources
		void steadyStateInitialization() {
			Real time = 0;

			// Initialize right side vector and components
			for (auto comp : mPowerComponents) {
				comp->mnaInitialize(mSystem.mSystemOmega, mTimeStep);
				comp->mnaApplyRightSideVectorStamp(mRightSideVector);
				comp->mnaApplySystemMatrixStamp(mSystemMatrices[0]);
				comp->mnaApplyInitialSystemMatrixStamp(mSystemMatrices[0]);
			}
			// Compute LU-factorization for system matrix
			mLuFactorizations.push_back(Eigen::PartialPivLU<Matrix>(mSystemMatrices[0]));

			Matrix prevLeftSideVector = Matrix::Zero(2 * mNumNodes, 1);
			Matrix diff;
			Real maxDiff, max;

			while (time < 10) {
				time = step(time);

				diff = prevLeftSideVector - mLeftSideVector;
				prevLeftSideVector = mLeftSideVector;
				maxDiff = diff.lpNorm<Eigen::Infinity>();
				max = mLeftSideVector.lpNorm<Eigen::Infinity>();

				if ((maxDiff / max) < 0.0001)
					break;
			}
			mLog.info() << "Initialization finished. Max difference: "
				<< maxDiff << " or " << maxDiff / max << "% at time " << time << std::endl;
			mSystemMatrices.pop_back();
			mLuFactorizations.pop_back();
			mSystemIndex = 0;

			createEmptySystemMatrix();
		}

		/// Create left and right side vector	
		void createEmptyVectors();

		/// Create system matrix
		void createEmptySystemMatrix();

		/// TODO remove and replace with function that handles breakers
		void addSystemTopology(CPS::SystemTopology system) {
			mSystem = system;

			// It is assumed that the system size does not change
			createEmptySystemMatrix();
			for (auto comp : system.mComponents)
				comp->mnaApplySystemMatrixStamp(mSystemMatrices[mSystemMatrices.size()]);

			mLuFactorizations.push_back(CPS::LUFactorized(mSystemMatrices[mSystemMatrices.size()]));
		}

		/// TODO This should be activated by switch/breaker components
		void switchSystemMatrix(UInt systemIndex) {
			if (systemIndex < mSystemMatrices.size())
				mSystemIndex = systemIndex;
		}

		/// Solve system matrices
		void solve()  {
			mLeftSideVector = mLuFactorizations[mSystemIndex].solve(mRightSideVector);
		}

	public:
		/// This constructor should not be called by users.
		MnaSolver(String name,
			Real timeStep,
			CPS::Domain domain = CPS::Domain::DP,
			CPS::Logger::Level logLevel = CPS::Logger::Level::INFO,
			Bool steadyStateInit = false, Int downSampleRate = 1) :			
			mLog("Logs/" + name + "_MNA.log", logLevel),
			mLeftVectorLog("Logs/" + name + "_LeftVector.csv", logLevel),
			mRightVectorLog("Logs/" + name + "_RightVector.csv", logLevel) {

			mTimeStep = timeStep;
			mDomain = domain;
			mLogLevel = logLevel;
			mDownSampleRate = downSampleRate;
			mSteadyStateInit = steadyStateInit;
		}

		/// Constructor to be used in simulation examples.
		MnaSolver(String name, CPS::SystemTopology system,
			Real timeStep,
			CPS::Domain domain = CPS::Domain::DP,
			CPS::Logger::Level logLevel = CPS::Logger::Level::INFO,
			Bool steadyStateInit = false,
			Int downSampleRate = 1)
			: MnaSolver(name, timeStep, domain,
			logLevel, steadyStateInit, downSampleRate) {
			initialize(system);

			// Logging
			for (auto comp : system.mComponents)
				mLog.info() << "Added " << comp->getType() << " '" << comp->getName() << "' to simulation." << std::endl;

			mLog.info() << "System matrix:" << std::endl;
			mLog.info(mSystemMatrices[0]);
			mLog.info() << "LU decomposition:" << std::endl;
			mLog.info(mLuFactorizations[0].matrixLU());
			mLog.info() << "Right side vector:" << std::endl;
			mLog.info(mRightSideVector);
		}

		///
		virtual ~MnaSolver() { };

		/// Solve system A * x = z for x and current time
		Real step(Real time) {
			mRightSideVector.setZero();

			// First, step signal components and then power components
			for (auto comp : mSignalComponents) {
				comp->step(time);
			}
			for (auto comp : mPowerComponents) {
				comp->mnaStep(mSystemMatrices[mSystemIndex], mRightSideVector, mLeftSideVector, time);
			}

			// Solve MNA system
			solve();

			// Some components need to update internal states
			for (auto comp : mPowerComponents) {
				comp->mnaPostStep(mRightSideVector, mLeftSideVector, time);
			}

			// TODO Try to avoid this step.
			for (UInt nodeIdx = 0; nodeIdx < mNumNetNodes; nodeIdx++) {
				mNodes[nodeIdx]->mnaUpdateVoltage(mLeftSideVector);
			}

			// Handle switching events
			if (mSwitchTimeIndex < mSwitchEvents.size()) {
				if (time >= mSwitchEvents[mSwitchTimeIndex].switchTime) {
					switchSystemMatrix(mSwitchEvents[mSwitchTimeIndex].systemIndex);
					++mSwitchTimeIndex;

					mLog.info() << "Switched to system " << mSwitchTimeIndex << " at " << time << std::endl;
					mLog.info() << "New matrix:" << std::endl << mSystemMatrices[mSystemIndex] << std::endl;
					mLog.info() << "New decomp:" << std::endl << mLuFactorizations[mSystemIndex].matrixLU() << std::endl;
				}
			}

			// Calculate new simulation time
			return time + mTimeStep;
		}

		/// Log left and right vector values for each simulation step
		void log(Real time) {
			mLeftVectorLog.LogNodeValues(time, getLeftSideVector());
			mRightVectorLog.LogNodeValues(time, getRightSideVector());
		}

		/// TODO This should be handled by a switch/breaker component
		void setSwitchTime(Real switchTime, Int systemIndex) {
			SwitchConfiguration newSwitchConf;
			newSwitchConf.switchTime = switchTime;
			newSwitchConf.systemIndex = systemIndex;
			mSwitchEvents.push_back(newSwitchConf);
		}

		// #### Getter ####
		Matrix& getLeftSideVector() { return mLeftSideVector; }
		Matrix& getRightSideVector() { return mRightSideVector; }
		Matrix& getSystemMatrix() { return mSystemMatrices[mSystemIndex]; }
	};

	
}
