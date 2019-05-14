#include "pch.h"
#include "Branch.h"


Branch::Branch() {
	m_name = "N/A";
	m_parent = nullptr;
}
Branch::Branch(std::string name, Branch* parent) {
	m_name = name;
	m_parent = parent;
}

const std::string& Branch::getName() {
	return m_name;
}

const Branch* Branch::getParent() {
	return m_parent;
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

std::vector<Command>& Branch::getCommands() {
	return m_commands;
}

std::vector<Branch::Commit>& Branch::getCommits() {
	return m_commits;
}

void Branch::resetCommandList() {
	m_commands.clear();
}
