#pragma once

#include "IconsFontAwesome5.h"

struct Command {
	std::string name;
	char* icon;
};

struct Branch {
	std::string name;
	std::vector<Command> commands;
//private:
	Branch* parent = nullptr;

public:
	Branch() = default;
	Branch(std::string s) : name(s) {}
	Branch(std::string s, Branch* p) : name(s), parent(p) {}
	~Branch() = default;
};

class BranchManager {
private:
	std::vector<Branch> branches;
	int index = 0;
	int currentCommand = 0;

public:
	BranchManager() {
		branches.push_back(Branch("Master"));
		// Add dummy commands
		for (int i = 0; i < 100; i++) {
			branches[0].commands.push_back({ "Generate", ICON_FA_PLUS });
			branches[0].commands.push_back({ "Move", ICON_FA_ARROWS_ALT });
			branches[0].commands.push_back({ "Rotate", ICON_FA_UNDO });
		}

		branches.push_back(Branch("Kaka"));
		branches[1].commands.push_back({ "Generate", ICON_FA_PLUS });
	}

	~BranchManager() = default;

	std::vector<const char*> getBranchNames() const {
		std::vector<const char*> brNames;
		for (auto& b : branches)
			brNames.push_back(b.name.c_str());

		return brNames;
	}

	int getSize() const {
		return branches.size();
	}

	int& getIndex()  {
		return index;
	}

	void addBranch() {
		branches.push_back({ std::string("Branch") + std::to_string(branches.size()), &branches[index] });
		index = branches.size() - 1;
		branches[index].commands.push_back({ "Branching", ICON_FA_SHARE_ALT });
		currentCommand = 0;
	}

	bool canMerge() const {
		return branches[index].parent;
	}

	std::vector<Command>& getCommands() {
		return branches[index].commands;
	}

	bool isCurretCommand(const Command& c) const {
		return &branches[index].commands[currentCommand] == &c;
	}

	void setCurrentCommand(const Command& c) {
		currentCommand = &c - branches[index].commands.data();
	}
};