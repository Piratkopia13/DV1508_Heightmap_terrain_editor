#pragma once

class DX12Mesh;
class DX12Renderer;
class VertexBuffer;
class IndexBuffer;
struct Vertex;

class Fence
{
private:
	std::unique_ptr<DX12Mesh> m_mesh;
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;

	Vertex* vertices;
	unsigned int* indices;
public:
	Fence(DX12Renderer* renderer, XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 p3, XMFLOAT3 p4);
	~Fence();
	DX12Mesh* getMesh();
	VertexBuffer* getVertexBuffer();
	IndexBuffer* getIndexBuffer();
};

