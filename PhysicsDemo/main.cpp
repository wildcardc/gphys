#include <sstream>
#include <iomanip>
#include <random>
#include <iostream>

//DirectX includes
#include <DirectXMath.h>
using namespace DirectX;

// Effect framework includes
#include <d3dx11effect.h>

// DXUT includes
#include <DXUT.h>
#include <DXUTcamera.h>

// DirectXTK includes
#include "Effects.h"
#include "VertexTypes.h"
#include "PrimitiveBatch.h"
#include "GeometricPrimitive.h"
#include "ScreenGrab.h"

// AntTweakBar includes
#include "AntTweakBar.h"

// Internal includes
#include "util/util.h"

#include "MassSpringSystem.h"
#include "RigidBody.h"
 
// DXUT camera
// NOTE: CModelViewerCamera does not only manage the standard view transformation/camera position 
//       (CModelViewerCamera::GetViewMatrix()), but also allows for model rotation
//       (CModelViewerCamera::GetWorldMatrix()). 
//       Look out for CModelViewerCamera::SetButtonMasks(...).
CModelViewerCamera g_camera;

// Effect corresponding to "effect.fx"
ID3DX11Effect* g_pEffect = nullptr;

// Main tweak bar
TwBar* g_pTweakBar;
TwBar* g_pMSSBar = 0;

// DirectXTK effect, input layout and primitive batch for position/color vertices
// (for drawing multicolored & unlit primitives)
BasicEffect*                          g_pEffectPositionColor          = nullptr;
ID3D11InputLayout*                    g_pInputLayoutPositionColor     = nullptr;
PrimitiveBatch<VertexPositionColor>*  g_pPrimitiveBatchPositionColor  = nullptr;

// DirectXTK effect, input layout and primitive batch for position/normal vertices
// (for drawing unicolor & oriented & lit primitives)
BasicEffect*                          g_pEffectPositionNormal         = nullptr;
ID3D11InputLayout*                    g_pInputLayoutPositionNormal    = nullptr;
PrimitiveBatch<VertexPositionNormal>* g_pPrimitiveBatchPositionNormal = nullptr;

// DirectXTK simple geometric primitives
std::unique_ptr<GeometricPrimitive> g_pSphere;
std::unique_ptr<GeometricPrimitive> g_pTeapot;
std::unique_ptr<GeometricPrimitive> g_pCube;

// Movable object management
XMINT2   g_viMouseDelta = XMINT2(0,0);
XMFLOAT3 g_vfMovableObjectPos = XMFLOAT3(0,0,0);

MassSpringSystem* g_MassSpringSystem = 0;

std::vector<RigidBody*> g_RigidBodySystem;

bool g_bSimulationEnabled = true;

// TweakAntBar GUI variables
int   g_iNumSpheres    = 100;
float g_fSphereSize    = 0.04f;
bool  g_bDrawTeapot    = false;
bool  g_bDrawTriangle  = false;
bool  g_bDrawSpheres   = false;

XMVECTOR g_G = { .0f, -9.81f * .05f, .0f, .0f};

float g_TimeStep = 1.0f/60.0f;
float g_dT = .0f;

 typedef enum
 {
	INTEGRATOR_EXPLICIT_EULER = 0,
	INTEGRATOR_MIDPOINT,
	INTEGRATOR_LEAPFROG,
	
	INTEGRATOR_LAST,
 } Integrators;

Integrators g_Integrator = INTEGRATOR_LEAPFROG;
 
TwEnumVal twIntegratorEV[] = { {INTEGRATOR_EXPLICIT_EULER, "Explicit Euler"}, {INTEGRATOR_MIDPOINT, "Midpoint"}, {INTEGRATOR_LEAPFROG, "Leapfrog"} };
TwType integratorType;


MassSpringSystem* Euler(float, MassSpringSystem*);
MassSpringSystem* MidPoint(float, MassSpringSystem*);
MassSpringSystem* LeapFrog(float, MassSpringSystem*);
void ClearForces(MassSpringSystem* mss);

typedef MassSpringSystem* (*IntegratorPtr)(float, MassSpringSystem*);
IntegratorPtr IntegratorFuncs[] = { &Euler, &MidPoint, &LeapFrog};

void InitRigidBodySystem();
void InitMassSpringSystem();

// Create TweakBar and add required buttons and variables
void InitTweakBar(ID3D11Device* pd3dDevice)
{
    TwInit(TW_DIRECT3D11, pd3dDevice);

    g_pTweakBar = TwNewBar("Main");

    // HINT: For buttons you can directly pass the callback function as a lambda expression.
    TwAddButton(g_pTweakBar, "Reset Camera", [](void *){g_camera.Reset();}, nullptr, "");
	TwAddButton(g_pTweakBar, "Reset MSS", [](void *){InitMassSpringSystem();}, nullptr, "");
	TwAddVarRW(g_pTweakBar, "Enable Simulation",  TW_TYPE_BOOLCPP, &g_bSimulationEnabled, "");

    //TwAddVarRW(g_pTweakBar, "Draw Teapot",   TW_TYPE_BOOLCPP, &g_bDrawTeapot, "");
    //TwAddVarRW(g_pTweakBar, "Draw Triangle", TW_TYPE_BOOLCPP, &g_bDrawTriangle, "");
    //TwAddVarRW(g_pTweakBar, "Draw Spheres",  TW_TYPE_BOOLCPP, &g_bDrawSpheres, "");
    //TwAddVarRW(g_pTweakBar, "Num Spheres",   TW_TYPE_INT32, &g_iNumSpheres, "min=1");
    TwAddVarRW(g_pTweakBar, "Sphere Size",   TW_TYPE_FLOAT, &g_fSphereSize, "min=0.01 step=0.01");
	TwAddVarRW(g_pTweakBar, "Gravity",   TW_TYPE_DIR3F, &g_G, "");
	
	integratorType = TwDefineEnum("IntegratorType", twIntegratorEV, INTEGRATOR_LAST);
	TwAddVarRW(g_pTweakBar, "Integrator", integratorType, &g_Integrator, NULL);
}

// Draw the edges of the bounding box [-0.5;0.5]� rotated with the cameras model tranformation.
// (Drawn as line primitives using a DirectXTK primitive batch)
void DrawBoundingBox(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/color effect
    g_pEffectPositionColor->SetWorld(g_camera.GetWorldMatrix());
    
    g_pEffectPositionColor->Apply(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionColor);

    // Draw
    g_pPrimitiveBatchPositionColor->Begin();
    
    // Lines in x direction (red color)
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet(-0.5f, (float)(i%2)-0.5f, (float)(i/2)-0.5f, 1), Colors::Red),
            VertexPositionColor(XMVectorSet( 0.5f, (float)(i%2)-0.5f, (float)(i/2)-0.5f, 1), Colors::Red)
        );
    }

    // Lines in y direction
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, -0.5f, (float)(i/2)-0.5f, 1), Colors::Green),
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f,  0.5f, (float)(i/2)-0.5f, 1), Colors::Green)
        );
    }

    // Lines in z direction
    for (int i=0; i<4; i++)
    {
        g_pPrimitiveBatchPositionColor->DrawLine(
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, (float)(i/2)-0.5f, -0.5f, 1), Colors::Blue),
            VertexPositionColor(XMVectorSet((float)(i%2)-0.5f, (float)(i/2)-0.5f,  0.5f, 1), Colors::Blue)
        );
    }

    g_pPrimitiveBatchPositionColor->End();
}

// Draw a large, square plane at y=-1.
// (Drawn as a quad, i.e. triangle strip, using a DirectXTK primitive batch)
void DrawFloor(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal effect
    g_pEffectPositionNormal->SetWorld(XMMatrixIdentity());
    g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetDiffuseColor(0.6f * Colors::White);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(1000);
    
    g_pEffectPositionNormal->Apply(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionNormal);

    // Draw
    g_pPrimitiveBatchPositionNormal->Begin();
    g_pPrimitiveBatchPositionNormal->DrawQuad(
        VertexPositionNormal(XMFLOAT3(-100, -1,  100), XMFLOAT3(0,1,0)),
        VertexPositionNormal(XMFLOAT3( 100, -1,  100), XMFLOAT3(0,1,0)),
        VertexPositionNormal(XMFLOAT3( 100, -1, -100), XMFLOAT3(0,1,0)),
        VertexPositionNormal(XMFLOAT3(-100, -1, -100), XMFLOAT3(0,1,0))
    );
    g_pPrimitiveBatchPositionNormal->End();    
}

void DrawMSS(ID3D11DeviceContext* pd3dImmediateContext)
{
	if(!g_MassSpringSystem)
		return;

    // Setup position/normal effect (constant variables)
    g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(100);
      
    std::mt19937 eng;
    std::uniform_real_distribution<float> randCol( 0.0f, 1.0f);

	for (auto i = g_MassSpringSystem->points.begin(); i != g_MassSpringSystem->points.end(); i++)
    {
        // Setup position/normal effect (per object variables)
        g_pEffectPositionNormal->SetDiffuseColor(0.6f * XMColorHSVToRGB(XMVectorSet(randCol(eng), 1, 1, 0)));
        XMMATRIX scale    = XMMatrixScaling(g_fSphereSize, g_fSphereSize, g_fSphereSize);
        XMMATRIX trans    = XMMatrixTranslation(XMVectorGetX((*i)->position), XMVectorGetY((*i)->position), XMVectorGetZ((*i)->position));
        g_pEffectPositionNormal->SetWorld(scale * trans * g_camera.GetWorldMatrix());

        // Draw
        // NOTE: The following generates one draw call per object, so performance will be bad for n>>1000 or so
        g_pSphere->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
    }

	g_pEffectPositionColor->SetWorld(g_camera.GetWorldMatrix());
    g_pEffectPositionColor->Apply(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout(g_pInputLayoutPositionColor);

    // Draw
    g_pPrimitiveBatchPositionColor->Begin();

	for (auto i = g_MassSpringSystem->springs.begin(); i != g_MassSpringSystem->springs.end(); i++)
	{
		g_pPrimitiveBatchPositionColor->DrawLine(
			VertexPositionColor(g_MassSpringSystem->points[(*i)->point1]->position, Colors::Blue),
			VertexPositionColor(g_MassSpringSystem->points[(*i)->point2]->position, Colors::Blue));
	}

    g_pPrimitiveBatchPositionColor->End();
}

void DrawRBS(ID3D11DeviceContext* pd3dImmediateContext)
{
	for(auto i = g_RigidBodySystem.cbegin(); i != g_RigidBodySystem.cend(); i++)
	{
		g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
		g_pEffectPositionNormal->SetDiffuseColor(0.6f * Colors::Cornsilk);
		g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
		g_pEffectPositionNormal->SetSpecularPower(100);

		XMMATRIX scale    = XMMatrixScalingFromVector((*i)->size);    
		XMMATRIX trans    = XMMatrixTranslationFromVector((*i)->position);
		g_pEffectPositionNormal->SetWorld(scale * trans);

		// Draw
		g_pCube->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
	}
}

// Draw several objects randomly positioned in [-0.5f;0.5]�  using DirectXTK geometric primitives.
void DrawSomeRandomObjects(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal effect (constant variables)
    g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(100);
      
    std::mt19937 eng;
    std::uniform_real_distribution<float> randCol( 0.0f, 1.0f);
    std::uniform_real_distribution<float> randPos(-0.5f, 0.5f);

    for (int i=0; i<g_iNumSpheres; i++)
    {
        // Setup position/normal effect (per object variables)
        g_pEffectPositionNormal->SetDiffuseColor(0.6f * XMColorHSVToRGB(XMVectorSet(randCol(eng), 1, 1, 0)));
        XMMATRIX scale    = XMMatrixScaling(g_fSphereSize, g_fSphereSize, g_fSphereSize);
        XMMATRIX trans    = XMMatrixTranslation(randPos(eng),randPos(eng),randPos(eng));
        g_pEffectPositionNormal->SetWorld(scale * trans * g_camera.GetWorldMatrix());

        // Draw
        // NOTE: The following generates one draw call per object, so performance will be bad for n>>1000 or so
        g_pSphere->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
    }
}

// Draw a teapot at the position g_vfMovableObjectPos.
void DrawMovableTeapot(ID3D11DeviceContext* pd3dImmediateContext)
{
    // Setup position/normal effect (constant variables)
    g_pEffectPositionNormal->SetEmissiveColor(Colors::Black);
    g_pEffectPositionNormal->SetDiffuseColor(0.6f * Colors::Cornsilk);
    g_pEffectPositionNormal->SetSpecularColor(0.4f * Colors::White);
    g_pEffectPositionNormal->SetSpecularPower(100);

    XMMATRIX scale    = XMMatrixScaling(0.5f, 0.5f, 0.5f);    
    XMMATRIX trans    = XMMatrixTranslation(g_vfMovableObjectPos.x, g_vfMovableObjectPos.y, g_vfMovableObjectPos.z);
    g_pEffectPositionNormal->SetWorld(scale * trans);

    // Draw
    g_pTeapot->Draw(g_pEffectPositionNormal, g_pInputLayoutPositionNormal);
}

// Draw a simple triangle using custom shaders (g_pEffect)
void DrawTriangle(ID3D11DeviceContext* pd3dImmediateContext)
{
	XMMATRIX world = g_camera.GetWorldMatrix();
	XMMATRIX view  = g_camera.GetViewMatrix();
	XMMATRIX proj  = g_camera.GetProjMatrix();
	XMFLOAT4X4 mViewProj;
	XMStoreFloat4x4(&mViewProj, world * view * proj);
	g_pEffect->GetVariableByName("g_worldViewProj")->AsMatrix()->SetMatrix((float*)mViewProj.m);
	g_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->Apply(0, pd3dImmediateContext);
    
	pd3dImmediateContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	pd3dImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
	pd3dImmediateContext->IASetInputLayout(nullptr);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dImmediateContext->Draw(3, 0);
}


// ============================================================
// DXUT Callbacks
// ============================================================


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();;

    std::wcout << L"Device: " << DXUTGetDeviceStats() << std::endl;
    
    // Load custom effect from "effect.fxo" (compiled "effect.fx")
	std::wstring effectPath = GetExePath() + L"effect.fxo";
	if(FAILED(hr = D3DX11CreateEffectFromFile(effectPath.c_str(), 0, pd3dDevice, &g_pEffect)))
	{
        std::wcout << L"Failed creating effect with error code " << int(hr) << std::endl;
		return hr;
	}

    // Init AntTweakBar GUI
    InitTweakBar(pd3dDevice);

    // Create DirectXTK geometric primitives for later usage
    g_pSphere = GeometricPrimitive::CreateGeoSphere(pd3dImmediateContext, 1.0f, 2, false);
    g_pTeapot = GeometricPrimitive::CreateTeapot(pd3dImmediateContext, 1.5f, 8, false);
	g_pCube = GeometricPrimitive::CreateCube(pd3dImmediateContext, 1.0f, false);

    // Create effect, input layout and primitive batch for position/color vertices (DirectXTK)
    {
        // Effect
        g_pEffectPositionColor = new BasicEffect(pd3dDevice);
        g_pEffectPositionColor->SetVertexColorEnabled(true); // triggers usage of position/color vertices

        // Input layout
        void const* shaderByteCode;
        size_t byteCodeLength;
        g_pEffectPositionColor->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
        
        pd3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
                                      VertexPositionColor::InputElementCount,
                                      shaderByteCode, byteCodeLength,
                                      &g_pInputLayoutPositionColor);

        // Primitive batch
        g_pPrimitiveBatchPositionColor = new PrimitiveBatch<VertexPositionColor>(pd3dImmediateContext);
    }

    // Create effect, input layout and primitive batch for position/normal vertices (DirectXTK)
    {
        // Effect
        g_pEffectPositionNormal = new BasicEffect(pd3dDevice);
        g_pEffectPositionNormal->EnableDefaultLighting(); // triggers usage of position/normal vertices
        g_pEffectPositionNormal->SetPerPixelLighting(true);

        // Input layout
        void const* shaderByteCode;
        size_t byteCodeLength;
        g_pEffectPositionNormal->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        pd3dDevice->CreateInputLayout(VertexPositionNormal::InputElements,
                                      VertexPositionNormal::InputElementCount,
                                      shaderByteCode, byteCodeLength,
                                      &g_pInputLayoutPositionNormal);

        // Primitive batch
        g_pPrimitiveBatchPositionNormal = new PrimitiveBatch<VertexPositionNormal>(pd3dImmediateContext);
    }

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
	if (g_MassSpringSystem != 0)
			delete g_MassSpringSystem;

	SAFE_RELEASE(g_pEffect);
	
    TwDeleteAllBars();
	TwTerminate();

    g_pSphere.reset();
    g_pTeapot.reset();
	g_pCube.reset();
    
    SAFE_DELETE (g_pPrimitiveBatchPositionColor);
    SAFE_RELEASE(g_pInputLayoutPositionColor);
    SAFE_DELETE (g_pEffectPositionColor);

    SAFE_DELETE (g_pPrimitiveBatchPositionNormal);
    SAFE_RELEASE(g_pInputLayoutPositionNormal);
    SAFE_DELETE (g_pEffectPositionNormal);
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    // Update camera parameters
	int width = pBackBufferSurfaceDesc->Width;
	int height = pBackBufferSurfaceDesc->Height;
	g_camera.SetWindow(width, height);
	g_camera.SetProjParams(XM_PI / 4.0f, float(width) / float(height), 0.1f, 100.0f);

    // Inform AntTweakBar about back buffer resolution change
  	TwWindowSize(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	if(bKeyDown)
	{
		switch(nChar)
		{
            // ALT+RETURN: toggle fullscreen
			case VK_RETURN :
			{
				if(bAltDown) DXUTToggleFullScreen();
				break;
			}
            // F8: Take screenshot
			case VK_F8:
			{
                // Save current render target as png
                static int nr = 0;
				std::wstringstream ss;
				ss << L"Screenshot" << std::setfill(L'0') << std::setw(4) << nr++ << L".png";

                ID3D11Resource* pTex2D = nullptr;
                DXUTGetD3D11RenderTargetView()->GetResource(&pTex2D);
                SaveWICTextureToFile(DXUTGetD3D11DeviceContext(), pTex2D, GUID_ContainerFormatPng, ss.str().c_str());
                SAFE_RELEASE(pTex2D);

                std::wcout << L"Screenshot written to " << ss.str() << std::endl;
				break;
			}
			default : return;
		}
	}
}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext )
{
    // Track mouse movement if left mouse key is pressed
    {
        static int xPosSave = 0, yPosSave = 0;

        if (bLeftButtonDown)
        {
            // Accumulate deltas in g_viMouseDelta
            g_viMouseDelta.x += xPos - xPosSave;
            g_viMouseDelta.y += yPos - yPosSave;
        }    

        xPosSave = xPos;
        yPosSave = yPos;
    }
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
    // Send message to AntTweakbar first
    if (TwEventWin(hWnd, uMsg, wParam, lParam))
    {
        *pbNoFurtherProcessing = true;
        return 0;
    }

    // If message not processed yet, send to camera
	if(g_camera.HandleMessages(hWnd,uMsg,wParam,lParam))
    {
        *pbNoFurtherProcessing = true;
		return 0;
    }

	return 0;
}

void DoMSSPhysics(float dt)
{
	//clear forces
	ClearForces(g_MassSpringSystem);

	g_MassSpringSystem = IntegratorFuncs[g_Integrator](g_TimeStep, g_MassSpringSystem);

	// collisions
	for (auto i = g_MassSpringSystem->points.begin(); i != g_MassSpringSystem->points.end(); i++)
    {
		// boundary check -y; has to be done first for now
		if(XMVectorGetY((*i)->position) < -.5f)
			(*i)->position = XMVectorSetY((*i)->position, -.5f);
		
		// really really crude MP-MP collision
		for (auto j = i + 1; j != g_MassSpringSystem->points.end(); )
		{
			assert(*i != *j);

			auto d = (*i)->position - (*j)->position;
			float l = XMVectorGetX(XMVector3Length(d));

			if (l < g_fSphereSize)
			{
				// collision response away from Y plane
				auto top = XMVectorGetY((*j)->position) < XMVectorGetY((*i)->position) ? *i : *j;
				auto n = XMVector3Normalize(d);
				auto respone = n * (g_fSphereSize - l);

				if(XMVectorGetY(n) > 0)
					top->position += respone;
				else
					top->position -= respone;
			}
		}
	}
}

void FixBoxCollision(RigidBody* a, RigidBody* b)
{

}

void DoRBSPhysics(float dt)
{
	for(auto i = g_RigidBodySystem.cbegin(); i != g_RigidBodySystem.cend(); i++)
	{
		for(int j = 0; j < 8; j++)
		{

		}
	}
	

	for(auto i = g_RigidBodySystem.cbegin(); i != g_RigidBodySystem.cend(); i++)
	{

		for(auto j = i + 1; j != g_RigidBodySystem.cend(); j++)
		{
			FixBoxCollision(*i, *j);
			FixBoxCollision(*j, *i);
		}

	}
}

void DoPhysics(float dt)
{
	if(g_MassSpringSystem)
		DoMSSPhysics(dt);

	DoRBSPhysics(dt);
}

void ClearForces(MassSpringSystem* mss)
{
	for(auto i = mss->points.begin(); i != mss->points.end(); i++)
	{
		(*i)->force = XMVectorZero();
	}
}

void ApplyForces(MassSpringSystem* mss)
{

	int k = 0;
	//Update Springs
	for (auto i = mss->springs.begin(); i != mss->springs.end(); i++)
	{
		//calculate spring forces
		float l = (*i)->currentLength(mss);
		if(l == 0)
			l = .001f;

		XMVECTOR f = -(*i)->stiffness*(l - (*i)->initialLength) * (mss->points[(*i)->point1]->position - mss->points[(*i)->point2]->position) / l;

		//overwrite old forces with spring forces
		mss->points[(*i)->point1]->force += f;
		mss->points[(*i)->point2]->force += -f;
		k++;
	}

	for (auto i = mss->points.begin(); i != mss->points.end(); i++)
    {
		//update forces with gravitation forces
		(*i)->force -= (*i)->damping * (*i)->velocity;
		//(*i)->force -= (*i)->damping * (*i)->force;
	}

	//old version -----------------------------------------------------------------
	//int k = 0;
	////Update Springs
	//for (auto i = mss->springs.begin(); i != mss->springs.end(); i++)
	//{
	//	//calculate spring forces
	//	float l = (*i)->currentLength(mss);
	//	if(l == 0)
	//		l = .001f;

	//	XMVECTOR f = -(*i)->stiffness*(l - (*i)->initialLength) * (mss->points[(*i)->point1]->position - mss->points[(*i)->point2]->position) / l;

	//	//overwrite old forces with spring forces
	//	mss->points[(*i)->point1]->force = f;
	//	mss->points[(*i)->point2]->force = -f;
	//	k++;
	//}

	//for (auto i = mss->points.begin(); i != mss->points.end(); i++)
 //   {
	//	//update forces with gravitation forces
	//	//(*i)->force += g_G * (*i)->mass - (*i)->damping * (*i)->velocity;
	//	(*i)->force -= (*i)->damping * (*i)->velocity;
	//}
	
	// external forces..
}

MassSpringSystem* Euler(float dt, MassSpringSystem* mss)
{
	MassSpringSystem* out_mss = new MassSpringSystem(*mss);

	ApplyForces(out_mss);

	for (auto i = out_mss->points.begin(); i != out_mss->points.end(); i++)
    {
		//update positions
		(*i)->position += dt * (*i)->velocity;

		//update velocity
		//m*a = Fint + Fext - y*v
		//F = m * a = kg * m/s^2 - m/s * kg/s
		//v = (Fint/m + Fext/m - yv) * dt
		(*i)->velocity += ((*i)->force / (*i)->mass + g_G) * dt;
	}

	return out_mss;
}

// invalid value to always trigger the initial leapfrog step
Integrators g_PrevInt = INTEGRATOR_LAST;

MassSpringSystem* LeapFrog(float dt, MassSpringSystem* mss)
{
	ApplyForces(mss);

	for (auto i = mss->points.begin(); i != mss->points.end(); i++)
	{
		// while this doesnt really seem to do much, we'll just keep it for now
		if(g_PrevInt != INTEGRATOR_LEAPFROG)
			(*i)->velocity += ((*i)->force / (*i)->mass + g_G) * dt/2;
		else
			(*i)->velocity += ((*i)->force / (*i)->mass + g_G) * dt;
			
		//update positions
		(*i)->position += dt * (*i)->velocity;
	}
	
	return mss;
}

MassSpringSystem* MidPoint(float dt, MassSpringSystem* mss)
{
	MassSpringSystem* euler = Euler(dt/2, mss);

	ApplyForces(euler);

	for (int i = 0; i < mss->points.size(); i++)
    {
		//update positions
		mss->points[i]->position += dt * euler->points[i]->velocity;

		//update velocity
		mss->points[i]->velocity = euler->points[i]->velocity;
		//mss->points[i]->velocity += euler->points[i]->force / euler->points[i]->mass * dt + g_G * dt;
	}

	delete euler;

	return mss;
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double dTime, float fElapsedTime, void* pUserContext )
{
	UpdateWindowTitle(L"PhysicsDemo");

    // Move camera
    g_camera.FrameMove(fElapsedTime);

	if (g_bSimulationEnabled)
	{
		g_dT += fElapsedTime;
		while(g_dT >= g_TimeStep)
		{
			DoPhysics(g_TimeStep);
			g_dT -= g_TimeStep;
		}
		
		g_PrevInt = g_Integrator;
	}

    // Update effects with new view + proj transformations
    g_pEffectPositionColor->SetView       (g_camera.GetViewMatrix());
    g_pEffectPositionColor->SetProjection (g_camera.GetProjMatrix());

    g_pEffectPositionNormal->SetView      (g_camera.GetViewMatrix());
    g_pEffectPositionNormal->SetProjection(g_camera.GetProjMatrix());

    // Apply accumulated mouse deltas to g_vfMovableObjectPos (move along cameras view plane)
    if (g_viMouseDelta.x != 0 || g_viMouseDelta.y != 0)
    {
        // Calcuate camera directions in world space
        XMMATRIX viewInv = XMMatrixInverse(nullptr, g_camera.GetViewMatrix());
        XMVECTOR camRightWorld = XMVector3TransformNormal(g_XMIdentityR0, viewInv);
        XMVECTOR camUpWorld    = XMVector3TransformNormal(g_XMIdentityR1, viewInv);

        // Add accumulated mouse deltas to movable object pos
        XMVECTOR vMovableObjectPos = XMLoadFloat3(&g_vfMovableObjectPos);

        float speedScale = 0.001f;
        vMovableObjectPos = XMVectorAdd(vMovableObjectPos,  speedScale * (float)g_viMouseDelta.x * camRightWorld);
        vMovableObjectPos = XMVectorAdd(vMovableObjectPos, -speedScale * (float)g_viMouseDelta.y * camUpWorld);

        XMStoreFloat3(&g_vfMovableObjectPos, vMovableObjectPos);
        
        // Reset accumulated mouse deltas
        g_viMouseDelta = XMINT2(0,0);
    }
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{
	// Clear render target and depth stencil
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView( pRTV, ClearColor );
	pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    // Draw floor
    DrawFloor(pd3dImmediateContext);

    // Draw axis box
    DrawBoundingBox(pd3dImmediateContext);

    // Draw speheres
    if (g_bDrawSpheres) DrawSomeRandomObjects(pd3dImmediateContext);

    // Draw movable teapot
    if (g_bDrawTeapot) DrawMovableTeapot(pd3dImmediateContext);

    // Draw simple triangle
    if (g_bDrawTriangle) DrawTriangle(pd3dImmediateContext);

	DrawMSS(pd3dImmediateContext);

	DrawRBS(pd3dImmediateContext);

    // Draw GUI
    TwDraw();
}

void InitRigidBodySystem()
{
	g_RigidBodySystem.push_back(new RigidBody(XMVectorZero(),XMVectorSet(.1f,.1f,.1f,0),1));
	g_RigidBodySystem.push_back(new RigidBody(XMVectorSet(.05f,.2f,0,1),XMVectorSet(.1f,.1f,.1f,0),1));
}


void InitMassSpringSystem()
{
	if (g_MassSpringSystem != 0)
		delete g_MassSpringSystem;

	g_MassSpringSystem = new MassSpringSystem;

	//Diamond ----------------------------------------------------------------------------------------
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,.0f,.0f,1.0f), 1.0f, 1.0f));

	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,.075f,.0f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,-.075f,.0f,1.0f), 1.0f, 1.0f));

	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.0f,-.1f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.0f,.1f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(-.1f,.0f,.1f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(-.1f,.0f,-.1f,1.0f), 1.0f, 1.0f));


	//Cube--------------------------------------------------------------------------------------------
	/*g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,.0f,.1f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,.1f,.1f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.0f,.1f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.1f,.1f,1.0f), 1.0f, 1.0f));

	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,.0f,.0f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,.1f,.0f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.0f,.0f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.1f,.0f,1.0f), 1.0f, 1.0f));


	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.05f,.05f,.05f,1.0f), 1.0f, 1.0f));*/

	/*g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.05f,.1f,.05f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.05f,.0f,.05f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.05f,.05f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,.05f,.05f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.05f,.05f,.0f,1.0f), 1.0f, 1.0f));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.05f,.05f,.1f,1.0f), 1.0f, 1.0f));*/

	
	//Pyramid----------------------------------------------------------------------------------------
	/*g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.0f,.1f,1.0f), 1.0f, 1));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.0f,-.1f,1.0f), 1.0f, 1));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(-.1f,.0f,.1f,1.0f), 1.0f, 1));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(-.1f,.0f,-.1f,1.0f), 1.0f, 1));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.0f,-.25f,.0f,1.0f), 1.0f, 1));*/

	/*g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.1f,.0f,1.0f), 1.0f, 1));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.2f,.0f,1.0f), 1.0f, 1));
	g_MassSpringSystem->points.push_back(new MassPoint(XMVectorSet(.1f,.3f,.0f,1.0f), 1.0f, 1));*/

	/*Spring* s = new Spring(0,1,250.0f,g_MassSpringSystem);
	Spring* s2 = new Spring(1,2,250.0f,g_MassSpringSystem);
	g_MassSpringSystem->springs.push_back(s);
	g_MassSpringSystem->springs.push_back(s2);*/
	
	for (int i = 0; i < g_MassSpringSystem->points.size(); i++)
	{
		for (int j = i + 1; j < g_MassSpringSystem->points.size(); j++)
		{
			Spring* s = new Spring(j,i,500,g_MassSpringSystem);
			g_MassSpringSystem->springs.push_back(s);
		}
	}
	
	// something down here leaks
	if (g_pMSSBar)
		TwDeleteBar(g_pMSSBar);
	
	g_pMSSBar = TwNewBar("MSS");
	TwDefine("MSS label='Mass Spring System' position='15 400' alpha=222 valueswidth=fit"); // yup magic values

	char cnt[10];
	int cc = 0;
	for(int i = 0; i < g_MassSpringSystem->points.size(); i++)
	{
		char str[255];

		sprintf_s(str, "group='Mass Point %d' label='Mass'", i);
		sprintf_s(cnt, "%d", cc++);
		TwAddVarRW(g_pMSSBar, cnt,   TW_TYPE_FLOAT, &(g_MassSpringSystem->points[i]->mass), str);
		sprintf_s(str, "group='Mass Point %d' label='Damping'", i);
		sprintf_s(cnt, "%d", cc++);
		TwAddVarRW(g_pMSSBar, cnt,   TW_TYPE_FLOAT, &(g_MassSpringSystem->points[i]->damping), str);

		TwAddSeparator(g_pMSSBar, 0, 0);

		sprintf_s(str, "group='Mass Point %d' label='Position'", i);
		sprintf_s(cnt, "%d", cc++);
		TwAddVarRW(g_pMSSBar, cnt,   TW_TYPE_DIR3F, &(g_MassSpringSystem->points[i]->position), str);
		sprintf_s(str, "group='Mass Point %d' label='Velocity'", i);
		sprintf_s(cnt, "%d", cc++);
		TwAddVarRW(g_pMSSBar, cnt,   TW_TYPE_DIR3F, &(g_MassSpringSystem->points[i]->velocity), str);
		sprintf_s(str, "group='Mass Point %d' label='Force'", i);
		sprintf_s(cnt, "%d", cc++);
		TwAddVarRO(g_pMSSBar, cnt,   TW_TYPE_DIR3F, &(g_MassSpringSystem->points[i]->force), str);
	}

	for(int i = 0; i < g_MassSpringSystem->springs.size(); i++)
	{
		char str[255];

		sprintf_s(str, "group='Spring %d' label='Stiffness'", i);
		sprintf_s(cnt, "%d", cc++);
		TwAddVarRW(g_pMSSBar, cnt,   TW_TYPE_FLOAT, &(g_MassSpringSystem->springs[i]->stiffness), str);

		TwAddSeparator(g_pMSSBar, 0, 0);

		sprintf_s(str, "group='Spring %d' label='Initial Length'", i);
		sprintf_s(cnt, "%d", cc++);
		TwAddVarRO(g_pMSSBar, cnt,   TW_TYPE_FLOAT, &(g_MassSpringSystem->springs[i]->initialLength), str);

		//sprintf_s(str, "group='Spring %d' label='Current Length' readonly=true", i);
		//sprintf_s(cnt, "%d", cc++);
		//TwAddVarCB(g_pMSSBar, cnt,   TW_TYPE_FLOAT, 0, Spring::GetCurrentLengthCallback, g_MassSpringSystem->springs[i], str);
	}
}

//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
#if defined(DEBUG) | defined(_DEBUG)
	// Enable run-time memory check for debug builds.
	// (on program exit, memory leaks are printed to Visual Studio's Output console)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

#ifdef _DEBUG
	std::wcout << L"---- DEBUG BUILD ----\n\n";
#endif

	// Set general DXUT callbacks
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackMouse( OnMouse, true );
	DXUTSetCallbackKeyboard( OnKeyboard );

	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

	// Set the D3D11 DXUT callbacks
	DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
	DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
	DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
	DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
	DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
	DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Init camera
 	XMFLOAT3 eye(0.0f, 0.0f, -2.0f);
	XMFLOAT3 lookAt(0.0f, 0.0f, 0.0f);
	g_camera.SetViewParams(XMLoadFloat3(&eye), XMLoadFloat3(&lookAt));
    g_camera.SetButtonMasks(MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_RIGHT_BUTTON);

    // Init DXUT and create device
	DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
	//DXUTSetIsInGammaCorrectMode( false ); // true by default (SRGB backbuffer), disable to force a RGB backbuffer
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"PhysicsDemo" );
	DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 1280, 960 );
    
	//InitMassSpringSystem();
	InitRigidBodySystem();

	DXUTMainLoop(); // Enter into the DXUT render loop

	DXUTShutdown(); // Shuts down DXUT (includes calls to OnD3D11ReleasingSwapChain() and OnD3D11DestroyDevice())

	return DXUTGetExitCode();
}
