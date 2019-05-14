#include "pch.h"
#include "Geometry/Fence.h"
#include "DX12/DX12Mesh.h"
#include "DX12/DX12Renderer.h"


Fence::Fence(DX12Renderer* renderer, XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 p3, XMFLOAT3 p4) {
	int nrVertices = 12;
	int nrIndices = 24;
	float size = 30;

	m_mesh = std::unique_ptr<DX12Mesh>(static_cast<DX12Mesh*>(renderer->makeMesh()));
	m_vertexBuffer = std::unique_ptr<VertexBuffer>(renderer->makeVertexBuffer(nrVertices * sizeof(Vertex), VertexBuffer::DATA_USAGE::DYNAMIC));
	m_indexBuffer = std::unique_ptr<IndexBuffer>(renderer->makeIndexBuffer(nrIndices * sizeof(unsigned int), IndexBuffer::DATA_USAGE::STATIC));
	m_mesh->setIABinding(m_vertexBuffer.get(), m_indexBuffer.get(), 0, nrVertices, nrIndices, sizeof(Vertex));
	vertices = new Vertex[nrVertices];
	indices = new unsigned int[nrIndices];

	//vertices[0] =  { XMFLOAT3{x + -size,     0.1f,z + -size}      , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[1] =  { XMFLOAT3{x + -size + 1, 0.1f,z + -size}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[2] =  { XMFLOAT3{x + -size,     0.1f,z + -size + 1}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[3] =  { XMFLOAT3{x + size - 1,  0.1f,z + -size}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[4] =  { XMFLOAT3{x + size,      0.1f,z + -size}       , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[5] =  { XMFLOAT3{x + size,      0.1f,z + -size + 1}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[6] =  { XMFLOAT3{x + size,      0.1f,z + size - 1}    , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[7] =  { XMFLOAT3{x + size,      0.1f,z + size}        , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[8] =  { XMFLOAT3{x + size - 1,  0.1f,z + size}    , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[9] =  { XMFLOAT3{x + -size + 1, 0.1f,z + size}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[10] = { XMFLOAT3{x + -size,     0.1f,z + size}      , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	//vertices[11] = { XMFLOAT3{x + -size,     0.1f,z + size - 1}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[0] = { p1      , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[1] = { XMFLOAT3{p1.x + 1, p1.y, p1.z}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[2] = { XMFLOAT3{p1.x, p1.y, p1.z + 1}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[3] = { XMFLOAT3{p2.x - 1, p2.y, p2.z}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[4] = { p2   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[5] = { XMFLOAT3{p2.x, p2.y, p2.z + 1}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[6] = { XMFLOAT3{p3.x, p3.y, p3.z - 1}    , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[7] = { p3        , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[8] = { XMFLOAT3{p3.x - 1, p3.y, p3.z}    , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[9] = { XMFLOAT3{p4.x - 1, p4.y, p4.z}   , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[10] = { p4      , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };
	vertices[11] = { XMFLOAT3{p4.x, p4.y, p4.z -1}  , XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT2{0,0} };

	//bottom
	indices[0] = 0;
	indices[1] = 4;
	indices[2] = 5;
	indices[3] = 5;
	indices[4] = 2;
	indices[5] = 0;

	//right
	indices[6] = 4;
	indices[7] = 8;
	indices[8] = 7;
	indices[9] = 8;
	indices[10] = 4;
	indices[11] = 3;

	//top
	indices[12] = 6;
	indices[13] = 7;
	indices[14] = 10;
	indices[15] = 10;
	indices[16] = 11;
	indices[17] = 6;

	//left
	indices[18] = 9;
	indices[19] = 10;
	indices[20] = 0;
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
