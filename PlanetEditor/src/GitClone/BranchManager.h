#pragma once
#include "Branch.h"

class BranchManager {
private:
	std::vector<Branch> m_branches;
	int m_index;

public:
	BranchManager();
	~BranchManager() = default;

	bool loadBranches();
	bool saveBranches();
	
	const int getSize();
	std::vector<const char*> getBranchNames();

	const int getIndex();
	int getIndexOf(const std::string& name);
	Branch& getCurrentBranch();
	std::vector<Branch>& getAllBranches();


	Area getCurrentArea();
	bool validArea(Area a);
	const bool canMerge();
	void merge();


	const bool createBranch(const std::string& name, const Area& area, Branch* parent, EditableMesh* initalMesh, std::string commitMsg = "Branch created");
	const bool setBranch(size_t index);
	const bool setBranch(std::string name);


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