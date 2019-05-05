#pragma once

#include "Application.h"

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
	void imguiFunc();
	void imguiTimeline();
	void imguiGraph();
	
private:
	DX12Renderer* m_dxRenderer;

	std::unique_ptr<Camera> m_persCamera;
	std::unique_ptr<CameraController> m_persCameraController;

	std::unique_ptr<Technique> m_technique;
	std::unique_ptr<DX12Texture2DArray> m_floorTexArray;
	std::unique_ptr<Sampler2D> m_sampler;
	std::unique_ptr<Material> m_material;

	std::vector<std::unique_ptr<VertexBuffer>> m_vertexBuffers;
	std::vector<std::unique_ptr<IndexBuffer>> m_indexBuffers;
	std::vector<std::unique_ptr<DX12Mesh>> m_meshes;

	std::unique_ptr<PotatoFBXImporter> m_fbxImporter;
	std::vector<PotatoModel*> m_models;

	bool m_cursorInScene;

};