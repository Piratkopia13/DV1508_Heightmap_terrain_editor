#include "pch.h"
#include "Branch.h"


Branch::Branch() {
	m_name = "N/A";
	m_parent = nullptr;

	m_commandIndex = -1;
}
Branch::Branch(std::string name, Area area, Branch* parent) {
	m_name = name;
	m_area = area;
	m_parent = parent;
	m_commits.reserve(100);

	m_commandIndex = -1;
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
	m_commandIndex++;
	m_commands.insert(m_commands.begin() + m_commandIndex, {tool, params, newPosition});
}

void Branch::addCommand(Command cmd) {
	m_commandIndex++;
	m_commands.push_back(cmd);
}

void Branch::createCommit(const std::string & author, const std::string & message, EditableMesh * mesh) {
	clearCommandsToCurrentCommandIndex();
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
	m_commandIndex = -1;
	m_commands.clear();
}

void Branch::clearCommandsToIndex(unsigned int index) {
	if (m_commands.size() > 0) {
		if (index == m_commands.size() - 1)
			resetCommandList();
		else {
			auto eraseTo = m_commands.begin();
			eraseTo += index + 1;
			m_commands.erase(m_commands.begin(), eraseTo);
			m_commandIndex = -1;
		}
	}
}

void Branch::clearCommandsToCurrentCommandIndex() {
	clearCommandsToIndex(m_commandIndex);
}


const int Branch::getCommandIndex() {
	return m_commandIndex;
}

std::vector<std::pair<unsigned int, XMFLOAT3>> Branch::undo() {
	if (m_commandIndex >= 0) {
		std::vector<std::pair<unsigned int, XMFLOAT3>> toReturn(getCommands()[m_commandIndex--].newPosition);
		for (auto& pair : toReturn) {
			pair.second.x *= -1;
			pair.second.y *= -1;
			pair.second.z *= -1;
		}
		std::cout << "commandIndex: " << m_commandIndex << std::endl;
		return toReturn;
	}
	std::cout << "commandIndex: " << m_commandIndex << std::endl;
	return std::vector<std::pair<unsigned int, XMFLOAT3>>();
}

std::vector<std::pair<unsigned int, XMFLOAT3>> Branch::redo() {
	if (m_commandIndex < (int)getCommands().size() - 1) {
		std::cout << "commandIndex: " << m_commandIndex << std::endl;
		return getCommands()[++m_commandIndex].newPosition;
	}
	std::cout << "commandIndex: " << m_commandIndex << std::endl;
	return std::vector<std::pair<unsigned int, XMFLOAT3>>();
}

std::vector<std::pair<unsigned int, XMFLOAT3>> Branch::undoTo(size_t index) {
	if (index == m_commandIndex)
		return std::vector<std::pair<unsigned int, XMFLOAT3>>();
	std::vector<std::pair<unsigned int, XMFLOAT3>> sum;
	std::vector<std::pair<unsigned int, XMFLOAT3>> temp;
	while (m_commandIndex > index) {
		temp = undo();
		sum.insert(sum.end(), temp.begin(), temp.end());
	}


	return sum;
}

std::vector<std::pair<unsigned int, XMFLOAT3>> Branch::redoTo(size_t index) {
	if (index == m_commandIndex)
		return std::vector<std::pair<unsigned int, XMFLOAT3>>();
	std::vector<std::pair<unsigned int, XMFLOAT3>> sum;
	std::vector<std::pair<unsigned int, XMFLOAT3>> temp;
	while (m_commandIndex < index) {
		temp = redo();
		sum.insert(sum.end(), temp.begin(), temp.end());
	}

	return sum;
}

void Branch::setCurrentCommand(Command& c) {
	m_commandIndex = &c - getCommands().data();
}

void Branch::setCurrentCommand(int index) {
	if (index < getCommands().size()) {
		m_commandIndex = index;
	}
}

const bool Branch::isCurrentCommand(const Command& c) {
	return &getCommands()[m_commandIndex] == &c;
}

const bool Branch::isCurrentCommand(int index) {
	return index == m_commandIndex;
}
