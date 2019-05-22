#include "pch.h"
#include "EditableMesh.h"
#include "../DX12/DX12Mesh.h"
#include "../DX12/DX12Renderer.h"
#include "../DX12/DX12VertexBuffer.h"
#include "../DX12/DX12IndexBuffer.h"
#include "../../assets/shaders/CommonRT.hlsl"
#include "../GitClone/Branch.h"

#include <chrono>

// Currently creates a width by height large mesh with specified number of vertices
EditableMesh::EditableMesh(DX12Renderer* renderer, float width, float height, size_t numVertsX, size_t numVertsY) 
	: m_width(width)
	, m_height(height)
{

	assert(numVertsX > 2 && numVertsY > 2 && width > 0.f && height > 0.f);

	m_renderer = renderer;

	int numVertices = numVertsX * numVertsY;
	int numIndices = (numVertsX - 1) * (numVertsY - 1) * 6;
	m_mesh = std::unique_ptr<DX12Mesh>(static_cast<DX12Mesh*>(renderer->makeMesh()));
	m_vertexBuffer = std::unique_ptr<VertexBuffer>(renderer->makeVertexBuffer(numVertices * sizeof(Vertex), VertexBuffer::DATA_USAGE::DYNAMIC));
	m_indexBuffer = std::unique_ptr<IndexBuffer>(renderer->makeIndexBuffer(numIndices * sizeof(unsigned int), IndexBuffer::DATA_USAGE::STATIC));
	m_mesh->setIABinding(m_vertexBuffer.get(), m_indexBuffer.get(), 0, numVertices, numIndices, sizeof(Vertex));

	m_numVertsX = numVertsX;
	m_numVertsY = numVertsY;
	vertices = new Vertex[numVertices];
	indices = new unsigned int[numIndices];
	
	m_vertLengthX = width / float(numVertsX);
	m_vertLengthY = height / float(numVertsY);

	for (size_t y = 0; y < numVertsY; y++) {
		for (size_t x = 0; x < numVertsX; x++) {
			vertices[y * numVertsX + x] = { 	
				XMFLOAT3{x * m_vertLengthX, 0, y * m_vertLengthY}, // position
				XMFLOAT3{0.f, 1.f, 0.f},  // normal
				XMFLOAT2{float(x) / float(numVertsX - 1), float(y) / float(numVertsY - 1)} // uv
			};

			// Add square indices (2 triangles)
			if (x < numVertsX - 1 && y < numVertsY - 1) {
				unsigned int leftBottomIndex = (y * (numVertsX - 1) + x) * 6;
				unsigned int leftBottomVertIndex = (y * numVertsX + x);
				/*
					 Indices
				    2 _ _ _ 3
					 |	   |
					 |     |
					0|_ _ _|1
				*/
				// Bottom left triangle
				indices[leftBottomIndex + 0] = leftBottomVertIndex; // 0
				indices[leftBottomIndex + 1] = leftBottomVertIndex + numVertsX; // 2
				indices[leftBottomIndex + 2] = leftBottomVertIndex + 1; // 1

				// Top right triangle
				indices[leftBottomIndex + 3] = leftBottomVertIndex + 1; // 1
				indices[leftBottomIndex + 4] = leftBottomVertIndex + numVertsX; // 2
				indices[leftBottomIndex + 5] = leftBottomVertIndex + numVertsX + 1; // 3
			}
		}
	}

	m_vertexBuffer->setData(vertices, numVertices * sizeof(Vertex), 0);
	m_indexBuffer->setData(indices, numIndices * sizeof(unsigned int), 0);
	m_numIndices = numIndices;
}

EditableMesh::EditableMesh(const EditableMesh & other) {

	m_renderer = other.m_renderer;

	int numVertices = other.m_numVertsX * other.m_numVertsY;
	int numIndices = (other.m_numVertsX - 1) * (other.m_numVertsY - 1) * 6;
	m_mesh = std::unique_ptr<DX12Mesh>(static_cast<DX12Mesh*>(m_renderer->makeMesh()));
	m_vertexBuffer = std::unique_ptr<VertexBuffer>(m_renderer->makeVertexBuffer(numVertices * sizeof(Vertex), VertexBuffer::DATA_USAGE::DYNAMIC));
	m_indexBuffer = std::unique_ptr<IndexBuffer>(m_renderer->makeIndexBuffer(numIndices * sizeof(unsigned int), IndexBuffer::DATA_USAGE::STATIC));
	m_mesh->setIABinding(m_vertexBuffer.get(), m_indexBuffer.get(), 0, numVertices, numIndices, sizeof(Vertex));

	m_numVertsX = other.m_numVertsX;
	m_numVertsY = other.m_numVertsY;
	vertices = new Vertex[numVertices];
	indices = new unsigned int[numIndices];

	std::memcpy(vertices, other.vertices, numVertices * sizeof(Vertex));
	std::memcpy(indices, other.indices, numIndices * sizeof(unsigned int));

	m_vertLengthX = other.m_vertLengthX;
	m_vertLengthY = other.m_vertLengthY;

	m_numIndices = numIndices;
}


EditableMesh::~EditableMesh() {
	delete vertices;
	delete indices;
}

EditableMesh EditableMesh::operator=(const EditableMesh & other) {

	if (other.m_numVertsX != m_numVertsX || other.m_numVertsY != m_numVertsY)
		throw("EditableMesh can only be swapped out with another EditableMesh of the same dimensions");

	int numVertices = m_numVertsX * m_numVertsY;
	std::memcpy(vertices, other.vertices, numVertices * sizeof(Vertex));

	((DX12VertexBuffer*)m_vertexBuffer.get())->updateData(vertices, numVertices * sizeof(Vertex));

	return *this;
}

DX12Mesh * EditableMesh::getMesh() {
	return m_mesh.get();
}

VertexBuffer * EditableMesh::getVertexBuffer() {
	return m_vertexBuffer.get();
}

IndexBuffer * EditableMesh::getIndexBuffer() {
	return m_indexBuffer.get();
}

float EditableMesh::getWidth() {
	return m_width;
}

float EditableMesh::getHeight() {
	return m_height;
}

void EditableMesh::doCommand(const XMVECTOR& rayOrigin, const XMVECTOR& rayDir, const VertexCommand& cmd, Area area) {
	int maxIntDistX = int(cmd.radius / m_vertLengthX);
	int maxIntDistY = int(cmd.radius / m_vertLengthY);
	bool rayHit = false;
	float piHalf = 1.57079632679f;
	std::vector<std::pair<unsigned int, float>> vertIndicesAndDst;
	for (size_t y = 0; y < m_numVertsY - 1; y++) {
		for (size_t x = 0; x < m_numVertsX - 1; x++) {
			unsigned int leftBottomIndex = (y * (m_numVertsX - 1) + x) * 6;
			// Bottom left triangle
			XMFLOAT3 p0F = vertices[indices[leftBottomIndex + 0]].position;
			XMFLOAT3 p1F = vertices[indices[leftBottomIndex + 1]].position;
			XMFLOAT3 p2F = vertices[indices[leftBottomIndex + 2]].position;
			XMVECTOR p0 = DirectX::XMLoadFloat3(&p0F);
			XMVECTOR p1 = DirectX::XMLoadFloat3(&p1F);
			XMVECTOR p2 = DirectX::XMLoadFloat3(&p2F);
			XMFLOAT3 intersectionPoint;
			// Ray hit
			if (rayTriangleIntersect(rayOrigin, rayDir, p0, p1, p2, intersectionPoint))
				rayHit = true;

			// Top right triangle
			p0F = vertices[indices[leftBottomIndex + 3]].position;
			p1F = vertices[indices[leftBottomIndex + 4]].position;
			p2F = vertices[indices[leftBottomIndex + 5]].position;
			p0 = DirectX::XMLoadFloat3(&p0F);
			p1 = DirectX::XMLoadFloat3(&p1F);
			p2 = DirectX::XMLoadFloat3(&p2F);
			// Ray hit (skip if previous hit)
			if (!rayHit && rayTriangleIntersect(rayOrigin, rayDir, p0, p1, p2, intersectionPoint))
				rayHit = true;

			if (rayHit) {
				int middleX = int(round(intersectionPoint.x / m_vertLengthX));
				// Y on the mesh in world coordinates is Z
				int middleY = int(round(intersectionPoint.z / m_vertLengthY));
				for (size_t y_2 = max(0, middleY - maxIntDistY); y_2 < min(m_numVertsY, middleY + maxIntDistY); y_2++) {
					for (size_t x_2 = max(0, middleX - maxIntDistX); x_2 < min(m_numVertsX, middleX + maxIntDistX); x_2++) {
						auto& p = vertices[y_2 * m_numVertsX + x_2].position;
						if (p.x > area.maxX || p.x < area.minX || p.z > area.maxZ || p.z < area.minZ)
							continue;

						float a = std::powf(float(y_2) * m_vertLengthY - float(middleY) * m_vertLengthY, 2.f);
						float b = std::powf(float(x_2) * m_vertLengthX - float(middleX) * m_vertLengthX, 2.f);
						float dist = std::sqrtf(a + b);
						if (dist <= cmd.radius) {
							vertIndicesAndDst.push_back(std::pair(y_2 * m_numVertsX + x_2, (cmd.radius - dist) / cmd.radius));
						}
					}
				}
				break;
			}
		}
		if (rayHit)
			break;
	}

	if (rayHit) {
		cmd.func(vertices, vertIndicesAndDst);
		((DX12VertexBuffer*)m_vertexBuffer.get())->updateData(vertices, m_numVertsX * m_numVertsY * sizeof(Vertex));
	}
}

void EditableMesh::doChanges(const std::vector<std::pair<unsigned int, XMFLOAT3>>& delta) {
	for (const std::pair<unsigned int, XMFLOAT3>& change : delta) {
		vertices[change.first].position.x += change.second.x;
		vertices[change.first].position.y += change.second.y;
		vertices[change.first].position.z += change.second.z;
	}
	((DX12VertexBuffer*)m_vertexBuffer.get())->updateData(vertices, m_numVertsX* m_numVertsY * sizeof(Vertex));
}

void EditableMesh::updateSubArea(EditableMesh* e, Area area) {
	for (size_t y = 0; y < m_numVertsY - 1; y++) {
		for (size_t x = 0; x < m_numVertsX - 1; x++) {
			auto& p = vertices[y * m_numVertsX + x].position;
			if (p.x > area.maxX || p.x < area.minX || p.z > area.maxZ || p.z < area.minZ)
				continue;

			vertices[y * m_numVertsX + x] = e->vertices[y * m_numVertsX + x];
		}
	}
	((DX12VertexBuffer*)m_vertexBuffer.get())->setData(vertices, m_numVertsX* m_numVertsY * sizeof(Vertex), 0);
	((DX12VertexBuffer*)m_vertexBuffer.get())->updateData(vertices, m_numVertsX* m_numVertsY * sizeof(Vertex));
}

// TODO: Move to utility functions
bool EditableMesh::rayTriangleIntersect(XMVECTOR rayOrigin, XMVECTOR rayDir, XMVECTOR p0, XMVECTOR p1, XMVECTOR p2, XMFLOAT3& outIntersectionPoint) {
	XMVECTOR intersectionPoint;

	const float EPSILON = 0.0000001;
	XMVECTOR edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = p1 - p0;
	edge2 = p2 - p0;
	h = DirectX::XMVector3Cross(rayDir, edge2);
	XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, DirectX::XMVector3Dot(edge1, h));
	a = result.x;
	if (a > -EPSILON && a < EPSILON)
		return false;    // This ray is parallel to this triangle.
	f = 1.0 / a;
	s = rayOrigin - p0;
	DirectX::XMStoreFloat3(&result, DirectX::XMVector3Dot(s, h));
	u = f * (result.x);
	if (u < 0.0 || u > 1.0)
		return false;
	q = DirectX::XMVector3Cross(s, edge1);
	DirectX::XMStoreFloat3(&result, DirectX::XMVector3Dot(rayDir, q));
	v = f * result.x;
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	DirectX::XMStoreFloat3(&result, DirectX::XMVector3Dot(edge2, q));
	float t = f * result.x;
	if (t > EPSILON) { // ray intersection
		DirectX::XMStoreFloat3(&outIntersectionPoint, rayOrigin + rayDir * t);
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}

void EditableMesh::setVertexData(Vertex* _vertices) {
	size_t dataSize = m_numVertsX * m_numVertsY * sizeof(Vertex);
	std::memcpy(vertices, _vertices, dataSize);
	((DX12VertexBuffer*)m_vertexBuffer.get())->updateData(vertices, dataSize);
}

void EditableMesh::updateData() {
	m_renderer->executeNextOpenPreCommand([&]() {
		/*((DX12VertexBuffer*)m_vertexBuffer.get())->updateData(vertices, numVertices * sizeof(Vertex));
		((DX12IndexBuffer*)m_indexBuffer.get())->updateData(indices, numIndices * sizeof(unsigned int));*/
		m_vertexBuffer->setData(vertices, m_numVertsX * m_numVertsY * sizeof(Vertex), 0);
		m_indexBuffer->setData(indices, m_numIndices * sizeof(unsigned int), 0);
	});
}

Vertex * EditableMesh::getVertices() {
	return vertices;
}
