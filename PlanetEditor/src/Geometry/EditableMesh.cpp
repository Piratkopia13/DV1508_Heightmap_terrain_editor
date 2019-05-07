#include "pch.h"
#include "EditableMesh.h"
#include "../DX12/DX12Mesh.h"
#include "../DX12/DX12Renderer.h"
#include "../DX12/DX12VertexBuffer.h"
#include "../../assets/shaders/CommonRT.hlsl"

// Currently creates a width by height large mesh with specified number of vertices
EditableMesh::EditableMesh(DX12Renderer* renderer, float width, float height, size_t numVertsX, size_t numVertsY) {

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
	
	float xVertLength = width / float(numVertsX);
	float yVertLength = height / float(numVertsY);

	for (size_t y = 0; y < numVertsY; y++) {
		for (size_t x = 0; x < numVertsX; x++) {
			vertices[y * numVertsX + x] = { 	
				XMFLOAT3{x * xVertLength, 0, y * yVertLength}, // position
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
}


EditableMesh::~EditableMesh() {
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

void EditableMesh::doCommand(XMVECTOR rayOrigin, XMVECTOR rayDir/*, Command command*/) {
	bool somethingHasChanged = false;
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
			if (rayTriangleIntersect(rayOrigin, rayDir, p0, p1, p2, intersectionPoint)) {
				//std::cout << intersectionPoint.x << ", " << intersectionPoint.y << ", " << intersectionPoint.z << std::endl;

				// Stuff happens
				p0F.y += 0.5f;
				p1F.y += 0.5f;
				p2F.y += 0.5f;
				vertices[indices[leftBottomIndex + 0]].position = p0F;
				vertices[indices[leftBottomIndex + 1]].position = p1F;
				vertices[indices[leftBottomIndex + 2]].position = p2F;
				somethingHasChanged = true;
			}

			// Top right triangle
			p0F = vertices[indices[leftBottomIndex + 3]].position;
			p1F = vertices[indices[leftBottomIndex + 4]].position;
			p2F = vertices[indices[leftBottomIndex + 5]].position;
			p0 = DirectX::XMLoadFloat3(&p0F);
			p1 = DirectX::XMLoadFloat3(&p1F);
			p2 = DirectX::XMLoadFloat3(&p2F);
			// Ray hit
			if (rayTriangleIntersect(rayOrigin, rayDir, p0, p1, p2, intersectionPoint)) {
				//std::cout << intersectionPoint.x << ", " << intersectionPoint.y << ", " << intersectionPoint.z << std::endl;
				
				// Stuff happens
				p0F.y += 0.5f;
				p1F.y += 0.5f;
				p2F.y += 0.5f;
				vertices[indices[leftBottomIndex + 3]].position = p0F;
				vertices[indices[leftBottomIndex + 4]].position = p1F;
				vertices[indices[leftBottomIndex + 5]].position = p2F;
				somethingHasChanged = true;
			}
		}
	}

	if (somethingHasChanged) {
		((DX12VertexBuffer*)m_vertexBuffer.get())->updateData(vertices, m_numVertsX * m_numVertsY * sizeof(Vertex));
		std::cout << "Hit!" << std::endl;
	}
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
