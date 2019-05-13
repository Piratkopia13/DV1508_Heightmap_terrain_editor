#pragma once
#include "Command.h"
#include "IconsFontAwesome5.h"
#include "../Geometry/EditableMesh.h"



class Branch {
public:
	struct Commit {
		std::string author;
		std::string message;
		std::chrono::system_clock::time_point date;
		EditableMesh* mesh;

		Commit(const std::string& author, const std::string& message, EditableMesh* mesh) {
			this->author = author;
			this->message = message;
			this->date = std::chrono::system_clock::now();
			this->mesh = mesh;
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
	std::vector<Branch::Commit>& getCommits();
	void resetCommandList();

private:

	std::string m_name;
	std::vector<Command> m_commands;
	Branch* m_parent;

	std::vector<Commit> m_commits;






};

