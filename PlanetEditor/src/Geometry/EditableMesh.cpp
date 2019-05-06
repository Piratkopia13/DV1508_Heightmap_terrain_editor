#include "pch.h"
#include "EditableMesh.h"
#include "../DX12/DX12Mesh.h"
#include "../DX12/DX12Renderer.h"

// Currently creates a width by height large mesh with specified number of vertices
EditableMesh::EditableMesh(DX12Renderer* renderer, float width, float height, int numVertsX, int numVertsY) {

	throw(std::logic_error("Not yet fully implemented"));

	int numVertices = numVertsX * numVertsY;
	int numIndices = (numVertsX - 1) * (numVertsY - 1) * 6;
	m_mesh = std::make_unique<DX12Mesh>(static_cast<DX12Mesh*>(renderer->makeMesh()));
	m_vertexBuffer = std::make_unique<VertexBuffer>(renderer->makeVertexBuffer(numVertices, VertexBuffer::DATA_USAGE::DYNAMIC));
	m_indexBuffer = std::make_unique<IndexBuffer>(renderer->makeIndexBuffer(numIndices, IndexBuffer::DATA_USAGE::STATIC));
	m_mesh->setIABinding(m_vertexBuffer.get(), m_indexBuffer.get(), 0, numVertices, numIndices, sizeof(Vertex));
	// TODO: Set the technique
	//m_meshes.back()->technique = m_technique.get();
	// TODO: Set the texture(s)
	//m_meshes.back()->setTexture2DArray(m_floorTexArray.get());

	// TODO: Fix proper vertex
	Vertex* vertices = new Vertex[numVertices];
	int* indices = new int[numIndices];
	
	float xVertLength = width / float(numVertsX);
	float yVertLength = height / float(numVertsY);

	for (size_t y = 0; y < numVertsY; y++) {
		for (size_t x = 0; x < numVertsX; x++) {
			vertices[y * numVertsX + x] = {x * xVertLength, 0, y * yVertLength};

			// Add square indices (2 triangles)
			if (x < numVertsX - 2 && y < numVertsY - 2) {
				int leftBottomIndex = (y * numVertsY + x - 1) * 6;
				int leftBottomVertIndex = (y * numVertsX + x);
				/*
					 Indices
				    2 _ _ _ 3
					 |	   |
					 |     |
					0|_ _ _|1
				*/
				// Bottom left triangle
				indices[leftBottomIndex + 0] = leftBottomVertIndex; // 0
				indices[leftBottomIndex + 1] = leftBottomVertIndex + 1; // 1
				indices[leftBottomIndex + 2] = leftBottomVertIndex + numVertsX; // 2

				// Top right triangle
				indices[leftBottomIndex + 3] = leftBottomVertIndex + 1; // 1
				indices[leftBottomIndex + 4] = leftBottomVertIndex + numVertsX + 1; // 3
				indices[leftBottomIndex + 5] = leftBottomVertIndex + numVertsX; // 2
			}
		}
	}

	m_vertexBuffer->setData(vertices, numVertices, 0);
	m_indexBuffer->setData(indices, numIndices, 0);
}


EditableMesh::~EditableMesh() {
}
