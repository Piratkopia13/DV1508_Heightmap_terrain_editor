#pragma once

#include "Application.h"
#include "Branch.h"

#include "Core/Texture2D.h"
#include "DX12/DX12Texture2DArray.h"
#include "Core/Mesh.h"
#include "Core/Camera.h"
#include "Core/CameraController.h"
#include "potatoFBXImporter/PotatoFBXImporter.h"
#include "GameObject.h"
#include "TimerSaver.h"

class DX12Renderer;
class DX12Mesh;
class EditableMesh;

class Game : public Application {
public:
	Game();
	~Game();

	virtual void init() override;
	// Updates every frame
	virtual void update(double dt) override;
	// Tries to update 60 times per second
	virtual void fixedUpdate(double dt) override;
	// Renders every frame
	virtual void render(double dt) override;

private:
	void imguiInit();
	void imguiFunc();
	void imguiTopBar();
	void imguiTopBarWindows();
	void imguiSettingsWindow();
	void imguiTimeline();
	void imguiGraph();
	void imguiTools();
	void imguiToolOptions();

private:
	bool m_showingNewFile;
	bool m_showingOpenFile;
	bool m_showingSaveFileAs;
	bool m_showingSettings;

	bool m_showingTimeline;
	bool m_showingTimelineGraph;
	bool m_showingToolbar;
	bool m_showingToolOptions;
	bool m_showingBranches;
	bool m_showingBranchHistory;



private:
	float m_toolWidth;
	float m_toolStrength;

	int m_historyWarning;
	bool m_toolHelpText;

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
	Tool* m_currentTool;
	
	std::vector<Tool> m_tools;



private:
	DX12Renderer* m_dxRenderer;

	std::unique_ptr<Camera> m_persCamera;
	std::unique_ptr<CameraController> m_persCameraController;

	std::unique_ptr<Camera> m_aboveCamera;
	std::unique_ptr<StaticCameraController> m_aboveCameraController;

	std::unique_ptr<Technique> m_technique;
	std::unique_ptr<DX12Texture2DArray> m_floorTexArray;
	std::unique_ptr<Sampler2D> m_sampler;
	std::unique_ptr<Material> m_material;

	std::vector<std::unique_ptr<VertexBuffer>> m_vertexBuffers;
	std::vector<std::unique_ptr<IndexBuffer>> m_indexBuffers;
	std::vector<std::unique_ptr<DX12Mesh>> m_meshes;

	std::unique_ptr<EditableMesh> m_editableMesh;

	std::unique_ptr<PotatoFBXImporter> m_fbxImporter;
	std::vector<PotatoModel*> m_models;

	bool m_cursorInScene;

	bool m_branching = false;
	ImVec2 m_points[2];
	BranchManager bm;
};