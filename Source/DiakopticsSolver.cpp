/** Diakoptics solver
 *
 * @author Georg Reinke <georg.reinke@rwth-aachen.de>
 * @copyright 2017-2019, Institute for Automation of Complex Power Systems, EONERC
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

#include <dpsim/DiakopticsSolver.h>

#include <iomanip>

#include <cps/MathUtils.h>
#include <cps/Solver/MNATearInterface.h>
#include <dpsim/Definitions.h>
#include <dpsim/Simulation.h>

using namespace CPS;
using namespace DPsim;

namespace DPsim {

template <typename VarType>
DiakopticsSolver<VarType>::DiakopticsSolver(String name, SystemTopology system, Component::List tearComponents, Real timeStep, Logger::Level logLevel) :
	mName(name),
	mTimeStep(timeStep),
	mSystemFrequency(system.mSystemFrequency),
	mLog(name + "_Diakoptics", logLevel),
	mLeftVectorLog(name + "_LeftVector", logLevel != Logger::Level::NONE),
	mRightVectorLog(name + "_RightVector", logLevel != Logger::Level::NONE) {
	for (auto comp : tearComponents) {
		auto pcomp = std::dynamic_pointer_cast<PowerComponent<VarType>>(comp);
		if (pcomp)
			mTearComponents.push_back(pcomp);
	}
	init(system);
}

template <typename VarType>
void DiakopticsSolver<VarType>::init(const SystemTopology& system) {
	std::vector<SystemTopology> subnets;

	Simulation::splitSubnets<VarType>(system, subnets);
	initSubnets(subnets);
	setLogColumns();
	createMatrices();
	initComponents();
	initMatrices();
}

template <typename VarType>
void DiakopticsSolver<VarType>::initSubnets(const std::vector<SystemTopology>& subnets) {
	mSubnets.resize(subnets.size());
	for (UInt i = 0; i < subnets.size(); i++) {
		for (auto baseNode : subnets[i].mNodes) {
			// Add nodes to the list and ignore ground nodes.
			if (!baseNode->isGround()) {
				auto node = std::dynamic_pointer_cast< CPS::Node<VarType> >(baseNode);
				mSubnets[i].nodes.push_back(node);
			}
		}
		for (auto comp : subnets[i].mComponents) {
			// TODO switches
			auto mnaComp = std::dynamic_pointer_cast<CPS::MNAInterface>(comp);
			if (mnaComp)
				mSubnets[i].components.push_back(mnaComp);

			auto sigComp = std::dynamic_pointer_cast<CPS::SignalComponent>(comp);
			if (sigComp)
				mSignalComponents.push_back(sigComp);
		}
	}

	for (auto& net : mSubnets) {
		for (auto& node : net.nodes) {
			mNodeSubnetMap[node] = &net;
		}
	}

	for (UInt idx = 0; idx < mTearComponents.size(); idx++) {
		auto comp = mTearComponents[idx];
		auto tComp = std::dynamic_pointer_cast<MNATearInterface>(comp);
		if (!tComp)
			throw SystemError("Unsupported component type for diakoptics");

		if (comp->hasVirtualNodes()) {
			for (UInt node = 0; node < comp->virtualNodesNumber(); node++) {
				// sim node number doesn't matter here because it shouldn't be used anyway
				comp->setVirtualNodeAt(std::make_shared<CPS::Node<VarType>>(node), node);
			}
		}
		tComp->mnaTearSetIdx(idx);
		comp->initializeFromPowerflow(mSystemFrequency);
		tComp->mnaTearInitialize(2 * PI * mSystemFrequency, mTimeStep);

		for (auto gndComp : tComp->mnaTearGroundComponents()) {
			auto pComp = std::dynamic_pointer_cast<PowerComponent<VarType>>(gndComp);
			Subnet* net = nullptr;
			if (pComp->node(0)->isGround()) {
				net = mNodeSubnetMap[pComp->node(1)];
			} else if (pComp->node(1)->isGround()) {
				net = mNodeSubnetMap[pComp->node(0)];
			} else {
				throw SystemError("Invalid ground component passed from torn component");
			}
			net->components.push_back(gndComp);
		}
	}

	for (UInt i = 0; i < subnets.size(); i++) {
		createVirtualNodes(i);
		assignSimNodes(i);
	}
}

template <typename VarType>
void DiakopticsSolver<VarType>::createVirtualNodes(int net) {
	UInt virtualNode = mSubnets[net].nodes.size() - 1;
	for (auto comp : mSubnets[net].components) {
		auto pComp = std::dynamic_pointer_cast<PowerComponent<VarType>>(comp);
		if (!pComp)
			continue;

		if (pComp->hasVirtualNodes()) {
			for (UInt node = 0; node < pComp->virtualNodesNumber(); node++) {
				virtualNode++;
				mSubnets[net].nodes.push_back(std::make_shared<CPS::Node<VarType>>(virtualNode));
				pComp->setVirtualNodeAt(mSubnets[net].nodes[virtualNode], node);

				mLog.info() << "Created virtual node" << node << " = " << virtualNode
					<< " for " << pComp->name() << std::endl;
			}
		}
	}
}

template <typename VarType>
void DiakopticsSolver<VarType>::assignSimNodes(int net) {
	UInt simNodeIdx = 0;
	for (UInt idx = 0; idx < mSubnets[net].nodes.size(); idx++) {
		auto& node = mSubnets[net].nodes[idx];
		node->setSimNode(0, simNodeIdx);
		simNodeIdx++;
		if (node->phaseType() == CPS::PhaseType::ABC) {
			node->setSimNode(1, simNodeIdx);
			simNodeIdx++;
			node->setSimNode(2, simNodeIdx);
			simNodeIdx++;
		}
	}
	setSubnetSize(net, simNodeIdx);
	if (net == 0)
		mSubnets[net].sysOff = 0;
	else
		mSubnets[net].sysOff = mSubnets[net-1].sysOff + mSubnets[net-1].sysSize;
}

template<>
void DiakopticsSolver<Real>::setSubnetSize(int net, UInt nodes) {
	mSubnets[net].sysSize = nodes;
}

template<>
void DiakopticsSolver<Complex>::setSubnetSize(int net, UInt nodes) {
	mSubnets[net].sysSize = 2 * nodes;
}

template<>
void DiakopticsSolver<Real>::setLogColumns() {
	// nothing to do, column names generated by DataLogger are correct
}

template<>
void DiakopticsSolver<Complex>::setLogColumns() {
	std::vector<String> names;
	for (auto& subnet : mSubnets) {
		for (UInt i = subnet.sysOff; i < subnet.sysOff + subnet.sysSize; i++) {
			std::stringstream name;
			if (i < subnet.sysOff + subnet.sysSize / 2)
				name << "node" << std::setfill('0') << std::setw(5) << i - subnet.sysOff / 2 << ".real";
			else
				name << "node" << std::setfill('0') << std::setw(5) << i - (subnet.sysOff + subnet.sysSize) / 2 << ".imag";
			names.push_back(name.str());
		}
	}
	mLeftVectorLog.setColumnNames(names);
	mRightVectorLog.setColumnNames(names);
}

template <typename VarType>
void DiakopticsSolver<VarType>::createMatrices() {
	UInt totalSize = mSubnets.back().sysOff + mSubnets.back().sysSize;
	mSystemMatrix = Matrix::Zero(totalSize, totalSize);
	mYinv = Matrix::Zero(totalSize, totalSize);

	mRightSideVector = Matrix::Zero(totalSize, 1);
	mLeftSideVector = Matrix::Zero(totalSize, 1);
	mOrigLeftSideVector = Matrix::Zero(totalSize, 1);
	addAttribute<Matrix>("xOld", &mOrigLeftSideVector, Flags::read);
	mIPsiMapped = Matrix::Zero(totalSize, 1);

	for (auto& net : mSubnets) {
		// The subnets' components expect to be passed a left-side vector matching
		// the size of the subnet, so we have to create separate vectors here and
		// copy the solution there
		net.leftVector = Attribute<Matrix>::make(Flags::read | Flags::write);
		net.leftVector->set(Matrix::Zero(net.sysSize, 1));
	}

	createTearMatrices(totalSize);
}

template <>
void DiakopticsSolver<Real>::createTearMatrices(UInt totalSize) {
	mTearTopology = Matrix::Zero(totalSize, mTearComponents.size());
	mRemovedImpedance = Matrix::Zero(mTearComponents.size(), mTearComponents.size());
	mIPsi = Matrix::Zero(mTearComponents.size(), 1);
	mEPsi = Matrix::Zero(mTearComponents.size(), 1);
}

template <>
void DiakopticsSolver<Complex>::createTearMatrices(UInt totalSize) {
	mTearTopology = Matrix::Zero(totalSize, 2*mTearComponents.size());
	mRemovedImpedance = Matrix::Zero(2*mTearComponents.size(), 2*mTearComponents.size());
	mIPsi = Matrix::Zero(2*mTearComponents.size(), 1);
	mEPsi = Matrix::Zero(2*mTearComponents.size(), 1);
}

template <typename VarType>
void DiakopticsSolver<VarType>::initComponents() {
	for (UInt net = 0; net < mSubnets.size(); net++) {
		for (auto comp : mSubnets[net].components) {
			auto pComp = std::dynamic_pointer_cast<PowerComponent<VarType>>(comp);
			if (!pComp)
				continue;
			pComp->initializeFromPowerflow(mSystemFrequency);
		}

		// Initialize MNA specific parts of components.
		for (auto comp : mSubnets[net].components) {
			comp->mnaInitialize(2 * PI * mSystemFrequency, mTimeStep, mSubnets[net].leftVector);
			const Matrix& stamp = comp->template attribute<Matrix>("right_vector")->get();
			if (stamp.size() != 0) {
				mSubnets[net].rightVectorStamps.push_back(&stamp);
			}
		}
	}
	// Initialize signal components.
	for (auto comp : mSignalComponents)
		comp->initialize(2 * PI * mSystemFrequency, mTimeStep);
}

template <typename VarType>
void DiakopticsSolver<VarType>::initMatrices() {
	for (auto& net : mSubnets) {
		// We can't directly pass the block reference to mnaApplySystemMatrixStamp,
		// because it expects a concrete Matrix. It can't be changed to accept some common
		// base class like DenseBase because that would make it a template function (as
		// Eigen uses CRTP for polymorphism), which is impossible for virtual functions.
		Matrix partSys = Matrix::Zero(net.sysSize, net.sysSize);
		for (auto comp : net.components) {
			comp->mnaApplySystemMatrixStamp(partSys);
		}
		auto block = mSystemMatrix.block(net.sysOff, net.sysOff, net.sysSize, net.sysSize);
		block = partSys;
		mLog.debug() << "Block: \n" << block;
		net.luFactorization = Eigen::PartialPivLU<Matrix>(partSys);
		mLog.debug() << "Factorization: \n" << net.luFactorization.matrixLU() << std::endl;
	}
	mLog.debug() << "Complete system matrix:\n" << mSystemMatrix;


	// initialize tear topology matrix and impedance matrix of removed network
	for (UInt compIdx = 0; compIdx < mTearComponents.size(); compIdx++) {
		applyTearComponentStamp(compIdx);
	}
	mLog.debug() << "Topology matrix: \n" << mTearTopology << std::endl;
	mLog.debug() << "Removed impedance matrix: \n" << mRemovedImpedance << std::endl;
	// TODO this can be sped up as well by using the block diagonal form of Yinv
	for (auto& net : mSubnets) {
		mYinv.block(net.sysOff, net.sysOff, net.sysSize, net.sysSize) = net.luFactorization.inverse();
	}
	mNetToRemovedImpedance = Eigen::PartialPivLU<Matrix>(mRemovedImpedance + mTearTopology.transpose() * mYinv * mTearTopology);
	mLog.debug() << "Net to removed impedance matrix decomposition: \n" << mNetToRemovedImpedance.matrixLU() << std::endl;
}

template <>
void DiakopticsSolver<Real>::applyTearComponentStamp(UInt compIdx) {
	auto comp = mTearComponents[compIdx];
	mTearTopology(mNodeSubnetMap[comp->node(0)]->sysOff + comp->node(0)->simNode(), compIdx) = 1;
	mTearTopology(mNodeSubnetMap[comp->node(1)]->sysOff + comp->node(1)->simNode(), compIdx) = -1;

	auto tearComp = std::dynamic_pointer_cast<MNATearInterface>(comp);
	tearComp->mnaTearApplyMatrixStamp(mRemovedImpedance);
}

template <>
void DiakopticsSolver<Complex>::applyTearComponentStamp(UInt compIdx) {
	auto comp = mTearComponents[compIdx];

	auto net1 = mNodeSubnetMap[comp->node(0)];
	auto net2 = mNodeSubnetMap[comp->node(1)];

	mTearTopology(net1->sysOff + comp->node(0)->simNode(), compIdx) = 1;
	mTearTopology(net1->sysOff + net1->sysSize/2 + comp->node(0)->simNode(), mTearComponents.size() + compIdx) = 1;
	mTearTopology(net2->sysOff + comp->node(1)->simNode(), compIdx) = -1;
	mTearTopology(net2->sysOff + net2->sysSize/2 + comp->node(1)->simNode(), mTearComponents.size() + compIdx) = -1;

	auto tearComp = std::dynamic_pointer_cast<MNATearInterface>(comp);
	tearComp->mnaTearApplyMatrixStamp(mRemovedImpedance);
}

template <typename VarType>
Task::List DiakopticsSolver<VarType>::getTasks() {
	Task::List l;

	for (UInt net = 0; net < mSubnets.size(); net++) {
		for (auto comp : mSubnets[net].components) {
			for (auto task : comp->mnaTasks()) {
				l.push_back(task);
			}
		}
		l.push_back(std::make_shared<SubnetSolveTask>(*this, net));
	}

	for (auto comp : mSignalComponents) {
		for (auto task : comp->getTasks()) {
			l.push_back(task);
		}
	}
	l.push_back(std::make_shared<SolveTask>(*this));
	l.push_back(std::make_shared<LogTask>(*this));

	return l;
}

template <typename VarType>
void DiakopticsSolver<VarType>::SubnetSolveTask::execute(Real time, Int timeStepCount) {
	auto rBlock = mSolver.mRightSideVector.block(mSubnet.sysOff, 0, mSubnet.sysSize, 1);
	rBlock.setZero();

	for (auto stamp : mSubnet.rightVectorStamps)
		rBlock += *stamp;

	auto lBlock = mSolver.mOrigLeftSideVector.block(mSubnet.sysOff, 0, mSubnet.sysSize, 1);
	lBlock = mSubnet.luFactorization.solve(rBlock);
}

template <typename VarType>
void DiakopticsSolver<VarType>::SolveTask::execute(Real time, Int timeStepCount) {
	mSolver.mEPsi.setZero();
	for (auto comp : mSolver.mTearComponents) {
		auto tComp = std::dynamic_pointer_cast<MNATearInterface>(comp);
		tComp->mnaTearApplyVoltageStamp(mSolver.mEPsi);
	}
	mSolver.mEPsi -= mSolver.mTearTopology.transpose() * mSolver.mOrigLeftSideVector;
	mSolver.mIPsi = mSolver.mNetToRemovedImpedance.solve(mSolver.mEPsi);
	mSolver.mIPsiMapped = mSolver.mTearTopology * mSolver.mIPsi;
	mSolver.mLeftSideVector = mSolver.mOrigLeftSideVector;
	// TODO parallelize as well? would also enable to parallelize subnet components better
	for (auto& net : mSolver.mSubnets) {
		auto lBlock = mSolver.mLeftSideVector.block(net.sysOff, 0, net.sysSize, 1);
		auto rBlock = mSolver.mIPsiMapped.block(net.sysOff, 0, net.sysSize, 1);
		lBlock += net.luFactorization.solve(rBlock);
		*net.leftVector = lBlock;
	}
	// pass the voltages and current of the solution to the torn components
	mSolver.mEPsi = -mSolver.mTearTopology.transpose() * mSolver.mLeftSideVector;
	for (UInt compIdx = 0; compIdx < mSolver.mTearComponents.size(); compIdx++) {
		auto comp = mSolver.mTearComponents[compIdx];
		auto tComp = std::dynamic_pointer_cast<MNATearInterface>(comp);
		Complex voltage = Math::complexFromVectorElement(mSolver.mEPsi, compIdx);
		Complex current = Math::complexFromVectorElement(mSolver.mIPsi, compIdx);
		tComp->mnaTearPostStep(voltage, current);
	}
}

template <>
void DiakopticsSolver<Real>::log(Real time) {
	mLeftVectorLog.logEMTNodeValues(time, mLeftSideVector);
	mRightVectorLog.logEMTNodeValues(time, mRightSideVector);
}

template <>
void DiakopticsSolver<Complex>::log(Real time) {
	mLeftVectorLog.logPhasorNodeValues(time, mLeftSideVector);
	mRightVectorLog.logPhasorNodeValues(time, mRightSideVector);
}

template <typename VarType>
void DiakopticsSolver<VarType>::LogTask::execute(Real time, Int timeStepCount) {
	mSolver.log(time);
}

template class DiakopticsSolver<Real>;
template class DiakopticsSolver<Complex>;

}
