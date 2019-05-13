#include "pch.h"
#include "Game.h"
#include "DX12/DX12Renderer.h"
#include "DX12/DX12Material.h"
#include "DX12/DX12Mesh.h"
#include "DX12/DX12VertexBuffer.h"
#include "Utils/Input.h"
#include "Geometry/EditableMesh.h"

#include "IconsFontAwesome5.h"
#include "ImGui/imgui_internal.h"

#include "ImGui/imgui_internal.h"

Game::Game()
	: Application(1700, 900, "Loading..")
{
	m_cursorInScene = false;

	m_dxRenderer = static_cast<DX12Renderer*>(&getRenderer());
	getRenderer().setClearColor(0.2f, 0.4f, 0.2f, 1.0f);
	m_dxRenderer->createRenderToTextureResources();

	m_persCamera = std::make_unique<Camera>(m_dxRenderer->getWindow()->getWindowWidth() / (float)m_dxRenderer->getWindow()->getWindowHeight(), 110.f, 0.1f, 1000.f);
	m_persCamera->setPosition(XMVectorSet(7.37f, 12.44f, 13.5f, 0.f));
	m_persCamera->setDirection(XMVectorSet(0.17f, -0.2f, -0.96f, 1.0f));
	m_persCameraController = std::make_unique<CameraController>(m_persCamera.get(), m_persCamera->getDirectionVec());

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges);
	// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
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
	imguiInit();

	m_toolWidth = 4.0f;
	m_toolStrength = 1.0f;

	float floorHalfWidth = 50.0f;
	float floorTiling = 5.0f;
	const Vertex floorVertices[] = {
			{XMFLOAT3(-floorHalfWidth, 0.f, -floorHalfWidth), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(0.0f, floorTiling)},	// position, normal and UV
			{XMFLOAT3(-floorHalfWidth, 0.f,  floorHalfWidth), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(0.0f, 0.0f)},
			{XMFLOAT3(floorHalfWidth,  0.f,  floorHalfWidth), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(floorTiling, 0.0f)},
			{XMFLOAT3(floorHalfWidth, 0.f,  -floorHalfWidth), XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT2(floorTiling, floorTiling)},
	};
	const unsigned int floorIndices[]{
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
		m_editableMesh = std::unique_ptr<EditableMesh>(new EditableMesh(m_dxRenderer, 200.f, 200.f, 100, 100));
		m_editableMesh->getMesh()->technique = m_technique.get();
		m_editableMesh->getMesh()->setTexture2DArray(m_floorTexArray.get());
		m_meshes.emplace_back(m_editableMesh->getMesh());
		m_vertexBuffers.emplace_back(m_editableMesh->getVertexBuffer());
		m_indexBuffers.emplace_back(m_editableMesh->getIndexBuffer());
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

	Input::SetInputAllowed((m_cursorInScene || Input::IsCursorHidden()));

	keybinds();
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

	if (Input::IsMouseButtonPressed(Input::MouseButton::LEFT)) {
		DirectX::XMVECTOR rayOrigin = DirectX::XMLoadFloat3(&m_persCamera->getPositionF3());
		DirectX::XMVECTOR rayDir = m_persCamera->getDirectionVec();

		/* 
		*
		*		EXAMPLE OF HOW TO USE COMMANDS 
		*
		*/


		EditableMesh::VertexCommand cmd1 = { m_toolWidth, m_currentTool->func };

		m_dxRenderer->executeNextOpenCopyCommand([&, rayOrigin, rayDir, cmd1] {
			m_editableMesh->doCommand(rayOrigin, rayDir, cmd1);
		});
		
		/* 
		*
		*		END OF EXAMPLE OF HOW TO USE COMMANDS 
		*
		*/
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

void Game::keybinds() {


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

	if (Input::IsKeyPressed('Z')) {
		std::vector<std::pair<unsigned int, XMFLOAT3>> changes = m_bm.undo();
		if(changes.size() > 0)
			m_dxRenderer->executeNextOpenCopyCommand([&, changes] {
				m_editableMesh->doChanges(changes);
				});
	}
	if (Input::IsKeyPressed('Y')) {
		std::vector<std::pair<unsigned int, XMFLOAT3>> changes = m_bm.redo();
		if (changes.size() > 0)
			m_dxRenderer->executeNextOpenCopyCommand([&, changes] {
			m_editableMesh->doChanges(changes);
				});
	}

#pragma region TOOLS



	if (Input::IsKeyDown('1')) {
		m_currentTool = &m_tools[0];
	}
	if (Input::IsKeyDown('2')) {
		m_currentTool = &m_tools[1];

	}
#pragma endregion
}

void Game::imguiInit() {
	
	m_showingNewFile = false;
	m_showingOpenFile = false;
	m_showingSaveFileAs = false;
	m_showingSettings = false;
	
	m_showingTimeline = true;
	m_showingToolbar = true;
	m_showingToolOptions = true;
	m_showingTimelineGraph = true;

	m_toolWidth = 1;
	m_toolStrength = 1;




	m_historyWarning = 30;
	m_toolHelpText = true;
	m_tools.emplace_back(Tool::ToolInfo(ICON_FA_PAINT_BRUSH, "add/reduce height", "1", "this high and low things"), [&](Vertex * vertices, std::vector<std::pair<unsigned int, float>> vectorStuff) {
		std::vector<std::pair<unsigned int, XMFLOAT3>> positions;
		float delta = 0.0;
		for each (auto vertex in vectorStuff) {
			delta = m_toolStrength * std::sin(1.57079632679f * vertex.second);
			vertices[vertex.first].position.y += delta;
			positions.emplace_back(vertex.first, XMFLOAT3(0, delta,0));
		}

		m_bm.addCommand(m_currentTool, { m_toolStrength, m_toolWidth }, positions);

		});
	m_tools.emplace_back(Tool::ToolInfo(ICON_FA_PAINT_ROLLER, "Set Height", "2", "setting the height of things" ), [&](Vertex * vertices, std::vector<std::pair<unsigned int, float>> vectorStuff) {
		float highestImpact = 0.f;
		float height = 0.f;
		std::vector<std::pair<unsigned int, XMFLOAT3>> positions;
		for each (auto vertex in vectorStuff) {
			if (vertex.second > highestImpact) {
				height = vertices[vertex.first].position.y;
				highestImpact = vertex.second;
			}
		}
		for each (auto vertex in vectorStuff) {
			positions.emplace_back(vertex.first, XMFLOAT3(0, height - vertices[vertex.first].position.y, 0));
			vertices[vertex.first].position.y = height;
		}

		m_bm.addCommand(m_currentTool, { m_toolStrength, m_toolWidth }, positions);

		});
	
	m_currentTool = &m_tools[0];

}

void Game::imguiFunc() {
	// Style
	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;
	/*ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7, 7));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 0.f);*/

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

	imguiTopBar();
	ImGui::End();
	imguiTopBarWindows();
	if (m_showingSettings)
		imguiSettingsWindow();

	ImGui::ShowDemoWindow();

	// Scene window
	static ImVec2 size = ImVec2(400, 400);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Scene");
	m_cursorInScene = ImGui::IsMouseHoveringWindow();
	ImVec2 lastSize = size;
	size.x = ImGui::GetWindowSize().x;
	size.y = ImGui::GetWindowSize().y - 25;

	if (m_dxRenderer->isRenderingToTexture())
		ImGui::Image((ImTextureID)m_dxRenderer->getRenderedTextureGPUHandle().ptr, size);
	// Resize render output to window size
	if (lastSize.x != size.x || lastSize.y != size.y) {
		// Ignore imgui bug size
		if (size.y > 0) {
			m_persCamera->setAspectRatio(size.x / size.y);
			m_dxRenderer->executeNextPreFrameCommand([&]() {
				m_dxRenderer->resizeRenderTexture(size.x, size.y);
				});
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
	if (m_showingTimeline)
		imguiTimeline();
	if (m_showingToolbar)
		imguiTools();
	if (m_showingToolOptions)
		imguiToolOptions();
	if(m_showingTimelineGraph)
		imguiGraph();
}

void Game::imguiTopBar() {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				m_showingNewFile = true;
			}
			if (ImGui::MenuItem("Open")) {
				m_showingOpenFile = true;
			}
			ImGui::Separator();

			if (ImGui::MenuItem("Save")) {

			}
			if (ImGui::MenuItem("Save as")) {
				m_showingSaveFileAs = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Settings")) {
				m_showingSettings = true;
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Exit", "Alt+F4")) {
				PostQuitMessage(10);
			}

		ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo")) {

			}
			if (ImGui::MenuItem("Redo")) {

			}

		ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("History Bar"), "", &m_showingTimeline) {
			}
			if (ImGui::MenuItem("Graph"), "", &m_showingTimelineGraph) {
			}
			if (ImGui::MenuItem("Tools"), &m_showingToolbar) {
			}
			if (ImGui::MenuItem("Tool Settings"), "", &m_showingToolOptions) {
			}

		ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Branch")) {
			if (ImGui::MenuItem("New Local Branch")) {

			}
			if (ImGui::MenuItem("Merge Into")) {

			}
			if (ImGui::MenuItem("Reset")) {

			}
			if (ImGui::MenuItem("Rename")) {

			}
			ImGui::Separator();
			if (ImGui::MenuItem("Fetch")) {

			}
			if (ImGui::MenuItem("Pull")) {

			}
			if (ImGui::MenuItem("Push")) {

			}

		ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("View Help")) {

			}
			ImGui::Separator();
			if (ImGui::MenuItem("Send feedback")) {

			}
			if (ImGui::MenuItem("Check for updates")) {

			}
			ImGui::Separator();
			if (ImGui::MenuItem("About")) {

			}

		ImGui::EndMenu();
		}

		//if (ImGui::BeginMenu("Docking"))
		//{
		//	// Disabling fullscreen would allow the window to be moved to the front of other windows, 
		//	// which we can't undo at the moment without finer window depth/z control.
		//	//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

		//	if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
		//	if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
		//	if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
		//	if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0))     dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
		//	if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
		//	ImGui::Separator();
		//	/*if (ImGui::MenuItem("Close DockSpace", NULL, false, p_open != NULL))
		//		*p_open = false;*/
		//	ImGui::EndMenu();
		//}

	}
	ImGui::EndMenuBar();
}

void Game::imguiTopBarWindows() {
	if (m_showingNewFile) {
		ImGui::SetNextWindowSize(ImVec2(550, 550));
		if (ImGui::Begin("Create new File thing", &m_showingNewFile)) {
			ImGui::Text("planetSize thing");
			static float createNewPlanetSize = 1;
			ImGui::InputFloat("##value", &createNewPlanetSize, 1.0f);
			if (createNewPlanetSize < 10)
				createNewPlanetSize = 10;
			ImGui::Text("PATH: if time exists, include native window folder selection window thing");
			ImGui::End();
		}

	}
	if (m_showingOpenFile) {
		ImGui::SetNextWindowSize(ImVec2(550, 550));
		if (ImGui::Begin("Open File thing", &m_showingOpenFile)) {

			ImGui::Text("PATH: if time exists, include native window folder selection window thing");
		}
		ImGui::End();

	}
	if (m_showingSaveFileAs) {
		ImGui::SetNextWindowSize(ImVec2(550, 550));
		if (ImGui::Begin("Save File thing as", &m_showingSaveFileAs)) {

			ImGui::Text("PATH: if time exists, include native window folder selection window thing");
		}
		ImGui::End();

	}

}

void Game::imguiSettingsWindow() {

	if (m_showingSettings) {
		if (ImGui::Begin("SETTINGS OF THINGS", &m_showingSettings, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Max unsaved commands before warning");
			ImGui::InputInt("##value", &m_historyWarning, 1.0f);

			//ImGui::Text("Max unsaved commands before warning"); 
			ImGui::Checkbox("Enable tool help", &m_toolHelpText);


			//ImGui::Text("PATH: if time exists, include native window folder selection window thing");
		}
		ImGui::End();

	}
}

void Game::imguiTimeline() {
	if (!ImGui::Begin("Timeline")) {
		ImGui::End();
		return;
	}
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 10.f));

	ImGui::Text("Timeline things", &m_showingTimeline);
	/*std::string text = "Things here " ICON_FA_PLUS;
	auto width = ImGui::CalcTextSize(text.c_str());
	ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvailWidth() - width.x, ImGui::GetCursorPosY()));
	ImGui::Text(text.c_str());*/

	const char* popupOptions[] = {
		"Add tag",
		"Compare with current"
	};


	ImGui::SetNextItemWidth(100.0f);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Branch");
	ImGui::SameLine();

	int index = m_bm.getIndex();
	ImGui::Combo("##hidelabel", &index, m_bm.getBranchNames().data(), m_bm.getSize());
	m_bm.setBranch(index);

	ImGui::SameLine();
	ImGui::BeginGroup();
	static bool br = false;
	if (br) {
		static char str0[128] = "";
		ImGui::PushItemWidth(200);
		ImGui::InputTextWithHint("", "Branch Name", str0, IM_ARRAYSIZE(str0));
		ImGui::PopItemWidth();
		if (ImGui::Button("Ok")) {
			m_bm.createBranch(str0, nullptr);
			br = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			br = false;
		}
	}
	else {
		if (ImGui::Button("Branch", { 60,30 })) {
			//ImGui::OpenPopup("Branch Window");
			br = true;
		}

		// Make Merge button faded if it isn't possible to merge
		if (!m_bm.canMerge())
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("Merge", { 60,30 })) {
			std::cout << "Merging...\n";
		}
		if (!m_bm.canMerge())
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::EndGroup();
		// Add spacing to right align command buttons
		//ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - commands.size() * 45.f);
		ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - 10.6f * 45.f);


		// Scroll area
		ImGui::BeginChild("##ScrollingRegion", ImVec2(500, 50.f), false, ImGuiWindowFlags_HorizontalScrollbar);
		//static int size = m_bm.getCurrentBranch().getCommands().size();
		//if(size < m_bm.getCurrentBranch().getCommands().size())
		//	ImGui::SetScrollX(ImGui::GetScrollMaxX()+100);
		//size = m_bm.getCurrentBranch().getCommands().size();

		if (ImGui::IsWindowHovered() || ImGui::IsWindowFocused())
			ImGui::SetScrollX(ImGui::GetScrollX() + 20.0f * -ImGui::GetIO().MouseWheel); // Horizontal scroll from vertical wheel input

		// Draw buttons
		bool first = true;
		for (int i = m_bm.getCurrentBranch().getCommands().size() - 1; i >= 0; i--) {
			if (i != m_bm.getCurrentBranch().getCommands().size() - 1)
				ImGui::SameLine();
			bool currentCommand = m_bm.isCurrentCommand(i);
			if (currentCommand) {
				ImGui::PushStyleColor(ImGuiCol_Button, { 0.4,0.5,1.0,1.0 });
			}

			if (ImGui::Button(
				(m_bm.getCurrentBranch().getCommands()[i].toolUsed->info.icon+"##"+std::to_string(i)).c_str())) {
				if (m_bm.getCommandIndex() > i) {
					std::vector<std::pair<unsigned int, XMFLOAT3>> changes = m_bm.undoTo(i);
					if (changes.size() > 0)
						m_dxRenderer->executeNextOpenCopyCommand([&, changes] {
						m_editableMesh->doChanges(changes);
							});
				}
				else if (m_bm.getCommandIndex() < i) {
					std::vector<std::pair<unsigned int, XMFLOAT3>> changes = m_bm.redoTo(i);
					if (changes.size() > 0)
						m_dxRenderer->executeNextOpenCopyCommand([&, changes] {
						m_editableMesh->doChanges(changes);
							});
				}
				std::cout << "Revert to point" << std::endl;
			}
			if (currentCommand)
				ImGui::PopStyleColor();
			ImGui::OpenPopupOnItemClick("command_popup"); // Right click to open popup
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(("Go back to this " + m_bm.getCurrentBranch().getCommands()[i].toolUsed->info.name).c_str());
		}


		ImGui::EndChild();
		// End of scroll area


		static int selectedPopupOption = -1;
		if (ImGui::BeginPopupContextItem("command_popup")) {
			for (int i = 0; i < IM_ARRAYSIZE(popupOptions); i++)
				if (ImGui::Selectable(popupOptions[i]))
					selectedPopupOption = i;
			ImGui::EndPopup();
		}

		//ImGui::Text(ICON_FA_PAINT_BRUSH "  Paint");    // use string literal concatenation
		ImGui::PopStyleVar();
		ImGui::End();
	}
}
void Game::imguiGraph() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	if (!ImGui::Begin("Graph", &m_showingTimelineGraph, ImGuiWindowFlags_HorizontalScrollbar)) {
		ImGui::End();
		ImGui::PopStyleVar();
		return;
	}

	// enum, struct and vector should be class variables
	enum CommitType {
		COMMAND, MERGE, NEWBRANCH
	};
	struct DummyCommit {
		//Command cmd;
		std::string msg;
		std::string branch;
		CommitType type;
		std::string otherBranch;
	};
	std::vector<DummyCommit> commits;

	// Fill with dummy commits to dummy branches
	commits.push_back({ "Moved thing", "Master", COMMAND, "" });
	commits.push_back({ "Placed tree", "Feature", NEWBRANCH, "Master" });
	commits.push_back({ "Moved thing", "Master", COMMAND, "" });
	commits.push_back({ "Moved thing", "Banana", NEWBRANCH, "Master" });
	commits.push_back({ "Moved thing", "Master", COMMAND, "" });
	for (int i = 0; i < 3; i++) {
		commits.push_back({ "Placed tree", "Feature", COMMAND, "" });
	}
	commits.push_back({ "Placed tree", "Feature", MERGE, "Master" });
	commits.push_back({ "Moved thing", "Banana", COMMAND, "" });

	static ImVec2 lastGraphSize = ImVec2(40, 40);
	ImGui::BeginChild("##ScrollingRegion", lastGraphSize, false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 p = ImGui::GetCursorScreenPos();
	p.x += 20.0f;
	p.y += 20.0f;
	auto drawlist = ImGui::GetWindowDrawList();

	float distanceBetweenCommits = 40.f;
	float distanceBetweenBranches = 20.f;
	float commitRadius = 5.f;

	struct BranchDrawInfo {
		float yOffset;
		float lastCommitX;
		ImU32 color;
	};
	// Internal map rebuilt on every draw
	std::map<std::string, BranchDrawInfo> activeBranches;
	float lastOffset = -distanceBetweenBranches;
	std::hash<std::string> hasher;

	ImVec2 maxCoord = p;
	bool first = true;
	for (int i = 0; i < commits.size(); i++) {
		auto commit = commits[i];
		if (activeBranches.find(commit.branch) == activeBranches.end()) {
			// commit on branch not known before
			BranchDrawInfo info;
			srand(hasher(commit.branch));
			info.color = IM_COL32(rand() % 255 + 10, rand() % 255 + 10, rand() % 255 + 10, 255); // Random bright-ish color
			info.lastCommitX = p.x + max(i-1, 0) * distanceBetweenCommits;
			lastOffset = info.yOffset = lastOffset + distanceBetweenBranches;
			activeBranches.insert({ commit.branch, info });
		}
		BranchDrawInfo& info = activeBranches[commit.branch];

		float x = p.x + i * distanceBetweenCommits;
		float y = p.y + info.yOffset;
		float lastX = info.lastCommitX;
		//float lastX = p.x + (i - 1) * distanceBetweenCommits;
		float lastY = y;
		ImU32 circleColor = info.color;

		if (!first) {
			if (commit.type == NEWBRANCH) {
				BranchDrawInfo otherBranch = activeBranches[commit.otherBranch];
				float otherY = p.y + otherBranch.yOffset;
				// update lastY to draw diagonal
				lastY = p.y + otherBranch.yOffset;
			}
			else if (commit.type == MERGE) {
				BranchDrawInfo otherBranch = activeBranches[commit.otherBranch];
				float otherY = p.y + otherBranch.yOffset;
				y = p.y + otherBranch.yOffset;
				// Draw horizontal line on branch merged into
				drawlist->AddLine(ImVec2(otherBranch.lastCommitX, y), ImVec2(x, y), otherBranch.color, 3.0f);
				circleColor = otherBranch.color;
			}
			drawlist->AddLine(ImVec2(lastX, lastY), ImVec2(x, y), info.color, 3.0f);
			info.lastCommitX = x;
		}
		drawlist->AddCircleFilled(ImVec2(x, y), commitRadius, circleColor);
		maxCoord.x = x;
		maxCoord.y = max(maxCoord.y, y);

		first = false;
	}
	//ImGui::SetWindowSize(ImVec2(maxCoord.x - p.x, maxCoord.y - p.y));
	lastGraphSize = ImVec2(maxCoord.x - p.x + 40.f, maxCoord.y - p.y + 40.f);
	ImGui::EndChild();


	//ImTriangleContainsPoint(p, p, p, p);
	ImGui::End();
	ImGui::PopStyleVar();
}

void Game::imguiTools() {
	//ImGui::SetNextWindowSizeConstraints(ImVec2(70, 100), ImVec2(70, 10000));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));

	if (ImGui::Begin("TOOLS", &m_showingToolbar, ImGuiWindowFlags_AlwaysAutoResize)) {

		for (int i = 0; i < m_tools.size(); i++) {
			if (ImGui::Button(m_tools[i].info.icon.c_str())) {
				m_currentTool = &m_tools[i];
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				std::string tooltipText = m_tools[i].info.name + " (" + m_tools[i].info.shortcut + ")";
				if (m_toolHelpText)
					tooltipText += "\n" + m_tools[i].info.helpText;
				ImGui::Text(tooltipText.c_str());
				ImGui::EndTooltip();
			}
		}







	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void Game::imguiToolOptions() {
	//ImGui::SetNextWindowSize(ImVec2(550, 550));
	if (ImGui::Begin("SETTINGS", &m_showingToolOptions, ImGuiWindowFlags_AlwaysAutoResize)) {
		//ImGui::Text(std::to_string(ImGui::GetWindowDockID()).c_str());

		//ImGui::DragFloat("##value", &m_toolWidth, 1, 1.0f, 100.0f);
		//
		//ImGui::DragFloat("##value", &m_toolWidth, 1, 1.0f, 100.0f);
		ImGui::Text("Tool");
		ImGui::SameLine();
		ImGui::SliderFloat("Width", &m_toolWidth, 1.f, 100.f);
		ImGui::SameLine();
		ImGui::SameLine();
		ImGui::SliderFloat("Strength", &m_toolStrength, 1.f, 100.f);
		//ImGui::Slider


	}
	ImGui::End();

}
