#include "pch.h"
#include "Game.h"
#include "DX12/DX12Renderer.h"
#include "DX12/DX12Material.h"
#include "DX12/DX12Mesh.h"
#include "DX12/DX12VertexBuffer.h"
#include "Utils/Input.h"

Game::Game() 
	: Application(1700, 900, "Loading..")
{
	m_dxRenderer = static_cast<DX12Renderer*>(&getRenderer());
	getRenderer().setClearColor(0.2f, 0.4f, 0.2f, 1.0f);
	m_dxRenderer->createRenderToTextureResources();

	m_persCamera = std::make_unique<Camera>(m_dxRenderer->getWindow()->getWindowWidth() / (float)m_dxRenderer->getWindow()->getWindowHeight(), 110.f, 0.1f, 1000.f);
	m_persCamera->setPosition(XMVectorSet(7.37f, 12.44f, 13.5f, 0.f));
	m_persCamera->setDirection(XMVectorSet(0.17f, -0.2f, -0.96f, 1.0f));
	m_persCameraController = std::make_unique<CameraController>(m_persCamera.get(), m_persCamera->getDirectionVec());


}

Game::~Game() {
	for (int i = 0; i < m_models.size(); i++) {
		if (m_models[i])
			delete m_models[i];
	}
}

void Game::init() {
	m_fbxImporter = std::make_unique<PotatoFBXImporter>();
	std::cout << "Loading fbx models.." << std::endl;
	
	float floorHalfWidth = 50.0f;
	float floorTiling = 5.0f;
	const Vertex floorVertices[] = {
			{XMFLOAT3(-floorHalfWidth, 0.f, -floorHalfWidth), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(0.0f, floorTiling)},	// position, normal and UV
			{XMFLOAT3(-floorHalfWidth, 0.f,  floorHalfWidth), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(0.0f, 0.0f)},
			{XMFLOAT3(floorHalfWidth,  0.f,  floorHalfWidth), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(floorTiling, 0.0f)},
			{XMFLOAT3(floorHalfWidth, 0.f,  -floorHalfWidth), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(floorTiling, floorTiling)},
	};
	const unsigned int floorIndices[] {
		0, 1, 2, 0, 2, 3
	};

	// load Materials.
	std::string shaderPath = getRenderer().getShaderPath();
	std::string shaderExtension = getRenderer().getShaderExtension();
	float diffuse[4] = {
		1.0, 1.0, 1.0, 1.0
	};

	m_material = std::unique_ptr<Material>(getRenderer().makeMaterial("material_0"));
	m_material->setShader(shaderPath + "VertexShader" + shaderExtension, Material::ShaderType::VS);
	m_material->setShader(shaderPath + "FragmentShader" + shaderExtension, Material::ShaderType::PS);
	DX12Material* dxMaterial = ((DX12Material*)m_material.get());
	std::string err;
	dxMaterial->compileMaterial(err);

	// add a constant buffer to the material, to tint every triangle using this material
	m_material->addConstantBuffer("DiffuseTint", CB_REG_DIFFUSE_TINT, 4 * sizeof(float));
	// no need to update anymore
	// when material is bound, this buffer should be also bound for access.
	m_material->updateConstantBuffer(diffuse, CB_REG_DIFFUSE_TINT);

	// basic technique
	m_technique = std::unique_ptr<Technique>(getRenderer().makeTechnique(m_material.get(), getRenderer().makeRenderState()));
	
	// create textures
	DX12Texture2DArray* floorTexture = new DX12Texture2DArray(static_cast<DX12Renderer*>(&getRenderer()));
	m_floorTexArray = std::unique_ptr<DX12Texture2DArray>(floorTexture);
	std::vector<std::string> texFiles;
	texFiles.emplace_back("../assets/textures/floortilediffuse.png");
	m_floorTexArray->loadFromFiles(texFiles);
	
	unsigned int offset = 0;
	{
		// Set up floor mesh
		m_meshes.emplace_back(static_cast<DX12Mesh*>(getRenderer().makeMesh()));
		m_meshes.back()->setName("Floor");
		constexpr auto numVertices = std::extent<decltype(floorVertices)>::value;
		constexpr auto numIndices = std::extent<decltype(floorIndices)>::value;
		m_vertexBuffers.emplace_back(getRenderer().makeVertexBuffer(sizeof(floorVertices), VertexBuffer::DATA_USAGE::STATIC));
		m_vertexBuffers.back()->setData(floorVertices, sizeof(floorVertices), offset);
		m_indexBuffers.emplace_back(getRenderer().makeIndexBuffer(sizeof(floorIndices), IndexBuffer::STATIC));
		m_indexBuffers.back()->setData(floorIndices, sizeof(floorIndices), offset);
		m_meshes.back()->setIABinding(m_vertexBuffers.back().get(), m_indexBuffers.back().get(), offset, numVertices, numIndices, sizeof(Vertex));
		m_meshes.back()->technique = m_technique.get();
		m_meshes.back()->setTexture2DArray(m_floorTexArray.get());
	}

	if (m_dxRenderer->isDXREnabled()) {
		// Update raytracing acceleration structures
		m_dxRenderer->getDXR().setMeshes(m_meshes);
		m_dxRenderer->getDXR().useCamera(m_persCamera.get());
	}

	static_cast<DX12Renderer*>(&getRenderer())->useCamera(m_persCamera.get());

	m_persCamera->updateConstantBuffer();

}

void Game::update(double dt) {

	if (Input::IsMouseButtonPressed(Input::MouseButton::RIGHT)) {
		Input::showCursor(Input::IsCursorHidden());
	}
	// Lock mouseaaw
	if (Input::IsCursorHidden()) {
		POINT p;
		p.x = reinterpret_cast<DX12Renderer*>(&getRenderer())->getWindow()->getWindowWidth() / 2;
		p.y = reinterpret_cast<DX12Renderer*>(&getRenderer())->getWindow()->getWindowHeight() / 2;
		ClientToScreen(*reinterpret_cast<DX12Renderer*>(&getRenderer())->getWindow()->getHwnd(), &p);
		SetCursorPos(p.x, p.y);
	}

	// Camera movement
	m_persCameraController->update(float(dt));
	m_persCamera->updateConstantBuffer();

	// Update camera constant buffer for rasterisation
	for (auto& mesh : m_meshes)
		mesh->updateCameraCB((ConstantBuffer*)(m_persCamera->getConstantBuffer())); // Update camera constant buffer for rasterisation

	static bool renderToTexture = m_dxRenderer->isRenderingToTexture();
	if (Input::IsKeyPressed('T')) {
		renderToTexture = !renderToTexture;
		m_dxRenderer->renderToTexture(renderToTexture);
	}
	if (Input::IsKeyPressed('P')) {
		m_dxRenderer->executeNextPreFrameCommand([&]() {
			m_dxRenderer->resizeRenderTexture(300, 300);
		});
	}
	if (Input::IsKeyPressed('O')) {
		m_dxRenderer->executeNextPreFrameCommand([&]() {
			m_dxRenderer->resizeRenderTexture(100, 100);
		});
	}

}

void Game::fixedUpdate(double dt) {
	// Runs at 60 ticks/sec regardless of fps
}


void Game::render(double dt) {
	// Submit rasterization meshes
	for (auto& mesh : m_meshes)
		getRenderer().submit(mesh.get());

	// Render frame with gui
	std::function<void()> imgui = std::bind(&Game::imguiFunc, this);
	m_dxRenderer->frame(imgui);

	getRenderer().present();

}

void Game::imguiFunc() {
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Docking"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows, 
			// which we can't undo at the moment without finer window depth/z control.
			//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

			if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
			if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
			if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
			if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0))     dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
			if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
			ImGui::Separator();
			/*if (ImGui::MenuItem("Close DockSpace", NULL, false, p_open != NULL))
				*p_open = false;*/
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();

	ImGui::ShowDemoWindow();

	// Scene window
	static ImVec2 size = ImVec2(400, 400);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Scene");
	ImVec2 lastSize = size;
	size.x = ImGui::GetWindowSize().x;
	size.y = ImGui::GetWindowSize().y - 20;
	if (m_dxRenderer->isRenderingToTexture())
		ImGui::Image((ImTextureID)m_dxRenderer->getRenderedTextureGPUHandle().ptr, size);
	// Resize render output to window size
	if (lastSize.x != size.x || lastSize.y != size.y) {
		m_persCamera->setAspectRatio(size.x / size.y);
		m_dxRenderer->executeNextPreFrameCommand([&]() {
			m_dxRenderer->resizeRenderTexture(size.x, size.y);
		});
	}
	ImGui::End();
	ImGui::PopStyleVar();
}
