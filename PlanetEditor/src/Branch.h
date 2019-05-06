#pragma once

struct Command {
	std::string name;
	char* icon;
};

struct Branch {
	std::string name;
	std::vector<Command> commands;
private:
	Branch* parent = nullptr;

public:
	Branch() = default;
	Branch(std::string s) : name(s) {}
	~Branch() = default;
};