#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"




namespace HvyDX
{



    struct VHG_PoCo    //  Position & Color;
    {
        // Used to send per-vertex data to the vertex shader.

        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 color;
    };










	struct VHG_WVPStruct
	{
	    // Constant buffer used to send MVP matrices to the vertex shader.

		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};












	class EdelsbrunnerTube
	{
	public:

		EdelsbrunnerTube(const std::shared_ptr<DX::DeviceResources>& deviceResources);



		void CreateDeviceDependentResources();




		void ReleaseDeviceDependentResources();



		void Update(
            DirectX::XMMATRIX        const&           p_tubeWorldMatrix,
            DirectX::XMMATRIX        const&           p_sceneViewMatrix,
            DirectX::XMMATRIX        const&           p_sceneProjectionMatrix
        );





		void Render();



        std::vector<HvyDX::VHG_PoCo>const& SpaceCurveVector() { return m_spaceCurvePoints; }



        void  TubeRasterizerSetWireframe() { if (m_rasterizerUseWireframe == false) { m_rasterizerUseWireframe = true; CreateTubeRasterizerState(); } }


        void  TubeRasterizerSetSolid() { if (m_rasterizerUseWireframe == true) { m_rasterizerUseWireframe = false; CreateTubeRasterizerState(); } }



	private:

        void                ComputeTubeSpaceCurve(std::vector<unsigned short>& pIndexVector);


        void                ComputeTubeGrid(std::vector<unsigned short>& pIndexVector);


        void                CreateTubeVertexBuffer();


		void                CreateTubeRasterizerState();


	private:

		std::shared_ptr<DX::DeviceResources>                    m_deviceResources;


        std::vector<VHG_PoCo>                                   m_spaceCurvePoints;


		Microsoft::WRL::ComPtr<ID3D11InputLayout>	            m_inputLayout;

        
        Microsoft::WRL::ComPtr<ID3D11Buffer>		            m_TubeVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>                    m_TubeIndexBuffer;
        uint32	                                                m_TubeIndexCount;



		Microsoft::WRL::ComPtr<ID3D11VertexShader>	            m_tubeVertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>	        m_tubeGeometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	            m_tubePixelShader;



		Microsoft::WRL::ComPtr<ID3D11RasterizerState>           m_tubeRasterizerState;
        bool                                                    m_rasterizerUseWireframe;


		VHG_WVPStruct	                                        m_WVPConbufData;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		            m_WVPConbufBuffer;


		bool	                                                m_loadingComplete;


	};
}

