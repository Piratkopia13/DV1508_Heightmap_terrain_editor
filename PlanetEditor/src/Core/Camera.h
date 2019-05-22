#pragma once

#include <DirectXMath.h>

class DX12ConstantBuffer;
class DX12Renderer;

class Camera
{
public:
	// Aspect ratio, vertical field of view, near clipping plane, far clipping plane
	Camera(const float aspectRatio, const float fov, const float nearZ, const float farZ);
	~Camera();

	void init(DX12Renderer* renderer);

	void lookAt(const DirectX::XMVECTOR& position);

	void move(const DirectX::XMVECTOR& toMove);

	void setPosition(const DirectX::XMVECTOR& position);
	void setDirection(const DirectX::XMVECTOR& direction);
	// Vertical field of view
	void setFOV(const float fov);
	void setAspectRatio(const float aspectRatio);
	void setNearZ(const float nearZ);
	void setFarZ(const float farZ);

	const float getAspectRatio() const;
	const float getFOV() const;
	const float getNearZ() const;
	const float getFarZ() const;

	const DirectX::XMFLOAT3 getPositionF3();
	const DirectX::XMFLOAT3 getDirectionF3();
	const DirectX::XMFLOAT3 getUpF3();

	const DirectX::XMVECTOR& getPositionVec();
	const DirectX::XMVECTOR& getDirectionVec();
	const DirectX::XMVECTOR& getUpVec();
	//const DirectX::XMFLOAT3 getLookAt();

	// View matrix
	const DirectX::XMMATRIX& getViewMatrix();
	// Inverse view matrix
	const DirectX::XMMATRIX& getInvViewMatrix();
	// Projection matrix
	virtual const DirectX::XMMATRIX& getProjMatrix();
	// Inverse projection matrix
	const DirectX::XMMATRIX& getInvProjMatrix();
	// View-projection matrix
	const DirectX::XMMATRIX& getVPMatrix();

	void updateConstantBuffer();
	DX12ConstantBuffer* getConstantBuffer() const;

	const DirectX::XMVECTOR screenPointToRay(DirectX::XMVECTOR& pos);


private:
	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_invViewMatrix;
	DirectX::XMMATRIX m_projMatrix;
	DirectX::XMMATRIX m_invProjMatrix;
	DirectX::XMMATRIX m_VPMatrix;

	bool m_projMatNeedsUpdate;
	bool m_viewMatNeedsUpdate;
	bool m_VPMatNeedsUpdate;
	bool m_cbNeedsUpdate;

	DirectX::XMVECTOR m_pos;
	DirectX::XMVECTOR m_dir;
	DirectX::XMVECTOR m_up;

	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;
	float m_fov;

	DX12ConstantBuffer* m_constantBuffer;

	friend class StaticCameraController;
	friend class StaticCamera;
};

class StaticCamera : public Camera {
public:
	StaticCamera(const float aspectRatio, const float width, const float nearZ, const float farZ) : Camera(aspectRatio, 100, nearZ, farZ), m_width(width) {}
	const DirectX::XMMATRIX& getProjMatrix() {
		if (m_projMatNeedsUpdate) {
			m_projMatrix = XMMatrixOrthographicLH(m_width, m_width / m_aspectRatio, 0.1, 1000);//XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearZ, m_farZ)
			m_invProjMatrix = XMMatrixInverse(nullptr, m_projMatrix);
			m_projMatNeedsUpdate = false;
		}

		return m_projMatrix;
	}
	void setWidth(float w) {
		m_width = w;
		m_projMatNeedsUpdate = true;
		m_VPMatNeedsUpdate = true;
		m_cbNeedsUpdate = true;
	}
	void addWidth(float w) {
		m_width += w;
		m_projMatNeedsUpdate = true;
		m_VPMatNeedsUpdate = true;
		m_cbNeedsUpdate = true;
	}
	float getWidth() const {
		return m_width;
	}
	float getHeight() const {
		return m_width / m_aspectRatio;
	}
private:
	float m_width;
};

