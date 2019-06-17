



#include "pch.h"




#include "EdelsbrunnerTubes.h"



#include "..\Common\DirectXHelper.h"










using namespace HvyDX;

using namespace DirectX;
using namespace Windows::Foundation;







EdelsbrunnerTube::EdelsbrunnerTube(const std::shared_ptr<DX::DeviceResources>& deviceResources) :

	m_loadingComplete(false),

    m_rasterizerUseWireframe(false),

	m_TubeIndexCount(0),

	m_deviceResources(deviceResources)
{

	CreateDeviceDependentResources();
}























void EdelsbrunnerTube::Update(
    XMMATRIX        const&           p_tubeWorldMatrix,
    XMMATRIX        const&           p_sceneViewMatrix,
    XMMATRIX        const&           p_sceneProjectionMatrix
)
{
    XMStoreFloat4x4(
		&m_WVPConbufData.model,
		XMMatrixTranspose(p_tubeWorldMatrix)
    );

    XMStoreFloat4x4(
		&m_WVPConbufData.view,
		XMMatrixTranspose(p_sceneViewMatrix)
    );

    XMStoreFloat4x4(
		&m_WVPConbufData.projection,
		XMMatrixTranspose(p_sceneProjectionMatrix)
    );
}
















void EdelsbrunnerTube::Render()
{
	if (!m_loadingComplete) { return; }



	auto context = m_deviceResources->GetD3DDeviceContext();


    context->RSSetState(m_tubeRasterizerState.Get());



	context->UpdateSubresource1( m_WVPConbufBuffer.Get(), 0, NULL, &m_WVPConbufData, 0, 0, 0 );



	UINT stride = sizeof(VHG_PoCo);
	UINT offset = 0;
	context->IASetVertexBuffers( 0, 1, m_TubeVertexBuffer.GetAddressOf(), &stride, &offset );


	context->IASetIndexBuffer(
		m_TubeIndexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);


    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
	context->IASetInputLayout(m_inputLayout.Get());



    //   
    //      draw the enclosing tube: 
    //    

	context->VSSetShader( m_tubeVertexShader.Get(), nullptr, 0 );
	context->VSSetConstantBuffers1(0, 1, m_WVPConbufBuffer.GetAddressOf(), nullptr, nullptr );



	context->GSSetShader( m_tubeGeometryShader.Get(), nullptr, 0 );
	context->GSSetConstantBuffers1(0, 1, m_WVPConbufBuffer.GetAddressOf(), nullptr, nullptr );



	context->PSSetShader( m_tubePixelShader.Get(), nullptr, 0 );


    //  80





#ifdef GHV_OPTION_CREATE_GRID 
    int nAxialDivs = 16;
    for (int idxColumn = 0; idxColumn < 5; idxColumn++)
    {
        context->DrawIndexed(nAxialDivs, idxColumn * nAxialDivs, 0);
    }
#else

	//  undo context->DrawIndexed( m_TubeIndexCount, 0, 0 );  // ghv : use DrawIndexed();

	context->DrawIndexed( m_TubeIndexCount, 0, 0 ); 

#endif


}














void EdelsbrunnerTube::ComputeTubeGrid(std::vector<unsigned short>& pIndexVector)
{
    m_spaceCurvePoints.clear(); //  clear the vector;

    float halfEdgeLength = 5.f;  // was  3.0f;

    unsigned short nAxialDivisions = 16; //  16;  // good: 64; //  16;

    unsigned short nColumns = 5; 


    float x_coord = 0.f;
    float y_coord = 0.f;
    float z_coord = 0.f;
    VHG_PoCo tmp_vertexa;


    // size_t ghv1 = sizeof(nDivs); 

    unsigned short kIndex = 0; 

    // for (int idxP = 0; idxP <= 1 + nDivs; idxP++)

    // for (unsigned short idxP = 0; idxP < 1 + nDivs; idxP++)



    for (unsigned short gIdx = 0; gIdx < nColumns; gIdx++)
    {
        y_coord = -halfEdgeLength + gIdx * (2.f * halfEdgeLength) / (float)nColumns;

        for (unsigned short idxP = 0; idxP < nAxialDivisions; idxP++)
        {
            x_coord = -halfEdgeLength + idxP * (2.f * halfEdgeLength) / (float)nAxialDivisions;


            z_coord = 0.5f;  //  hazard: cannot use z = 0.f;  TODO:
            tmp_vertexa = { XMFLOAT3(x_coord, y_coord, z_coord), XMFLOAT3(0.f, 0.f, 0.f) };
            m_spaceCurvePoints.push_back(tmp_vertexa);
            pIndexVector.push_back(kIndex);
            kIndex++;
        }

    }






#if 1 == 2

    for (unsigned short idxP = 0; idxP < nMax; idxP++)
    {
        x_coord = +halfEdgeLength; 

        y_coord = -halfEdgeLength + idxP * (2.f * halfEdgeLength) / (float)nAxialDivisions;

        z_coord = 0.5f;  //  hazard: cannot use z = 0.f;  TODO:
        tmp_vertexa = { XMFLOAT3(x_coord, y_coord, z_coord), XMFLOAT3(0.f, 0.f, 0.f) };
        m_spaceCurvePoints.push_back(tmp_vertexa);
        pIndexVector.push_back(kIndex);
        kIndex++;
    }

#endif



}









void EdelsbrunnerTube::ComputeTubeSpaceCurve(std::vector<unsigned short>& pIndexVector)
{
    m_spaceCurvePoints.clear(); //  clear the vector;


#define GHV_OPTION_NUM_POINTS      250     //  revert to 100  

#define GHV_OPTION_SHAPE_CHOICE     4


    float t_parameter = 0.f;
    float x_coord = 0.f;
    float y_coord = 0.f;
    float z_coord = 0.f;
    float factor_n = 4.f;
    VHG_PoCo tmp_vertexa;


    for (int idx_point = 0; idx_point <= 1 + GHV_OPTION_NUM_POINTS; idx_point++)
    {
        
        t_parameter = 2.f * DirectX::XM_PI * idx_point / (GHV_OPTION_NUM_POINTS); 

#if (GHV_OPTION_SHAPE_CHOICE == 1)

        factor_n = 6.f;

        x_coord = 2.f * factor_n * cosf(3 * 2 * t_parameter) * cosf(4 * t_parameter);
        z_coord = factor_n * sinf(3 * 2 * t_parameter) * cosf(4 * t_parameter);
        y_coord = factor_n * sinf(3 * 2 * t_parameter); //  *cosf(4 * t_parameter); // Lissajous++;


#elif (GHV_OPTION_SHAPE_CHOICE == 2)

        factor_n = 3.f;
        
        x_coord = factor_n * cosf(3 * 2 * t_parameter) * cosf(4 * t_parameter);
        z_coord = factor_n * sinf(3 * 2 * t_parameter) * cosf(4 * t_parameter);
        y_coord = -1.2f + (factor_n * x_coord * cosf(3 * 2 * t_parameter) / (2.f - cosf(4 * t_parameter))); // Danish Modern;


#elif (GHV_OPTION_SHAPE_CHOICE == 3)

        factor_n = 6.f;

        x_coord = factor_n * cosf(3 * 2 * t_parameter) * cosf(4 * t_parameter);
        z_coord = factor_n * sinf(3 * 2 * t_parameter) * cosf(4 * t_parameter);
        y_coord = factor_n * sinf(4 * t_parameter); // Slow-hypnol;



#elif (GHV_OPTION_SHAPE_CHOICE == 4)

        //  Edelsbrunner: 

        float omega = 2.f; 
        t_parameter = 1.0f * DirectX::XM_2PI * idx_point / (-1 + GHV_OPTION_NUM_POINTS);  // for omega = 2, period = 2pi;


        omega = 5.f / 2.f; 
        t_parameter = 2.0f * DirectX::XM_2PI * idx_point / (-1 + GHV_OPTION_NUM_POINTS);  // for omega = 5/2, period = 4pi;

        factor_n = 4.2f;

        x_coord = factor_n * cosf(t_parameter) - 0.5f * factor_n * sinf(omega * t_parameter) * cosf(t_parameter);
        z_coord = factor_n * sinf(t_parameter) - 0.5f * factor_n * sinf(omega * t_parameter) * sinf(t_parameter);
        y_coord = 0.5f * factor_n * cosf(omega * t_parameter);

#else

        float decay = 1.f - (float)idx_point / (float)GHV_OPTION_NUM_POINTS;

        x_coord = decay * factor_n * cosf(3 * 2 * t_parameter) * cosf(4 * t_parameter);
        z_coord = decay * factor_n * sinf(3 * 2 * t_parameter) * cosf(4 * t_parameter);
        y_coord = decay * factor_n * sinf(4 * t_parameter); // Decay-hypnol;

#endif

        tmp_vertexa = { XMFLOAT3(x_coord, y_coord, z_coord), XMFLOAT3(0.f, 0.f, 0.f) };
        m_spaceCurvePoints.push_back(tmp_vertexa);
        pIndexVector.push_back(idx_point);
    }
}






















void EdelsbrunnerTube::CreateTubeVertexBuffer()
{
    std::vector<unsigned short> space_curve_indices;


#ifdef GHV_OPTION_CREATE_GRID 

    ComputeTubeGrid(space_curve_indices);

#else

    ComputeTubeSpaceCurve(space_curve_indices);

#endif 



    D3D11_SUBRESOURCE_DATA vb_subresource_data = {0};

    vb_subresource_data.pSysMem = &(m_spaceCurvePoints[0]);

    vb_subresource_data.SysMemPitch = 0;
    vb_subresource_data.SysMemSlicePitch = 0;


    CD3D11_BUFFER_DESC vertexBufferDesc(
        sizeof(VHG_PoCo) * (UINT)m_spaceCurvePoints.size(),
        D3D11_BIND_VERTEX_BUFFER
    );
	

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vb_subresource_data,
            &m_TubeVertexBuffer
            )
    );

   
    m_TubeIndexCount = (UINT)space_curve_indices.size();


		D3D11_SUBRESOURCE_DATA indexBufferData = {0};

		indexBufferData.pSysMem = &(space_curve_indices[0]);

		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;



        CD3D11_BUFFER_DESC indexBufferDesc(
            sizeof(unsigned short) * (UINT)space_curve_indices.size(), 
            D3D11_BIND_INDEX_BUFFER
        );



		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_TubeIndexBuffer
				)
			);


}












void EdelsbrunnerTube::CreateTubeRasterizerState()
{
    D3D11_RASTERIZER_DESC   rasterizer_description;
    ZeroMemory(&rasterizer_description, sizeof(rasterizer_description));


    rasterizer_description.MultisampleEnable = FALSE;


    rasterizer_description.FrontCounterClockwise = true;



    if (m_rasterizerUseWireframe)
    {
        rasterizer_description.FillMode = D3D11_FILL_WIREFRAME; 
        rasterizer_description.CullMode = D3D11_CULL_NONE; 
    }
    else
    {
        rasterizer_description.FillMode = D3D11_FILL_SOLID;
        rasterizer_description.CullMode = D3D11_CULL_BACK;
    }


    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(
        &rasterizer_description,
        m_tubeRasterizerState.ReleaseAndGetAddressOf() )
    );
}








void EdelsbrunnerTube::CreateDeviceDependentResources()
{

    CreateTubeRasterizerState();



	auto loadVS_Tube_Task = DX::ReadDataAsync(L"EdelsVS_TubeVertexShader.cso");
	auto loadGSTask = DX::ReadDataAsync(L"EdelsGS_TubeGeometryShader.cso");
    auto loadPSTask = DX::ReadDataAsync(L"EdelsPS_TubePixelShader.cso");











    auto createGSTask = loadGSTask.then([this](const std::vector<byte>& fileData) {
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGeometryShader(
            &fileData[0], fileData.size(), nullptr, &m_tubeGeometryShader));
    });




	auto createVS_TubeTask = loadVS_Tube_Task.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0], fileData.size(), nullptr, &m_tubeVertexShader ) );

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









	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_tubePixelShader
				)
			);



		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(VHG_WVPStruct) , D3D11_BIND_CONSTANT_BUFFER);


		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_WVPConbufBuffer
				)
			);
	});








	auto createCubeTask = (createPSTask && createVS_TubeTask && createGSTask).then([this] () 
    {
		// Load mesh vertices. 

        CreateTubeVertexBuffer();  

	});





	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});
}










void EdelsbrunnerTube::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;



	m_inputLayout.Reset();


	m_tubeVertexShader.Reset();

	m_tubePixelShader.Reset();


	m_WVPConbufBuffer.Reset();


	m_TubeVertexBuffer.Reset();



	//  m_indexBuffer.Reset();
}
