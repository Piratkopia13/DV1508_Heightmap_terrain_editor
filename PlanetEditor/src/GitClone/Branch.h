#pragma once
#include "Command.h"
#include "IconsFontAwesome5.h"
#include "../Geometry/EditableMesh.h"



class Branch {
public:
	class Commit {
	public:
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

		Commit(const Commit& other) {
			this->author = other.author;
			this->message = other.message;
			this->date = other.date;
			this->mesh = new EditableMesh(*other.mesh);
		}

		Commit& operator=(const Commit& other) {
			this->author = other.author;
			this->message = other.message;
			this->date = other.date;
			this->mesh = new EditableMesh(*other.mesh);

			return *this;
		}

		~Commit() {
			delete mesh;
		}
	};


	Branch();
	Branch(std::string name, Branch* parent = nullptr);
	~Branch() = default;

	const std::string& getName();
	const Branch* getParent();

	void addCommand(Tool* tool, Command::Parameters params, std::vector<std::pair<unsigned int, XMFLOAT3>> newPosition);
	void addCommand(Command cmd);
	void createCommit(const std::string& author, const std::string& message, EditableMesh* mesh);
	std::vector<Command>& getCommands();
	std::vector<Branch::Commit>& getCommits();
	void resetCommandList();

private:

	std::string m_name;
	std::vector<Command> m_commands;
	Branch* m_parent;

	std::vector<Commit> m_commits;
};

