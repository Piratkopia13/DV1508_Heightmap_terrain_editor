#pragma once

class DX12Mesh;
class DX12Renderer;
class VertexBuffer;
class IndexBuffer;
struct Vertex;

class EditableMesh {
public:
	EditableMesh(DX12Renderer* renderer, float width, float height, size_t numVertsX, size_t numVertsY);
	~EditableMesh();

	/*struct Vertex {
		float position[3];
	};*/

	DX12Mesh* getMesh();
	VertexBuffer* getVertexBuffer();
	IndexBuffer* getIndexBuffer();

	void doCommand(XMVECTOR rayOrigin, XMVECTOR rayDir/*, Command command*/);
	
	// Returns intersection point
	bool rayTriangleIntersect(XMVECTOR rayOrigin, XMVECTOR rayDir, XMVECTOR p0, XMVECTOR p1, XMVECTOR p2, XMFLOAT3& outIntersectionPoint);

private:
	std::unique_ptr<DX12Mesh> m_mesh;
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;

	Vertex* vertices;
	unsigned int* indices;

	float m_width;
	float m_height;
	size_t m_numVertsX;
	size_t m_numVertsY;

};

