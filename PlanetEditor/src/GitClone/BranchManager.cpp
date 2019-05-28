#include "pch.h"
#include "BranchManager.h"


BranchManager::BranchManager() {
	//TODO: add loadbranches
	m_index = 0;
	//m_currentBranch = &m_branches[0];
	//m_currentCommand = nullptr;
	m_branches.reserve(50);
}
std::vector<const char*> BranchManager::getBranchNames() {
	std::vector<const char*> brNames;
	for (auto& b : m_branches)
		brNames.push_back(b.getName().c_str());
	return brNames;
}

const int BranchManager::getSize() {
	return m_branches.size();
}

const int BranchManager::getIndex() {
	return m_index;
}

int BranchManager::getIndexOf(const std::string& name) {
	for (int i = 0; i < m_branches.size(); i++) {
		if (m_branches[i].getName() == name) {
			return i;
			break;
		}
	}
	return -1;
}

Branch& BranchManager::getCurrentBranch() {
	return m_branches[m_index];
}

std::vector<Branch>& BranchManager::getAllBranches() {
	return m_branches;
}

Area BranchManager::getCurrentArea() {
	return getCurrentBranch().getArea();
}

bool BranchManager::validArea(Area a) {
	Area parent = getCurrentArea();

	for (int i = 1; i < m_branches.size(); ++i) {
		if (i == m_index)
			continue;

		Area temp = m_branches[i].getArea();
		if (!(a.maxX < temp.minX || a.minX > temp.maxX || a.minZ > temp.maxZ || a.maxZ < temp.minZ))
			return false;
	}
	if (parent.minX <= a.minX && parent.minZ <= a.minZ && parent.maxX >= a.maxX && parent.maxZ >= a.maxZ)
		return true;

	return false;
}

const bool BranchManager::canMerge() {
	return m_branches[m_index].getParent() && m_branches[m_index].getCommands().size() == 0;
}

void BranchManager::merge() {
	auto parent = m_branches[m_index].getParent();
	auto cs = parent->getCommits();
	auto c = cs[cs.size() - 1];
	EditableMesh* e = new EditableMesh(*c.mesh);
	e->updateData();
	cs = m_branches[m_index].getCommits();
	c = cs[cs.size() - 1];
	e->updateSubArea((c.mesh).get(), getCurrentArea());

	parent->createCommit(c.author, "Merge", e);
}

const bool BranchManager::createBranch(const std::string& name, const Area& area, Branch* parent, EditableMesh* initalMesh, std::string commitMsg) {
	m_branches.emplace_back(name, area, parent);
	m_branches[m_branches.size() - 1].createCommit("Branch", commitMsg, initalMesh);
	m_index = m_branches.size() - 1;
	return true;
}

const bool BranchManager::setBranch(size_t index) {
	if (index < m_branches.size()) {
		m_index = index;
		return true;
	} else
		return false;
}

const bool BranchManager::setBranch(std::string name) {
	for(int i = 0; i < m_branches.size(); i++) {
		if (m_branches[i].getName() == name) {
			m_index = i;
			return true;
		}
	}
	return false;
}





