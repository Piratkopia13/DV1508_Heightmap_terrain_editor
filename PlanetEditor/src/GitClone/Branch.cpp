#include "pch.h"
#include "Branch.h"


Branch::Branch() {
	m_name = "N/A";
	m_parent = nullptr;
}
Branch::Branch(std::string name, Area area, Branch* parent) {
	m_name = name;
	m_area = area;
	m_parent = parent;
	m_commits.reserve(100);
}

const std::string& Branch::getName() const {
	return m_name;
}

Branch* Branch::getParent() const {
	return m_parent;
}

Area Branch::getArea() const {
	return m_area;
}

void Branch::addCommand(Tool* tool, Command::Parameters params, std::vector<std::pair<unsigned int, XMFLOAT3>> newPosition) {
	m_commands.push_back({tool, params, newPosition});
}

void Branch::addCommand(Command cmd) {
	m_commands.push_back(cmd);
}

void Branch::createCommit(const std::string & author, const std::string & message, EditableMesh * mesh) {
	resetCommandList();
	m_commits.emplace_back(author, message, mesh);
}

void Branch::clearCommitsToIndex(unsigned int index) {
	while (m_commits.size() - 1 > index)
		m_commits.pop_back();
}

std::vector<Command>& Branch::getCommands() {
	return m_commands;
}

std::vector<Branch::Commit>& Branch::getCommits() {
	return m_commits;
}

void Branch::resetCommandList() {
	m_commands.clear();
}
