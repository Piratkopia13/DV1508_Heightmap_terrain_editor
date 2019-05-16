#pragma once
#include "Branch.h"

class BranchManager {
private:
	std::vector<Branch> m_branches;
	int m_index;
	int m_commandIndex;

public:
	BranchManager();
	~BranchManager() = default;

	bool loadBranches();
	bool saveBranches();

	bool addCommand(Tool* tool, Command::Parameters params, std::vector<std::pair<unsigned int, XMFLOAT3>> newPosition);
	
	std::vector<std::pair<unsigned int, XMFLOAT3>> undo();
	std::vector<std::pair<unsigned int, XMFLOAT3>> redo();
	std::vector<std::pair<unsigned int, XMFLOAT3>> undoTo(size_t index);
	std::vector<std::pair<unsigned int, XMFLOAT3>> redoTo(size_t index);
	
	const int getSize();
	std::vector<const char*> getBranchNames();

	const int getIndex();
	const int getCommandIndex();
	Branch& getCurrentBranch();
	std::vector<Branch>& getAllBranches();


	Area getCurrentArea();
	const bool canMerge();

	const bool createBranch(const std::string& name, const Area& area, Branch* parent, EditableMesh* initalMesh);
	const bool setBranch(size_t index);
	const bool setBranch(std::string name);



	void setCurrentCommand(Command& c);
	void setCurrentCommand(int index);
	const bool isCurrentCommand(const Command& c);
	const bool isCurrentCommand(int index);


/*
	bool canMerge() const {
		return branches[index].parent;
	}

	std::vector<Command>& getCommands() {
		return branches[index].commands;
	}

	bool isCurretCommand(const Command & c) const {
		return &branches[index].commands[currentCommand] == &c;
	}

	void setCurrentCommand(const Command & c) {
		currentCommand = &c - branches[index].commands.data();
	}*/
};