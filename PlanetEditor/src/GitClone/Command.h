#pragma once
#include "pch.h"
struct Tool {
	struct ToolInfo {
		std::string icon;
		std::string name;
		std::string shortcut;
		std::string helpText;
		ToolInfo() {
			icon = "N/A";
			name = "N/A";
			shortcut = "N/A";
			helpText = "N/A";
		}
		ToolInfo(std::string _icon, std::string _name, std::string _shortcut, std::string _helpText) {
			icon = _icon;
			name = _name;
			shortcut = _shortcut;
			helpText = _helpText;
		}
	} info;
	std::function<void(Vertex*, std::vector<std::pair<unsigned int, float>>)> func;
	Tool(ToolInfo _info, std::function<void(Vertex*, std::vector<std::pair<unsigned int, float>>)> _func) {
		info = _info;
		func = _func;
	}

};



struct Command {
	Tool* toolUsed;
	struct Parameters {
		float toolStrength = 0.0f;
		float toolWidth = 0.0f;
	} parameters;
	std::vector<std::pair<unsigned int, XMFLOAT3>> newPosition;
};