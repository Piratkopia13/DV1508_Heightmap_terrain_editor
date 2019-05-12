#pragma once
#include "Command.h"
#include "IconsFontAwesome5.h"



class Branch {
public:
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







};

