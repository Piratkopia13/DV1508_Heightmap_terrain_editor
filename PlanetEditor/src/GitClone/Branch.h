#pragma once
#include "Command.h"
#include "IconsFontAwesome5.h"
#include "../Geometry/EditableMesh.h"

struct Area {
	float minX;
	float maxX;
	float minZ;
	float maxZ;

	std::string print() {
		return "x(" + std::to_string(minX) + ":" + std::to_string(maxX) +
			") z(" + std::to_string(minZ) + ":" + std::to_string(maxZ)+")";
	}
};

class Branch {
public:
	class Commit {
	public:
		std::string author;
		std::string message;
		std::chrono::system_clock::time_point date;
		std::shared_ptr<EditableMesh> mesh;

		Commit(const std::string& author, const std::string& message, EditableMesh* mesh) {
			this->author = author;
			this->message = message;
			this->date = std::chrono::system_clock::now();
			this->mesh = std::shared_ptr<EditableMesh>(mesh);
		}

		Commit(const Commit& other) {
			this->author = other.author;
			this->message = other.message;
			this->date = other.date;
			this->mesh = std::shared_ptr<EditableMesh>(new EditableMesh(*other.mesh));
		}

		Commit& operator=(const Commit& other) {
			this->author = other.author;
			this->message = other.message;
			this->date = other.date;
			this->mesh = std::shared_ptr<EditableMesh>(new EditableMesh(*other.mesh));

			return *this;
		}

		~Commit() {
		}
	};


	Branch();
	Branch(std::string name, Area area = {0, 200, 0, 200}, Branch* parent = nullptr);
	~Branch() = default;

	const std::string& getName() const;
	const Branch* getParent() const;
	Area getArea() const;

	void addCommand(Tool* tool, Command::Parameters params, std::vector<std::pair<unsigned int, XMFLOAT3>> newPosition);
	void addCommand(Command cmd);
	void createCommit(const std::string& author, const std::string& message, EditableMesh* mesh);
	void clearCommitsToIndex(unsigned int index);
	std::vector<Command>& getCommands();
	std::vector<Branch::Commit>& getCommits();
	void resetCommandList();

private:

	std::string m_name;
	std::vector<Command> m_commands;
	Branch* m_parent;
	Area m_area;

	std::vector<Commit> m_commits;
};

