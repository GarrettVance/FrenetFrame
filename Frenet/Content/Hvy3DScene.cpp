#include "pch.h"
#include "Hvy3DScene.h"

#include "..\Common\DirectXHelper.h"

using namespace Frenet;

using namespace DirectX;
using namespace Windows::Foundation;


Hvy3DScene::Hvy3DScene(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();


    e_keyboard = std::make_unique<DirectX::Keyboard>();
    e_keyboard->SetWindow(Windows::UI::Core::CoreWindow::GetForCurrentThread());

    e_mouse = std::make_unique<DirectX::Mouse>();
    e_mouse->SetWindow(Windows::UI::Core::CoreWindow::GetForCurrentThread());




    m_Edelsbrunner = std::make_unique<HvyDX::EdelsbrunnerTube>(deviceResources);
}







// Initializes view parameters when the window size changes.

void Hvy3DScene::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.




	// Hvy3DScene makes use of a right-handed coordinate system using row-major matrices.



	//  XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(

	e_perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);



	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(e_perspectiveMatrix)
		);



	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.


	// static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };

	static const XMVECTORF32 eye = { 0.0f, 0.7f, +4.5f, 0.0f };


	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };


    e_viewMatrix = XMMatrixLookAtRH(eye, at, up);


	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(e_viewMatrix));

}















void Hvy3DScene::Update(DX::StepTimer const& timer)
{

	if (!m_loadingComplete) { return; }


    DirectX::Keyboard::State           kb = e_keyboard->GetState();



    if (kb.F3)
    {
        this->m_Edelsbrunner->TubeRasterizerSetSolid();
    }


    if (kb.F4)
    {
        this->m_Edelsbrunner->TubeRasterizerSetWireframe();
    }




    //  Called once per frame, rotates the cube and 
    //  calculates the model and view matrices.
    //  ============================================
	//  keywords: World Transformation, model matrix;
    //      

	float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
	double accumulatedRadians = timer.GetTotalSeconds() * radiansPerSecond;
	float properRadians = static_cast<float>(fmod(accumulatedRadians, DirectX::XM_2PI));


    XMMATRIX mxTubeInclination = XMMatrixRotationX(properRadians);


    // mxTubeInclination = XMMatrixIdentity();  //  undo



    float tube_scale = 0.4f; 
    XMMATRIX mxTubeScaling = XMMatrixScaling(tube_scale, tube_scale, tube_scale); 


    XMMATRIX mxTubeWorldTransformation = mxTubeInclination * mxTubeScaling;

    this->m_Edelsbrunner->Update(mxTubeWorldTransformation, e_viewMatrix, e_perspectiveMatrix); 








    float cubicle_scale = 0.4f;
    XMMATRIX mxCubicleSpin = XMMatrixRotationY(3.f * properRadians) * XMMatrixScaling(cubicle_scale, cubicle_scale, cubicle_scale);



    static uint32_t  idxUpdateCall = 0; 

    idxUpdateCall = (1 + idxUpdateCall) % 3; 

    static uint32_t idxSpaceCurveElt = 0;

        
    XMFLOAT3 spaceCurvePos{ 0.f, 0.f, 0.f }; 

    if (!m_Edelsbrunner->SpaceCurveVector().empty())
    {
        uint32_t card = (uint32_t)(m_Edelsbrunner->SpaceCurveVector().size());

        spaceCurvePos = this->m_Edelsbrunner->SpaceCurveVector().at(idxSpaceCurveElt).pos;

        if (idxUpdateCall == 0) 
        {
            idxSpaceCurveElt += 1; 

            if (idxSpaceCurveElt + 2 == card)
            {
                idxSpaceCurveElt = 0;
            }
        }

    }

    XMMATRIX mxCubicleXlat = XMMatrixTranslation(spaceCurvePos.x, spaceCurvePos.y, spaceCurvePos.z); 


    //  Compose the tube's World transformation (inclination) with the cubicle's spin rotation
    //  and the position of the Space Curve point (mxCubicleXlat): 

    //  First transformation to be applied to the cubicle is its spin rotation. 
    //  Secondly, apply the translation taking the cubicle to a point on the space curve. 
    //  Finally, multiply by the tube's world transformation which gives an inclination 
    //  about the x-axis: 

    //  The cubicle is rotated about its central body axis, the rotated cubicle
    //  is translated to a point on the space curve, then the entire space curve
    //  inclined by rotation about the x-axis.


    XMMATRIX mxCubicleWorldTransformation = mxCubicleSpin * mxCubicleXlat * mxTubeWorldTransformation;


	XMStoreFloat4x4(
        &m_constantBufferData.model, 
        XMMatrixTranspose(
            mxCubicleWorldTransformation
        )
    );
}








































void Hvy3DScene::Render()
{

    // Loading is asynchronous. Only draw geometry after it's loaded.

	if (!m_loadingComplete) { return; }






    this->m_Edelsbrunner->Render();





	auto context = m_deviceResources->GetD3DDeviceContext();



    //  hazard: 
    //  The Edelsbrunner class renders using a Geometry Shader. 
    //  Must remove the Geometry Shader from the pipeline
    //  prior to rendering other models: 


	context->GSSetShader( nullptr, nullptr, 0 );



    context->RSSetState(e_CubicleRasterizer.Get()); 




	context->UpdateSubresource1( m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0 );


	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers( 0, 1, m_CubeVertexBuffer.GetAddressOf(), &stride, &offset );

	context->IASetIndexBuffer(
		m_CubeIndexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
		);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
		);

	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
		);
}





























void Hvy3DScene::CreateDeviceDependentResources()
{


    D3D11_RASTERIZER_DESC   rasterizer_description;
    ZeroMemory(&rasterizer_description, sizeof(rasterizer_description));

    rasterizer_description.MultisampleEnable = FALSE;
    rasterizer_description.FrontCounterClockwise = true;

    rasterizer_description.FillMode = D3D11_FILL_SOLID;
    rasterizer_description.CullMode = D3D11_CULL_BACK; 

    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(
        &rasterizer_description,
        e_CubicleRasterizer.ReleaseAndGetAddressOf()
    ));






	// Load shaders asynchronously.

	auto loadVSTask = DX::ReadDataAsync(L"t1VertexShader.cso");

	auto loadPSTask = DX::ReadDataAsync(L"t1PixelShader.cso");



	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
				)
			);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
				)
			);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer) , D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this] () {

		// Load mesh vertices. Each vertex has a position and a color.
		static const VertexPositionColor cubeVertices[] = 
		{
			{XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			{XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			{XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			{XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			{XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			{XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			{XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			{XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = cubeVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_CubeVertexBuffer
				)
			);

		// Load mesh indices. Each trio of indices represents
		// a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes
		// 0, 2 and 1 from the vertex buffer compose the 
		// first triangle of this mesh.
		static const unsigned short cubeIndices [] =
		{
			0,2,1, // -x
			1,2,3,

			4,5,6, // +x
			5,7,6,

			0,1,5, // -y
			0,5,4,

			2,6,7, // +y
			2,7,3,

			0,4,6, // -z
			0,6,2,

			1,3,7, // +z
			1,7,5,
		};

		m_indexCount = ARRAYSIZE(cubeIndices);

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_CubeIndexBuffer
				)
			);
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});
}

void Hvy3DScene::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_CubeVertexBuffer.Reset();
	m_CubeIndexBuffer.Reset();
}
