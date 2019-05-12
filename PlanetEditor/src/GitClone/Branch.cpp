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

std::vector<Command>& Branch::getCommands() {
	return m_commands;
}

void Branch::resetCommandList() {
	m_commands.clear();
}
