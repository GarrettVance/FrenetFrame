#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"


#include "EdelsbrunnerTubes.h"





namespace Frenet
{

	class Hvy3DScene
	{
	public:
		Hvy3DScene(const std::shared_ptr<DX::DeviceResources>& deviceResources);


		void CreateDeviceDependentResources();


		void CreateWindowSizeDependentResources();


		void ReleaseDeviceDependentResources();


		void Update(DX::StepTimer const& timer);



		void Render();






	private:

		std::shared_ptr<DX::DeviceResources> m_deviceResources;


        std::unique_ptr<HvyDX::EdelsbrunnerTube>        m_Edelsbrunner;


        std::unique_ptr<DirectX::Keyboard>                      e_keyboard;
        std::unique_ptr<DirectX::Mouse>                         e_mouse;




        DirectX::XMMATRIX                               e_perspectiveMatrix; 
        DirectX::XMMATRIX                               e_viewMatrix; 


		Microsoft::WRL::ComPtr<ID3D11RasterizerState>	e_CubicleRasterizer;


		// Direct3D resources for naive cube geometry.

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;

		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_CubeVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_CubeIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
	};
}


