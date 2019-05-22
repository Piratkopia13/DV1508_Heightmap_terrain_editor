#pragma once

#include "Application.h"
#include "GitClone/BranchManager.h"

#include "Core/Texture2D.h"
#include "DX12/DX12Texture2DArray.h"
#include "Core/Mesh.h"
#include "Core/Camera.h"
#include "Core/CameraController.h"
#include "potatoFBXImporter/PotatoFBXImporter.h"
#include "GameObject.h"
#include "TimerSaver.h"
#include "Geometry/Fence.h"

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
	void keybinds();

private:
	void imguiInit();
	void imguiFunc();
	void imguiTopBar();
	void imguiTopBarWindows();
	void imguiSettingsWindow();
	void imguiTimeline();
	void imguiGraph();
	void imguiBranchHistory();
	void imguiTools();
	void imguiToolOptions();
	void imguiCommitWindow();
	void imguiMasterBranchCommandsWarning();



private:
	Area calcualteArea();

	void jumpToCommitIndex(unsigned int index);
	void jumpToBranchIndex(unsigned int index);

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

	bool m_masterBranchCommandWarning;


	Area m_sceneWindow;


private:
	float m_toolWidth;
	float m_toolStrength;

	int m_historyWarning;
	bool m_historyWarningShow;
	bool m_toolHelpText;




private:
	unsigned int m_jumpToCommitIndex;
	unsigned int m_currentCommitIndex;
	



private:
	DX12Renderer* m_dxRenderer;

	std::unique_ptr<Camera> m_persCamera;
	std::unique_ptr<CameraController> m_persCameraController;

	std::unique_ptr<StaticCamera> m_aboveCamera;
	std::unique_ptr<StaticCameraController> m_aboveCameraController;

	std::unique_ptr<Technique> m_technique;
	std::unique_ptr<DX12Texture2DArray> m_floorTexArray;
	std::unique_ptr<Sampler2D> m_sampler;
	std::unique_ptr<Material> m_material;

	std::vector<VertexBuffer*> m_vertexBuffers;
	std::vector<IndexBuffer*> m_indexBuffers;
	std::vector<DX12Mesh*> m_meshes;

	std::unique_ptr<EditableMesh> m_editableMesh;

	std::unique_ptr<Fence> m_fence;
	std::unique_ptr<Fence> m_fence2;
	std::unique_ptr<DX12Texture2DArray> m_fenceTexArray;
	std::unique_ptr<DX12Texture2DArray> m_fence2TexArray;

	std::unique_ptr<PotatoFBXImporter> m_fbxImporter;
	std::vector<PotatoModel*> m_models;

	bool m_cursorInScene;

	Tool* m_currentTool;
	std::vector<Tool> m_tools;
	BranchManager m_bm;
	bool m_branching = false;
	ImVec2 m_points[2];
	XMFLOAT3 p1;
	XMFLOAT3 p2;
	XMFLOAT3 p3;
	XMFLOAT3 p4;
};
