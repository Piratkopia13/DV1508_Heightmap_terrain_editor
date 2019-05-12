#pragma once
#include "Command.h"
#include "IconsFontAwesome5.h"
#include "../Geometry/EditableMesh.h"



class Branch {
public:
	struct Commit {
		std::string name;
		std::string tag;
		std::chrono::time_point<std::chrono::steady_clock> time;
		EditableMesh* mesh;

		Commit(std::string _name, EditableMesh* _mesh, std::string _tag = "N/A") {
			name = _name;
			tag = _tag;
			mesh = _mesh;
			time = std::chrono::high_resolution_clock::now();
		}
	};





	Branch();
	Branch(std::string name, Branch* parent = nullptr);
	~Branch() = default;

	const std::string& getName();
	const Branch* getParent();

	void addCommand(Tool* tool, Command::Parameters params, std::vector<std::pair<unsigned int, XMFLOAT3>> newPosition);
	void addCommand(Command cmd);
	std::vector<Command>& getCommands();
	void resetCommandList();

private:

	std::string m_name;
	std::vector<Command> m_commands;
	Branch* m_parent;

	std::vector<Commit> m_commits;






};

