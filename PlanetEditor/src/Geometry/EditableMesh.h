#pragma once

class DX12Mesh;
class DX12Renderer;
class VertexBuffer;
class IndexBuffer;

class EditableMesh {
public:
	EditableMesh(DX12Renderer* renderer, float width, float height, int numVertsX, int numVertsY);
	~EditableMesh();

	/*struct Vertex {
		float position[3];
	};*/

	DX12Mesh* getMesh();
	VertexBuffer* getVertexBuffer();
	IndexBuffer* getIndexBuffer();

private:
	std::unique_ptr<DX12Mesh> m_mesh;
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;

	float m_width;
	float m_height;
	int numVertsX;
	int numVertsY;

};

