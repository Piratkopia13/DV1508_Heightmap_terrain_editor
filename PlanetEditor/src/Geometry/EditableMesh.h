#pragma once

#include <functional>

class DX12Mesh;
class DX12Renderer;
class VertexBuffer;
class IndexBuffer;
struct Vertex;

class EditableMesh {
public:
	EditableMesh(DX12Renderer* renderer, float width, float height, size_t numVertsX, size_t numVertsY);
	EditableMesh(const EditableMesh& other);
	~EditableMesh();

	EditableMesh operator=(const EditableMesh& other);

	struct VertexCommand {
		float radius;
		std::function<void(Vertex*, std::vector<std::pair<unsigned int, float>>)> func;
	};
	/*struct Vertex {
		float position[3];
	};*/

	DX12Mesh* getMesh();
	VertexBuffer* getVertexBuffer();
	IndexBuffer* getIndexBuffer();

	void doCommand(const XMVECTOR& rayOrigin, const XMVECTOR& rayDir, const VertexCommand& cmd);
	void doChanges(const std::vector<std::pair<unsigned int, XMFLOAT3>>& delta);
	// Returns intersection point
	bool rayTriangleIntersect(XMVECTOR rayOrigin, XMVECTOR rayDir, XMVECTOR p0, XMVECTOR p1, XMVECTOR p2, XMFLOAT3& outIntersectionPoint);

	void setVertexData(Vertex* vertices);
	Vertex* getVertices();


private:
	std::unique_ptr<DX12Mesh> m_mesh;
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;

	DX12Renderer* m_renderer;

	Vertex* vertices;
	unsigned int* indices;

	float m_width;
	float m_height;
	float m_vertLengthX;
	float m_vertLengthY;
	size_t m_numVertsX;
	size_t m_numVertsY;
};

