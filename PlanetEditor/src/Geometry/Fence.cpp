#include "pch.h"
#include "Geometry/Fence.h"
#include "DX12/DX12Mesh.h"
#include "DX12/DX12Renderer.h"


Fence::Fence(DX12Renderer* renderer, float x, float z) {
	int nrVertices = 12;
	int nrIndices = 24;
	float size = 30;

	m_mesh = std::unique_ptr<DX12Mesh>(static_cast<DX12Mesh*>(renderer->makeMesh()));
	m_vertexBuffer = std::unique_ptr<VertexBuffer>(renderer->makeVertexBuffer(nrVertices * sizeof(Vertex), VertexBuffer::DATA_USAGE::DYNAMIC));
	m_indexBuffer = std::unique_ptr<IndexBuffer>(renderer->makeIndexBuffer(nrIndices * sizeof(unsigned int), IndexBuffer::DATA_USAGE::STATIC));
	m_mesh->setIABinding(m_vertexBuffer.get(), m_indexBuffer.get(), 0, nrVertices, nrIndices, sizeof(Vertex));
	vertices = new Vertex[nrVertices];
	indices = new unsigned int[nrIndices];

	vertices[0] =  { XMFLOAT3{x + -size,     0.1f,z + -size}      , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[1] =  { XMFLOAT3{x + -size + 1, 0.1f,z + -size}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[2] =  { XMFLOAT3{x + -size,     0.1f,z + -size + 1}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[3] =  { XMFLOAT3{x + size - 1,  0.1f,z + -size}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[4] =  { XMFLOAT3{x + size,      0.1f,z + -size}       , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[5] =  { XMFLOAT3{x + size,      0.1f,z + -size + 1}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[6] =  { XMFLOAT3{x + size,      0.1f,z + size - 1}    , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[7] =  { XMFLOAT3{x + size,      0.1f,z + size}        , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[8] =  { XMFLOAT3{x + size - 1,  0.1f,z + size}    , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[9] =  { XMFLOAT3{x + -size + 1, 0.1f,z + size}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[10] = { XMFLOAT3{x + -size,     0.1f,z + size}      , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[11] = { XMFLOAT3{x + -size,     0.1f,z + size - 1}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };

	//bottom
	indices[0] = 0;
	indices[1] = 5;
	indices[2] = 4;
	indices[3] = 5;
	indices[4] = 0;
	indices[5] = 2;

	//right
	indices[6] = 4;
	indices[7] = 8;
	indices[8] = 7;
	indices[9] = 8;
	indices[10] = 4;
	indices[11] = 3;

	//top
	indices[12] = 6;
	indices[13] = 10;
	indices[14] = 7;
	indices[15] = 10;
	indices[16] = 6;
	indices[17] = 11;

	//left
	indices[18] = 9;
	indices[19] = 0;
	indices[20] = 10;
	indices[21] = 0;
	indices[22] = 9;
	indices[23] = 1;

	m_vertexBuffer->setData(vertices, nrVertices * sizeof(Vertex), 0);
	m_indexBuffer->setData(indices, nrIndices * sizeof(unsigned int), 0);
}


Fence::~Fence()
{
}

DX12Mesh * Fence::getMesh()
{
	return m_mesh.get();
}

VertexBuffer * Fence::getVertexBuffer()
{
	return m_vertexBuffer.get();
}

IndexBuffer * Fence::getIndexBuffer()
{
	return m_indexBuffer.get();
}
