/** @file Week7-2-TreeBillboardsApp.cpp
 *  @brief Tree Billboarding Demo
 *   Adding Billboarding to our previous Hills, Mountain, Crate, and Wave Demo
 * 
 *   Controls:
 *   Hold the left mouse button down and move the mouse to rotate.
 *   Hold the right mouse button down and move the mouse to zoom in and out.
 *
 *  @author Hooman Salamat
 */
//week7

#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "../../Common/Camera.h"
#include "FrameResource.h"
#include "Waves.h"
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = 3;

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
	RenderItem() = default;

	BoundingBox wallCollider;

	
    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;

};


enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	Count
};

class TreeBillboardsApp : public D3DApp
{
public:
    TreeBillboardsApp(HINSTANCE hInstance);
    TreeBillboardsApp(const TreeBillboardsApp& rhs) = delete;
    TreeBillboardsApp& operator=(const TreeBillboardsApp& rhs) = delete;
    ~TreeBillboardsApp();

    virtual bool Initialize()override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateWaves(const GameTimer& gt); 

	void LoadTextures();
    void BuildRootSignature();
	void BuildDescriptorHeaps();
    void BuildShadersAndInputLayouts();
    void BuildLandGeometry();
    void BuildWavesGeometry();
	void BuildBoxGeometry();
	void BuildTreeSpritesGeometry();
	void BuildDoorGeometry();
	void BuildConeGeometry();
	void BuildCylinderGeometry();
	void BuildPyramidGeometry();
	void BuildWedgeGeometry();
	void BuildTorusGeometry();
	void BuildDiamondGeometry();
	void BuildTriangularPrismGeometry();
	void BuildWallGeometry();
    void BuildPSOs();
    void BuildFrameResources();
    void BuildMaterials();
    void BuildRenderItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

    float GetHillsHeight(float x, float z)const;
    XMFLOAT3 GetHillsNormal(float x, float z)const;

private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    UINT mCbvSrvDescriptorSize = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mStdInputLayout;
	std::vector<D3D12_INPUT_ELEMENT_DESC> mTreeSpriteInputLayout;

    RenderItem* mWavesRitem = nullptr;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	std::unique_ptr<Waves> mWaves;

    PassConstants mMainPassCB;

	//XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	//XMFLOAT4X4 mView = MathHelper::Identity4x4();
	//XMFLOAT4X4 mProj = MathHelper::Identity4x4();

 //   float mTheta = 1.5f*XM_PI;
 //   float mPhi = XM_PIDIV2 - 0.1f;
 //   float mRadius = 50.0f;

	//
	Camera mCamera;



	BoundingBox wallcollider;
	std::vector<BoundingBox> mColliders;

    POINT mLastMousePos;


private:
	// Camera movement
	float mCameraMoveSpeed = 10.0f;  // Camera movement speed (units per second)
	bool mMoveForward = false;
	bool mMoveBackward = false;
	bool mStrafeLeft = false;
	bool mStrafeRight = false;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        TreeBillboardsApp theApp(hInstance);
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

TreeBillboardsApp::TreeBillboardsApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

TreeBillboardsApp::~TreeBillboardsApp()
{
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool TreeBillboardsApp::Initialize()
{
    if(!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    // Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
    mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    mWaves = std::make_unique<Waves>(160, 128/5.2, 1.0f, 0.03f, 4.0f, 2.0f);

	mCamera.SetPosition(55.0f, 4.0f, -65.0f);

	//mCamera.SetPosition(0.0f, 70.0f, 0.0f);
 
	LoadTextures();
    BuildRootSignature();
	BuildDescriptorHeaps();
    BuildShadersAndInputLayouts();
    BuildLandGeometry();
    BuildWavesGeometry();
	BuildBoxGeometry();
	BuildTreeSpritesGeometry();
	BuildDoorGeometry();
	BuildConeGeometry();
	BuildCylinderGeometry();
	BuildPyramidGeometry();
	BuildWedgeGeometry();
	BuildTorusGeometry();
	BuildDiamondGeometry();
	BuildTriangularPrismGeometry();
	BuildWallGeometry();
	BuildMaterials();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}
 
void TreeBillboardsApp::OnResize()
{
    D3DApp::OnResize();

    //// The window resized, so update the aspect ratio and recompute the projection matrix.
    //XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    //XMStoreFloat4x4(&mProj, P);
	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void TreeBillboardsApp::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);
	UpdateCamera(gt);
	mCamera.UpdateViewMatrix();
    // Cycle through the circular frame resource array.
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
    UpdateWaves(gt);
}

void TreeBillboardsApp::Draw(const GameTimer& gt)
{
    auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&mMainPassCB.FogColor, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

    DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTested]);

	mCommandList->SetPipelineState(mPSOs["opaque"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["treeSprites"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites]);

	mCommandList->SetPipelineState(mPSOs["transparent"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Transparent]);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    mCurrFrameResource->Fence = ++mCurrentFence;

    // Add an instruction to the command queue to set a new fence point. 
    // Because we are on the GPU timeline, the new fence point won't be 
    // set until the GPU finishes processing all the commands prior to this Signal().
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void TreeBillboardsApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void TreeBillboardsApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void TreeBillboardsApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
    //if((btnState & MK_LBUTTON) != 0)
    //{
    //    // Make each pixel correspond to a quarter of a degree.
    //    float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
    //    float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

    //    // Update angles based on input to orbit camera around box.
    //    mTheta += dx;
    //    mPhi += dy;

    //    // Restrict the angle mPhi.
    //    mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    //}
    //else if((btnState & MK_RBUTTON) != 0)
    //{
    //    // Make each pixel correspond to 0.2 unit in the scene.
    //    float dx = 0.2f*static_cast<float>(x - mLastMousePos.x);
    //    float dy = 0.2f*static_cast<float>(y - mLastMousePos.y);

    //    // Update the camera radius based on input.
    //    mRadius += dx - dy;

    //    // Restrict the radius.
    //    mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
    //}

    //mLastMousePos.x = x;
    //mLastMousePos.y = y;
}
 
void TreeBillboardsApp::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();
	float moveSpeed = mCameraMoveSpeed;

	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		moveSpeed *= 5.0f;

	XMFLOAT3 currentPos = mCamera.GetPosition3f();
	XMVECTOR displacement = XMVectorZero();

	if (GetAsyncKeyState('W') & 0x8000)
		displacement += mCamera.GetLook() * moveSpeed * dt;
	if (GetAsyncKeyState('S') & 0x8000)
		displacement -= mCamera.GetLook() * moveSpeed * dt;
	if (GetAsyncKeyState('A') & 0x8000)
		displacement -= mCamera.GetRight() * moveSpeed * dt;
	if (GetAsyncKeyState('D') & 0x8000)
		displacement += mCamera.GetRight() * moveSpeed * dt;

	// Calculate new position
	XMVECTOR newPosVec = XMLoadFloat3(&currentPos) + displacement;
	XMFLOAT3 newPos;
	XMStoreFloat3(&newPos, newPosVec);

	// Create a bounding sphere for the camera (radius 0.5 units)
	BoundingSphere cameraSphere;
	cameraSphere.Center = newPos;
	cameraSphere.Radius = 0.5f;

	// Check against all colliders
	bool collision = false;
	for (const auto& collider : mColliders)
	{
		if (collider.Intersects(cameraSphere))
		{
			collision = true;
			break;
		}
	}

	// Update camera position only if no collision
	if (!collision)
	{
		mCamera.SetPosition(newPos.x, newPos.y, newPos.z);
	}
	//const float dt = gt.DeltaTime();
	//float moveSpeed = mCameraMoveSpeed;

	//if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	//	moveSpeed *= 5.0f;  // Double speed when Shift is held


	//if (GetAsyncKeyState('W') & 0x8000)
	//	mCamera.Walk(moveSpeed * dt);

	//if (GetAsyncKeyState('S') & 0x8000)
	//	mCamera.Walk(-moveSpeed * dt);

	//if (GetAsyncKeyState('A') & 0x8000)
	//	mCamera.Strafe(-moveSpeed * dt);

	//if (GetAsyncKeyState('D') & 0x8000)
	//	mCamera.Strafe(moveSpeed * dt);
}
 
void TreeBillboardsApp::UpdateCamera(const GameTimer& gt)
{

}

void TreeBillboardsApp::AnimateMaterials(const GameTimer& gt)
{
	// Scroll the water material texture coordinates.
	auto waterMat = mMaterials["water"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.1f * gt.DeltaTime();
	tv += 0.02f * gt.DeltaTime();

	if(tu >= 1.0f)
		tu -= 1.0f;

	if(tv >= 1.0f)
		tv -= 1.0f;

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = gNumFrameResources;
}

void TreeBillboardsApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for(auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if(e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void TreeBillboardsApp::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for(auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if(mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void TreeBillboardsApp::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();
	//XMMATRIX view = XMLoadFloat4x4(&mView);
	//XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mCamera.GetPosition3f();
	/*mMainPassCB.EyePosW = mEyePos;*/
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	mMainPassCB.FogColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // fog color black

	// randomly changing the light 
	static float lightningTimer = 0.0f;
	static bool lightningActive = false;
	static float lightningDuration = 0.1f; // Duration of each lightning 
	static float lightningCooldown = 3.0f; // lightning flashes

	lightningTimer += gt.DeltaTime();

	if (lightningActive)
	{
		if (lightningTimer >= lightningDuration)
		{
			lightningCooldown = 1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (5.0f - 1.0f)));

			lightningActive = false;
			lightningTimer = 0.0f;
		}
		else
		{
			// Bright white light for lightning
			mMainPassCB.Lights[0].Strength = { 1.0f, 1.0f, 1.0f };
			mMainPassCB.Lights[1].Strength = { 1.0f, 1.0f, 1.0f };
			mMainPassCB.Lights[2].Strength = { 1.0f, 1.0f, 1.0f };
			mMainPassCB.Lights[3].Strength = { 1.0f, 1.0f, 1.0f };
		}
	}
	else
	{
		if (lightningTimer >= lightningCooldown)
		{
			lightningActive = true;
			lightningTimer = 0.0f;
		}
		else
		{
			// Normal lighting conditions
			mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
			mMainPassCB.Lights[0].Strength = { 0.15f, 0.15f, 0.0f };
			mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
			mMainPassCB.Lights[1].Strength = { 1.0f, 0.5f, 0.0f };
			mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
			mMainPassCB.Lights[2].Strength = { 1.0f, 1.0f, 0.0f };
			mMainPassCB.Lights[3].Direction = { 0.0f, 0.707f, 0.707f };
			mMainPassCB.Lights[3].Strength = { 1.0f, 0.5f, 0.0f };
		}
	}


	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);


}

void TreeBillboardsApp::UpdateWaves(const GameTimer& gt)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if((mTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, mWaves->RowCount() - 5);
		int j = MathHelper::Rand(4, mWaves->ColumnCount() - 5);

		float r = MathHelper::RandF(0.2f, 0.5f);

		mWaves->Disturb(i, j, r);
	}

	// Update the wave simulation.
	mWaves->Update(gt.DeltaTime());

	// Update the wave vertex buffer with the new solution.
	auto currWavesVB = mCurrFrameResource->WavesVB.get();
	for(int i = 0; i < mWaves->VertexCount(); ++i)
	{
		Vertex v;

		v.Pos = mWaves->Position(i);
		v.Normal = mWaves->Normal(i);
		
		// Derive tex-coords from position by 
		// mapping [-w/2,w/2] --> [0,1]
		v.TexC.x = 0.5f + v.Pos.x / mWaves->Width();
		v.TexC.y = 0.5f - v.Pos.z / mWaves->Depth();

		currWavesVB->CopyData(i, v);
	}

	// Set the dynamic VB of the wave renderitem to the current frame VB.
	mWavesRitem->Geo->VertexBufferGPU = currWavesVB->Resource();
}

void TreeBillboardsApp::LoadTextures()
{
	auto grassTex = std::make_unique<Texture>();
	grassTex->Name = "grassTex";
	grassTex->Filename = L"../../Textures/grass.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), grassTex->Filename.c_str(),
		grassTex->Resource, grassTex->UploadHeap));

	auto waterTex = std::make_unique<Texture>();
	waterTex->Name = "waterTex";
	waterTex->Filename = L"../../Textures/water1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), waterTex->Filename.c_str(),
		waterTex->Resource, waterTex->UploadHeap));

	auto fenceTex = std::make_unique<Texture>();
	fenceTex->Name = "fenceTex";
	fenceTex->Filename = L"../../Textures/diamond1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), fenceTex->Filename.c_str(),
		fenceTex->Resource, fenceTex->UploadHeap));

	auto bricksTex = std::make_unique<Texture>();
	bricksTex->Name = "bricksTex";
	bricksTex->Filename = L"../../Textures/subsea.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), bricksTex->Filename.c_str(),
		bricksTex->Resource, bricksTex->UploadHeap));

	auto treeArrayTex = std::make_unique<Texture>();
	treeArrayTex->Name = "treeArrayTex";
	treeArrayTex->Filename = L"../../Textures/crookTextureArray.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), treeArrayTex->Filename.c_str(),
		treeArrayTex->Resource, treeArrayTex->UploadHeap));

	auto bricks3Tex = std::make_unique<Texture>();
	bricks3Tex->Name = "bricks3Tex";
	bricks3Tex->Filename = L"../../Textures/wall.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), bricks3Tex->Filename.c_str(),
		bricks3Tex->Resource, bricks3Tex->UploadHeap));

	auto woodCrateTex = std::make_unique<Texture>();
	woodCrateTex->Name = "woodCrateTex";
	woodCrateTex->Filename = L"../../Textures/wooddoor.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), woodCrateTex->Filename.c_str(),
		woodCrateTex->Resource, woodCrateTex->UploadHeap));

	auto tileTex = std::make_unique<Texture>();
	tileTex->Name = "tileTex";
	tileTex->Filename = L"../../Textures/wooden_roof.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), tileTex->Filename.c_str(),
		tileTex->Resource, tileTex->UploadHeap));

	auto checkboardTex = std::make_unique<Texture>();
	checkboardTex->Name = "checkboardTex";
	checkboardTex->Filename = L"../../Textures/jerus.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), checkboardTex->Filename.c_str(),
		checkboardTex->Resource, checkboardTex->UploadHeap));

	auto bricks2Tex = std::make_unique<Texture>();
	bricks2Tex->Name = "bricks2Tex";
	bricks2Tex->Filename = L"../../Textures/brick5.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), bricks2Tex->Filename.c_str(),
		bricks2Tex->Resource, bricks2Tex->UploadHeap));

	auto mazeWallTex = std::make_unique<Texture>();
	mazeWallTex->Name = "mazeWallTex";
	mazeWallTex->Filename = L"../../Textures/wall.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), mazeWallTex->Filename.c_str(),
		mazeWallTex->Resource, mazeWallTex->UploadHeap));

	mTextures[grassTex->Name] = std::move(grassTex);
	mTextures[waterTex->Name] = std::move(waterTex);
	mTextures[fenceTex->Name] = std::move(fenceTex);
	mTextures[bricksTex->Name] = std::move(bricksTex);
	mTextures[treeArrayTex->Name] = std::move(treeArrayTex);
	mTextures[bricks3Tex->Name] = std::move(bricks3Tex);
	mTextures[woodCrateTex->Name] = std::move(woodCrateTex);
	mTextures[tileTex->Name] = std::move(tileTex);
	mTextures[checkboardTex->Name] = std::move(checkboardTex);
	mTextures[bricks2Tex->Name] = std::move(bricks2Tex);
	mTextures[mazeWallTex->Name] = std::move(mazeWallTex);

}

void TreeBillboardsApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[1].InitAsConstantBufferView(0);
    slotRootParameter[2].InitAsConstantBufferView(1);
    slotRootParameter[3].InitAsConstantBufferView(2);


	auto staticSamplers = GetStaticSamplers();

    // A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if(errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void TreeBillboardsApp::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 11;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = mTextures["grassTex"]->Resource;
	auto waterTex = mTextures["waterTex"]->Resource;
	auto fenceTex = mTextures["fenceTex"]->Resource;
	auto bricksTex = mTextures["bricksTex"]->Resource;
	auto treeArrayTex = mTextures["treeArrayTex"]->Resource;
	auto bricks3Tex = mTextures["bricks3Tex"]->Resource;
	auto woodCrateTex = mTextures["woodCrateTex"]->Resource;
	auto tileTex = mTextures["tileTex"]->Resource; 
	auto checkboardTex = mTextures["checkboardTex"]->Resource; 
	auto bricks2Tex = mTextures["bricks2Tex"]->Resource; 
	auto mazeWallTex = mTextures["mazeWallTex"]->Resource;


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	md3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = fenceTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(fenceTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = bricksTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(bricksTex.Get(), &srvDesc, hDescriptor);


	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	
	auto desc = treeArrayTex->GetDesc();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	md3dDevice->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = bricks3Tex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	md3dDevice->CreateShaderResourceView(bricks3Tex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = woodCrateTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	md3dDevice->CreateShaderResourceView(woodCrateTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = tileTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(tileTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = checkboardTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(checkboardTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = bricks2Tex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(bricks2Tex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	srvDesc.Format = mazeWallTex->GetDesc().Format;
	md3dDevice->CreateShaderResourceView(mazeWallTex.Get(), &srvDesc, hDescriptor);
	
}

void TreeBillboardsApp::BuildShadersAndInputLayouts()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", defines, "PS", "ps_5_1");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_1");
	
	mShaders["treeSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["treeSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_1");
	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", alphaTestDefines, "PS", "ps_5_1");

    mStdInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

	mTreeSpriteInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void TreeBillboardsApp::BuildLandGeometry()
{
    GeometryGenerator geoGen;
    GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, 50, 50);

    //
    // Extract the vertex elements we are interested and apply the height function to
    // each vertex.  In addition, color the vertices based on their height so we have
    // sandy looking beaches, grassy low hills, and snow mountain peaks.
    //

    std::vector<Vertex> vertices(grid.Vertices.size());
    for(size_t i = 0; i < grid.Vertices.size(); ++i)
    {
        auto& p = grid.Vertices[i].Position;
        vertices[i].Pos = p;
		vertices[i].Pos.y = 0.0f; // Set Y to 0 to make it completely flat
		vertices[i].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
        //vertices[i].Pos.y = GetHillsHeight(p.x, p.z);
        //vertices[i].Normal = GetHillsNormal(p.x, p.z);
		vertices[i].TexC = grid.Vertices[i].TexC;
    }

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

    std::vector<std::uint16_t> indices = grid.GetIndices16();
    const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["landGeo"] = std::move(geo);
}

void TreeBillboardsApp::BuildWavesGeometry()
{
    std::vector<std::uint16_t> indices(3 * mWaves->TriangleCount()); // 3 indices per face
	assert(mWaves->VertexCount() < 0x0000ffff);

    // Iterate over each quad.
    int m = mWaves->RowCount();
    int n = mWaves->ColumnCount();
    int k = 0;
    for(int i = 0; i < m - 1; ++i)
    {
        for(int j = 0; j < n - 1; ++j)
        {
            indices[k] = i*n + j;
            indices[k + 1] = i*n + j + 1;
            indices[k + 2] = (i + 1)*n + j;

            indices[k + 3] = (i + 1)*n + j;
            indices[k + 4] = i*n + j + 1;
            indices[k + 5] = (i + 1)*n + j + 1;

            k += 6; // next quad
        }
    }

	UINT vbByteSize = mWaves->VertexCount()*sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size()*sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	// Set dynamically.
	geo->VertexBufferCPU = nullptr;
	geo->VertexBufferGPU = nullptr;

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["grid"] = submesh;

	mGeometries["waterGeo"] = std::move(geo);
}

void TreeBillboardsApp::BuildBoxGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(15.0f, 8.0f, 15.0f, 3);

	std::vector<Vertex> vertices(box.Vertices.size());
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		auto& p = box.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = box.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["box"] = submesh;

	mGeometries["boxGeo"] = std::move(geo);
}

void TreeBillboardsApp::BuildTreeSpritesGeometry()
{
	//step5
	struct TreeSpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};

	static const int treeCount = 16;
	std::array<TreeSpriteVertex, 16> vertices;
	std::array<XMFLOAT3, 16> treePositions = {
	   XMFLOAT3(20.0f, 55.0f, -10.0f),
	   XMFLOAT3(-20.0f, 0.0f, 10.0f),
	   XMFLOAT3(20.0f, 0.0f, -10.0f),
	   XMFLOAT3(-20.0f, 0.0f, -10.0f),
	   XMFLOAT3(20.0f, 0.0f, 20.0f),
	   XMFLOAT3(-20.0f, 0.0f, 20.0f),
	   XMFLOAT3(20.0f, 0.0f, -20.0f),
	   XMFLOAT3(-20.0f, 0.0f, -20.0f),
	   XMFLOAT3(30.0f, 0.0f, 30.0f),
	   XMFLOAT3(-30.0f, 0.0f, 30.0f),
	   XMFLOAT3(30.0f, 0.0f, -30.0f),
	   XMFLOAT3(-30.0f, 0.0f, -30.0f),
	   XMFLOAT3(40.0f, 0.0f, 40.0f),
	   XMFLOAT3(-40.0f, 0.0f, 40.0f),
	   XMFLOAT3(40.0f, 0.0f, -40.0f),
	   XMFLOAT3(-40.0f, 0.0f, -40.0f)
	};
	for(UINT i = 0; i < treeCount; ++i)
	{
		float x = treePositions[i].x;
		float z = treePositions[i].z;
		float y = GetHillsHeight(x, z);

		// Move tree slightly above land height.
		y += 8.0f;


		//float x = MathHelper::RandF(-45.0f, 45.0f);
		//float z = MathHelper::RandF(-45.0f, 45.0f);
		//float y = GetHillsHeight(x, z);

		//// Move tree slightly above land height.
		//y += 8.0f;

		vertices[i].Pos = XMFLOAT3(x, 10.0f, z);
		vertices[i].Size = XMFLOAT2(20.0f, 20.0f);
	}

	std::array<std::uint16_t, 16> indices =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(TreeSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "treeSpritesGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(TreeSpriteVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["points"] = submesh;

	mGeometries["treeSpritesGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildDoorGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData door = geoGen.CreateDoor(2.0f, 3.3f, 2.0f, 3);

	std::vector<Vertex> vertices(door.Vertices.size());
	for (size_t i = 0; i < door.Vertices.size(); ++i)
	{
		auto& p = door.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = door.Vertices[i].Normal;
		vertices[i].TexC = door.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = door.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "doorGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry boxsubmesh;
	boxsubmesh.IndexCount = (UINT)indices.size();
	boxsubmesh.StartIndexLocation = 0;
	boxsubmesh.BaseVertexLocation = 0;

	geo->DrawArgs["door"] = boxsubmesh;

	mGeometries["doorGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildConeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData cone = geoGen.CreateCone(2.0f, 4.0f, 20, 10); // Bottom radius , Height , Slices , Stacks

	std::vector<Vertex> vertices(cone.Vertices.size());
	for (size_t i = 0; i < cone.Vertices.size(); ++i)
	{
		auto& p = cone.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = cone.Vertices[i].Normal;
		vertices[i].TexC = cone.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = cone.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "coneGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry coneSubmesh;
	coneSubmesh.IndexCount = (UINT)indices.size();
	coneSubmesh.StartIndexLocation = 0;
	coneSubmesh.BaseVertexLocation = 0;

	geo->DrawArgs["cone"] = coneSubmesh;

	mGeometries["coneGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildCylinderGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(2.0f, 2.0f, 8.0f, 20, 10); // Bottom radius, Top radius , Height , Slices , Stacks 

	std::vector<Vertex> vertices(cylinder.Vertices.size());
	for (size_t i = 0; i < cylinder.Vertices.size(); ++i)
	{
		auto& p = cylinder.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = cylinder.Vertices[i].Normal;
		vertices[i].TexC = cylinder.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = cylinder.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "cylinderGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)indices.size();
	cylinderSubmesh.StartIndexLocation = 0;
	cylinderSubmesh.BaseVertexLocation = 0;

	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	mGeometries["cylinderGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildPyramidGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData pyramid = geoGen.CreatePyramid(15.0f, 10.0f); // Base width , Height 

	std::vector<Vertex> vertices(pyramid.Vertices.size());
	for (size_t i = 0; i < pyramid.Vertices.size(); ++i)
	{
		auto& p = pyramid.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = pyramid.Vertices[i].Normal;
		vertices[i].TexC = pyramid.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = pyramid.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "pyramidGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry pyramidSubmesh;
	pyramidSubmesh.IndexCount = (UINT)indices.size();
	pyramidSubmesh.StartIndexLocation = 0;
	pyramidSubmesh.BaseVertexLocation = 0;

	geo->DrawArgs["pyramid"] = pyramidSubmesh;

	mGeometries["pyramidGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildWedgeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData wedge = geoGen.CreateWedge(0.5f, 5.0f, 5.0f); // Width , Height , Depth 

	std::vector<Vertex> vertices(wedge.Vertices.size());
	for (size_t i = 0; i < wedge.Vertices.size(); ++i)
	{
		auto& p = wedge.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = wedge.Vertices[i].Normal;
		vertices[i].TexC = wedge.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = wedge.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "wedgeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry wedgeSubmesh;
	wedgeSubmesh.IndexCount = (UINT)indices.size();
	wedgeSubmesh.StartIndexLocation = 0;
	wedgeSubmesh.BaseVertexLocation = 0;

	geo->DrawArgs["wedge"] = wedgeSubmesh;

	mGeometries["wedgeGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildTorusGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData torus = geoGen.CreateTorus(2.0f, 0.3f, 20, 20); // Radius , Tube radius , Slices0, Stacks

	std::vector<Vertex> vertices(torus.Vertices.size());
	for (size_t i = 0; i < torus.Vertices.size(); ++i)
	{
		auto& p = torus.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = torus.Vertices[i].Normal;
		vertices[i].TexC = torus.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = torus.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "torusGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry torusSubmesh;
	torusSubmesh.IndexCount = (UINT)indices.size();
	torusSubmesh.StartIndexLocation = 0;
	torusSubmesh.BaseVertexLocation = 0;

	geo->DrawArgs["torus"] = torusSubmesh;

	mGeometries["torusGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildDiamondGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData diamond = geoGen.CreateDiamond(4.0f, 2.0f, 0); // Height , Width , No subdivisions

	std::vector<Vertex> vertices(diamond.Vertices.size());
	for (size_t i = 0; i < diamond.Vertices.size(); ++i)
	{
		auto& p = diamond.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = diamond.Vertices[i].Normal;
		vertices[i].TexC = diamond.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = diamond.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "diamondGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry diamondSubmesh;
	diamondSubmesh.IndexCount = (UINT)indices.size();
	diamondSubmesh.StartIndexLocation = 0;
	diamondSubmesh.BaseVertexLocation = 0;

	geo->DrawArgs["diamond"] = diamondSubmesh;

	mGeometries["diamondGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildTriangularPrismGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData prism = geoGen.CreateTriangularPrism(15.0f, 9.0f, 2.0f); // Base width , Height, Depth

	std::vector<Vertex> vertices(prism.Vertices.size());
	for (size_t i = 0; i < prism.Vertices.size(); ++i)
	{
		auto& p = prism.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = prism.Vertices[i].Normal;
		vertices[i].TexC = prism.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = prism.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "prismGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry prismSubmesh;
	prismSubmesh.IndexCount = (UINT)indices.size();
	prismSubmesh.StartIndexLocation = 0;
	prismSubmesh.BaseVertexLocation = 0;

	geo->DrawArgs["prism"] = prismSubmesh;

	mGeometries["prismGeo"] = std::move(geo);
}
void TreeBillboardsApp::BuildWallGeometry()
{



	GeometryGenerator geoGen;
	GeometryGenerator::MeshData wall = geoGen.CreateBox(30.0f, 8.0f, 1.0f, 3); // Width, Height, Depth, subdivisions

	std::vector<Vertex> vertices(wall.Vertices.size());
	for (size_t i = 0; i < wall.Vertices.size(); ++i)
	{
		auto& p = wall.Vertices[i].Position;
		vertices[i].Pos = p;
		vertices[i].Normal = wall.Vertices[i].Normal;
		vertices[i].TexC = wall.Vertices[i].TexC;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);

	std::vector<std::uint16_t> indices = wall.GetIndices16();
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "wallGeo";


	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;



	geo->DrawArgs["wall"] = submesh;

	mGeometries["wallGeo"] = std::move(geo);



}
void TreeBillboardsApp::BuildPSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mStdInputLayout.data(), (UINT)mStdInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()), 
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;

	//there is abug with F2 key that is supposed to turn on the multisampling!
//Set4xMsaaState(true);
	//m4xMsaaState = true;

	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//transparentPsoDesc.BlendState.AlphaToCoverageEnable = true;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));

	//
	// PSO for alpha tested objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["alphaTestedPS"]->GetBufferPointer()),
		mShaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&mPSOs["alphaTested"])));

	//
	// PSO for tree sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = opaquePsoDesc;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteVS"]->GetBufferPointer()),
		mShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteGS"]->GetBufferPointer()),
		mShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpritePS"]->GetBufferPointer()),
		mShaders["treeSpritePS"]->GetBufferSize()
	};
	//step1
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&mPSOs["treeSprites"])));
}

void TreeBillboardsApp::BuildFrameResources()
{
    for(int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            1, (UINT)mAllRitems.size(), (UINT)mMaterials.size(), mWaves->VertexCount()));
    }
}

void TreeBillboardsApp::BuildMaterials()
{
	auto grass = std::make_unique<Material>();
	grass->Name = "grass";
	grass->MatCBIndex = 0;
	grass->DiffuseSrvHeapIndex = 0;
	grass->DiffuseAlbedo = XMFLOAT4(0.5f, 0.4f, 0.2f, 1.0f);
	grass->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	grass->Roughness = 0.125f;

	
	auto water = std::make_unique<Material>();
	water->Name = "water";
	water->MatCBIndex = 1;
	water->DiffuseSrvHeapIndex = 1;
	water->DiffuseAlbedo = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.5f);
	water->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	water->Roughness = 0.0f;

	auto wirefence = std::make_unique<Material>();
	wirefence->Name = "wirefence";
	wirefence->MatCBIndex = 2;
	wirefence->DiffuseSrvHeapIndex = 2;
	wirefence->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	wirefence->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	wirefence->Roughness = 0.25f;

	auto bricks = std::make_unique<Material>();
	bricks->Name = "bricks";
	bricks->MatCBIndex = 3;
	bricks->DiffuseSrvHeapIndex = 3;
	bricks->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks->Roughness = 0.25f;

	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "treeSprites";
	treeSprites->MatCBIndex = 4;
	treeSprites->DiffuseSrvHeapIndex = 4;
	treeSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;

	auto bricks3 = std::make_unique<Material>();
	bricks3->Name = "bricks3";
	bricks3->MatCBIndex = 5;
	bricks3->DiffuseSrvHeapIndex = 5; 
	bricks3->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks3->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	bricks3->Roughness = 0.3f;


	auto woodCrate = std::make_unique<Material>();
	woodCrate->Name = "woodCrate";
	woodCrate->MatCBIndex = 6;
	woodCrate->DiffuseSrvHeapIndex = 6; 
	woodCrate->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	woodCrate->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	woodCrate->Roughness = 0.5f; 

	auto tile = std::make_unique<Material>();
	tile->Name = "tile";
	tile->MatCBIndex = 7; 
	tile->DiffuseSrvHeapIndex = 7;
	tile->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tile->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f); 
	tile->Roughness = 0.3f; 

	
	auto checkboard = std::make_unique<Material>();
	checkboard->Name = "checkboard";
	checkboard->MatCBIndex = 8; 
	checkboard->DiffuseSrvHeapIndex = 8; 
	checkboard->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	checkboard->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f); 
	checkboard->Roughness = 0.5f;

	
	auto bricks2 = std::make_unique<Material>();
	bricks2->Name = "bricks2";
	bricks2->MatCBIndex = 9; 
	bricks2->DiffuseSrvHeapIndex = 9; 
	bricks2->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	bricks2->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f); 
	bricks2->Roughness = 0.4f; 

	auto mazeWall = std::make_unique<Material>();
	mazeWall->Name = "mazeWall";
	mazeWall->MatCBIndex = 10; // Make sure this index is unique and not already used
	mazeWall->DiffuseSrvHeapIndex = 10; // Make sure this index is unique and matches descriptor heap
	mazeWall->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mazeWall->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
	mazeWall->Roughness = 0.3f;

	mMaterials["grass"] = std::move(grass);
	mMaterials["water"] = std::move(water);
	mMaterials["wirefence"] = std::move(wirefence);
	mMaterials["bricks"] = std::move(bricks);
	mMaterials["treeSprites"] = std::move(treeSprites);
	mMaterials["bricks3"] = std::move(bricks3);
	mMaterials["woodCrate"] = std::move(woodCrate);
	mMaterials["tile"] = std::move(tile);
	mMaterials["checkboard"] = std::move(checkboard);
	mMaterials["bricks2"] = std::move(bricks2);
	mMaterials["mazeWall"] = std::move(mazeWall);
}

void TreeBillboardsApp::BuildRenderItems()
{
	// WATER
    auto wavesRitem = std::make_unique<RenderItem>();
    wavesRitem->World = MathHelper::Identity4x4();
	XMMATRIX wavesWorld = XMMatrixTranslation(69.0f, 0.1f, 0.0f);
	XMStoreFloat4x4(&wavesRitem->World, wavesWorld);
	XMStoreFloat4x4(&wavesRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem->ObjCBIndex = 0;
	wavesRitem->Mat = mMaterials["water"].get();
	wavesRitem->Geo = mGeometries["waterGeo"].get();
	wavesRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem->IndexCount = wavesRitem->Geo->DrawArgs["grid"].IndexCount;
	wavesRitem->StartIndexLocation = wavesRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem->BaseVertexLocation = wavesRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

    mWavesRitem = wavesRitem.get();

	mRitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem.get());

	// LAND
    auto gridRitem = std::make_unique<RenderItem>();
    gridRitem->World = MathHelper::Identity4x4();
	XMStoreFloat4x4(&gridRitem->World, XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	XMStoreFloat4x4(&gridRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	gridRitem->ObjCBIndex = 1;
	gridRitem->Mat = mMaterials["grass"].get();
	gridRitem->Geo = mGeometries["landGeo"].get();
	gridRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    gridRitem->IndexCount = gridRitem->Geo->DrawArgs["grid"].IndexCount;
    gridRitem->StartIndexLocation = gridRitem->Geo->DrawArgs["grid"].StartIndexLocation;
    gridRitem->BaseVertexLocation = gridRitem->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(gridRitem.get());

	// WALL
	auto boxRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem->World, XMMatrixTranslation(0.0f, 4.0f, 0.0f));
	boxRitem->ObjCBIndex = 2;
	boxRitem->Mat = mMaterials["bricks2"].get();
	boxRitem->Geo = mGeometries["boxGeo"].get();
	boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(boxRitem.get());


	// TREES
	auto treeSpritesRitem = std::make_unique<RenderItem>();
	treeSpritesRitem->World = MathHelper::Identity4x4();
	treeSpritesRitem->ObjCBIndex = 3;
	treeSpritesRitem->Mat = mMaterials["treeSprites"].get();
	treeSpritesRitem->Geo = mGeometries["treeSpritesGeo"].get();
	//step2
	treeSpritesRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	treeSpritesRitem->IndexCount = treeSpritesRitem->Geo->DrawArgs["points"].IndexCount;
	treeSpritesRitem->StartIndexLocation = treeSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
	treeSpritesRitem->BaseVertexLocation = treeSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites].push_back(treeSpritesRitem.get());

	
	//DOOR
	auto boxRitem2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&boxRitem2->World, XMMatrixTranslation(0.0f, 1.6f, -6.8f));
	boxRitem2->ObjCBIndex = 4;
	boxRitem2->Mat = mMaterials["woodCrate"].get();
	boxRitem2->Geo = mGeometries["doorGeo"].get();
	boxRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem2->IndexCount = boxRitem2->Geo->DrawArgs["door"].IndexCount;
	boxRitem2->StartIndexLocation = boxRitem2->Geo->DrawArgs["door"].StartIndexLocation;
	boxRitem2->BaseVertexLocation = boxRitem2->Geo->DrawArgs["door"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::AlphaTested].push_back(boxRitem2.get());

	
	// CONES
	auto coneRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&coneRitem->World, XMMatrixTranslation(7.0f, 10.0f, 7.0f)); // Position the cone at (3, 1, 0)
	coneRitem->ObjCBIndex = 5; 
	coneRitem->Mat = mMaterials["checkboard"].get(); // "checkboard" material 
	coneRitem->Geo = mGeometries["coneGeo"].get();
	coneRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	coneRitem->IndexCount = coneRitem->Geo->DrawArgs["cone"].IndexCount;
	coneRitem->StartIndexLocation = coneRitem->Geo->DrawArgs["cone"].StartIndexLocation;
	coneRitem->BaseVertexLocation = coneRitem->Geo->DrawArgs["cone"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(coneRitem.get());

	auto coneRitem2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&coneRitem2->World, XMMatrixTranslation(-7.0f, 10.0f, 7.0f)); // Position the cone at (3, 1, 0)
	coneRitem2->ObjCBIndex = 6; 
	coneRitem2->Mat = mMaterials["checkboard"].get(); //  "checkboard" material 
	coneRitem2->Geo = mGeometries["coneGeo"].get();
	coneRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	coneRitem2->IndexCount = coneRitem2->Geo->DrawArgs["cone"].IndexCount;
	coneRitem2->StartIndexLocation = coneRitem2->Geo->DrawArgs["cone"].StartIndexLocation;
	coneRitem2->BaseVertexLocation = coneRitem2->Geo->DrawArgs["cone"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(coneRitem2.get());

	auto coneRitem3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&coneRitem3->World, XMMatrixTranslation(7.0f, 10.0f, -7.0f)); // Position the cone at (3, 1, 0)
	coneRitem3->ObjCBIndex = 7; 
	coneRitem3->Mat = mMaterials["checkboard"].get(); 
	coneRitem3->Geo = mGeometries["coneGeo"].get();
	coneRitem3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	coneRitem3->IndexCount = coneRitem3->Geo->DrawArgs["cone"].IndexCount;
	coneRitem3->StartIndexLocation = coneRitem3->Geo->DrawArgs["cone"].StartIndexLocation;
	coneRitem3->BaseVertexLocation = coneRitem3->Geo->DrawArgs["cone"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(coneRitem3.get());

	auto coneRitem4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&coneRitem4->World, XMMatrixTranslation(-7.0f, 10.0f, -7.0f)); 
	coneRitem4->ObjCBIndex = 8; 
	coneRitem4->Mat = mMaterials["checkboard"].get(); 
	coneRitem4->Geo = mGeometries["coneGeo"].get();
	coneRitem4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	coneRitem4->IndexCount = coneRitem4->Geo->DrawArgs["cone"].IndexCount;
	coneRitem4->StartIndexLocation = coneRitem4->Geo->DrawArgs["cone"].StartIndexLocation;
	coneRitem4->BaseVertexLocation = coneRitem4->Geo->DrawArgs["cone"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(coneRitem4.get());



	// CYLINDER
	auto cylinderRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&cylinderRitem->World, XMMatrixTranslation(7.0f, 4.0f, 7.0f));
	cylinderRitem->ObjCBIndex = 9; 
	cylinderRitem->Mat = mMaterials["bricks"].get(); 
	cylinderRitem->Geo = mGeometries["cylinderGeo"].get();
	cylinderRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	cylinderRitem->IndexCount = cylinderRitem->Geo->DrawArgs["cylinder"].IndexCount;
	cylinderRitem->StartIndexLocation = cylinderRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
	cylinderRitem->BaseVertexLocation = cylinderRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(cylinderRitem.get());

	auto cylinderRitem2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&cylinderRitem2->World, XMMatrixTranslation(-7.0f, 4.0f, 7.0f)); 
	cylinderRitem2->ObjCBIndex = 10;
	cylinderRitem2->Mat = mMaterials["bricks"].get(); 
	cylinderRitem2->Geo = mGeometries["cylinderGeo"].get();
	cylinderRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	cylinderRitem2->IndexCount = cylinderRitem2->Geo->DrawArgs["cylinder"].IndexCount;
	cylinderRitem2->StartIndexLocation = cylinderRitem2->Geo->DrawArgs["cylinder"].StartIndexLocation;
	cylinderRitem2->BaseVertexLocation = cylinderRitem2->Geo->DrawArgs["cylinder"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(cylinderRitem2.get());

	auto cylinderRitem3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&cylinderRitem3->World, XMMatrixTranslation(-7.0f, 4.0f, -7.0f));
	cylinderRitem3->ObjCBIndex = 11; 
	cylinderRitem3->Mat = mMaterials["bricks"].get();
	cylinderRitem3->Geo = mGeometries["cylinderGeo"].get();
	cylinderRitem3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	cylinderRitem3->IndexCount = cylinderRitem3->Geo->DrawArgs["cylinder"].IndexCount;
	cylinderRitem3->StartIndexLocation = cylinderRitem3->Geo->DrawArgs["cylinder"].StartIndexLocation;
	cylinderRitem3->BaseVertexLocation = cylinderRitem3->Geo->DrawArgs["cylinder"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(cylinderRitem3.get());

	auto cylinderRitem4 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&cylinderRitem4->World, XMMatrixTranslation(7.0f, 4.0f, -7.0f)); 
	cylinderRitem4->ObjCBIndex = 12; 
	cylinderRitem4->Mat = mMaterials["bricks"].get(); 
	cylinderRitem4->Geo = mGeometries["cylinderGeo"].get();
	cylinderRitem4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	cylinderRitem4->IndexCount = cylinderRitem4->Geo->DrawArgs["cylinder"].IndexCount;
	cylinderRitem4->StartIndexLocation = cylinderRitem4->Geo->DrawArgs["cylinder"].StartIndexLocation;
	cylinderRitem4->BaseVertexLocation = cylinderRitem4->Geo->DrawArgs["cylinder"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(cylinderRitem4.get());
	

	// PYRAMID
	auto pyramidRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&pyramidRitem->World, XMMatrixTranslation(0.0f, 13.0f, 0.0f)); 
	pyramidRitem->ObjCBIndex = 13; 
	pyramidRitem->Mat = mMaterials["tile"].get(); 
	pyramidRitem->Geo = mGeometries["pyramidGeo"].get();
	pyramidRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	pyramidRitem->IndexCount = pyramidRitem->Geo->DrawArgs["pyramid"].IndexCount;
	pyramidRitem->StartIndexLocation = pyramidRitem->Geo->DrawArgs["pyramid"].StartIndexLocation;
	pyramidRitem->BaseVertexLocation = pyramidRitem->Geo->DrawArgs["pyramid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(pyramidRitem.get());

	// WEDGE
	auto wedgeRitem = std::make_unique<RenderItem>();
	XMMATRIX rotation = XMMatrixRotationY(XMConvertToRadians(180.0f)); // Rotate 180 degrees around Y-axis
	XMMATRIX translation = XMMatrixTranslation(-2.0f, 2.4f, -10.0f);
	XMMATRIX world = rotation * translation;
	XMStoreFloat4x4(&wedgeRitem->World, world);
	wedgeRitem->ObjCBIndex = 14;
	wedgeRitem->Mat = mMaterials["bricks3"].get();
	wedgeRitem->Geo = mGeometries["wedgeGeo"].get();
	wedgeRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wedgeRitem->IndexCount = wedgeRitem->Geo->DrawArgs["wedge"].IndexCount;
	wedgeRitem->StartIndexLocation = wedgeRitem->Geo->DrawArgs["wedge"].StartIndexLocation;
	wedgeRitem->BaseVertexLocation = wedgeRitem->Geo->DrawArgs["wedge"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(wedgeRitem.get());

	auto wedgeRitem2 = std::make_unique<RenderItem>();
	XMMATRIX rotation2 = XMMatrixRotationY(XMConvertToRadians(180.0f)); // Rotate 180 degrees around Y-axis
	XMMATRIX translation2 = XMMatrixTranslation(2.0f, 2.4f, -10.0f);
	XMMATRIX world2 = rotation2 * translation2;
	XMStoreFloat4x4(&wedgeRitem2->World, world2);
	wedgeRitem2->ObjCBIndex = 15;
	wedgeRitem2->Mat = mMaterials["bricks3"].get();
	wedgeRitem2->Geo = mGeometries["wedgeGeo"].get();
	wedgeRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wedgeRitem2->IndexCount = wedgeRitem2->Geo->DrawArgs["wedge"].IndexCount;
	wedgeRitem2->StartIndexLocation = wedgeRitem2->Geo->DrawArgs["wedge"].StartIndexLocation;
	wedgeRitem2->BaseVertexLocation = wedgeRitem2->Geo->DrawArgs["wedge"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(wedgeRitem2.get());



	// TORUS
	auto torusRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&torusRitem->World, XMMatrixTranslation(0.0f, 19.9f, 0.0f)); // Position 
	torusRitem->ObjCBIndex = 16; 
	torusRitem->Mat = mMaterials["bricks"].get(); 
	torusRitem->Geo = mGeometries["torusGeo"].get();
	torusRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	torusRitem->IndexCount = torusRitem->Geo->DrawArgs["torus"].IndexCount;
	torusRitem->StartIndexLocation = torusRitem->Geo->DrawArgs["torus"].StartIndexLocation;
	torusRitem->BaseVertexLocation = torusRitem->Geo->DrawArgs["torus"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(torusRitem.get());

	//DIAMOND
	auto diamondRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&diamondRitem->World, XMMatrixTranslation(0.0f, 20.0f, 0.0f)); // Position 
	diamondRitem->ObjCBIndex = 17; 
	diamondRitem->Mat = mMaterials["wirefence"].get(); // material
	diamondRitem->Geo = mGeometries["diamondGeo"].get();
	diamondRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	diamondRitem->IndexCount = diamondRitem->Geo->DrawArgs["diamond"].IndexCount;
	diamondRitem->StartIndexLocation = diamondRitem->Geo->DrawArgs["diamond"].StartIndexLocation;
	diamondRitem->BaseVertexLocation = diamondRitem->Geo->DrawArgs["diamond"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(diamondRitem.get());


	// PRISM
	auto prismRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&prismRitem->World, XMMatrixTranslation(0.0f, 4.0f, 8.0f)); // Position 
	prismRitem->ObjCBIndex = 18;
	prismRitem->Mat = mMaterials["bricks2"].get();
	prismRitem->Geo = mGeometries["prismGeo"].get();
	prismRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	prismRitem->IndexCount = prismRitem->Geo->DrawArgs["prism"].IndexCount;
	prismRitem->StartIndexLocation = prismRitem->Geo->DrawArgs["prism"].StartIndexLocation;
	prismRitem->BaseVertexLocation = prismRitem->Geo->DrawArgs["prism"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(prismRitem.get());

	//Maze
	// FRONT Wall ONE
	auto wallRitem1 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&wallRitem1->World, XMMatrixTranslation(0.0f, 4.0f, -17.0f)); // Position the wall
	wallRitem1->ObjCBIndex = 19; // Next available index
	wallRitem1->Mat = mMaterials["mazeWall"].get();
	wallRitem1->Geo = mGeometries["wallGeo"].get();
	wallRitem1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wallRitem1->IndexCount = wallRitem1->Geo->DrawArgs["wall"].IndexCount;
	wallRitem1->StartIndexLocation = wallRitem1->Geo->DrawArgs["wall"].StartIndexLocation;
	wallRitem1->BaseVertexLocation = wallRitem1->Geo->DrawArgs["wall"].BaseVertexLocation;
	BoundingBox WallCollider;
	WallCollider.Center = XMFLOAT3(0.0f, 4.0f, -17.0f); // Same position as the wall
	WallCollider.Extents = XMFLOAT3(15.0f, 4.0f, 0.5f); // Half of width (30/2), height (8/2), and depth (1/2)

	// Add the collider to the list
	mColliders.push_back(WallCollider);

		
	// LEFT Wall ONE
	auto wallRitem2 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&wallRitem2->World, XMMatrixScaling(0.3f, 1.0f, 1.0f)*
	XMMatrixRotationY(XMConvertToRadians(90.0f))*  // Rotate 90 degrees to make it a side wall
	XMMatrixTranslation(15.0f, 4.0f, -12.5f));       // Position
	wallRitem2->ObjCBIndex = 20;
	wallRitem2->Mat = mMaterials["mazeWall"].get();
	wallRitem2->Geo = mGeometries["wallGeo"].get();
	wallRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wallRitem2->IndexCount = wallRitem2->Geo->DrawArgs["wall"].IndexCount;
	wallRitem2->StartIndexLocation = wallRitem2->Geo->DrawArgs["wall"].StartIndexLocation;
	wallRitem2->BaseVertexLocation = wallRitem2->Geo->DrawArgs["wall"].BaseVertexLocation;
	BoundingBox leftWallOneCollider; // Renamed for clarity
	leftWallOneCollider.Center = XMFLOAT3(15.0f, 4.0f, -12.5f);
	leftWallOneCollider.Extents = XMFLOAT3(0.5f, 4.0f, 4.5f); // Half the width, height, and depth

	// Add the collider to the list
	mColliders.push_back(leftWallOneCollider);



	// RIGHT WALL
	auto wallRitem3 = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&wallRitem3->World,
	XMMatrixRotationY(XMConvertToRadians(-90.0f))* // Rotate -90 degrees
	XMMatrixTranslation(-15.0f, 4.0f, -2.0f));      // Position
	wallRitem3->ObjCBIndex = 21;
	wallRitem3->Mat = mMaterials["mazeWall"].get();
	wallRitem3->Geo = mGeometries["wallGeo"].get();
	wallRitem3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wallRitem3->IndexCount = wallRitem3->Geo->DrawArgs["wall"].IndexCount;
	wallRitem3->StartIndexLocation = wallRitem3->Geo->DrawArgs["wall"].StartIndexLocation;
	wallRitem3->BaseVertexLocation = wallRitem3->Geo->DrawArgs["wall"].BaseVertexLocation;
	BoundingBox collider3;
	collider3.Center = XMFLOAT3(-15.0f, 4.0f, -2.0f);
	collider3.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f); // Rotated -90Y: Extents swap X/Z
	mColliders.push_back(collider3);

	// wallRitem4: BACK WALL
	auto wallRitem4 = std::make_unique<RenderItem>();
	XMFLOAT3 center4 = XMFLOAT3(-5.0f, 4.0f, 19.0f);
	float scaleX4 = 1.35f;
	XMStoreFloat4x4(&wallRitem4->World, XMMatrixScaling(scaleX4, 1.0f, 1.0f)* XMMatrixTranslation(center4.x, center4.y, center4.z));
	wallRitem4->ObjCBIndex = 22;
	wallRitem4->Mat = mMaterials["mazeWall"].get(); wallRitem4->Geo = mGeometries["wallGeo"].get(); wallRitem4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem4->IndexCount = wallRitem4->Geo->DrawArgs["wall"].IndexCount; wallRitem4->StartIndexLocation = wallRitem4->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem4->BaseVertexLocation = wallRitem4->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem4.get());
	BoundingBox collider4;
	collider4.Center = center4;
	collider4.Extents = XMFLOAT3(15.0f * scaleX4, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider4);


	// wallRitem5: LEFT WALL
	auto wallRitem5 = std::make_unique<RenderItem>();
	XMFLOAT3 center5 = XMFLOAT3(15.0f, 4.0f, 9.8f);
	float scaleX5 = 0.66f;
	XMStoreFloat4x4(&wallRitem5->World, XMMatrixScaling(scaleX5, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center5.x, center5.y, center5.z));
	wallRitem5->ObjCBIndex = 23;
	wallRitem5->Mat = mMaterials["mazeWall"].get(); wallRitem5->Geo = mGeometries["wallGeo"].get(); wallRitem5->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem5->IndexCount = wallRitem5->Geo->DrawArgs["wall"].IndexCount; wallRitem5->StartIndexLocation = wallRitem5->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem5->BaseVertexLocation = wallRitem5->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem5.get());
	BoundingBox collider5;
	collider5.Center = center5;
	collider5.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX5); // Rotated 90Y
	mColliders.push_back(collider5);


	// wallRitem6: RIGHT WALL
	auto wallRitem6 = std::make_unique<RenderItem>();
	XMFLOAT3 center6 = XMFLOAT3(-25.0f, 4.0f, -2.0f);
	float scaleX6 = 1.4f;
	XMStoreFloat4x4(&wallRitem6->World, XMMatrixScaling(scaleX6, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center6.x, center6.y, center6.z));
	wallRitem6->ObjCBIndex = 24;
	wallRitem6->Mat = mMaterials["mazeWall"].get(); wallRitem6->Geo = mGeometries["wallGeo"].get(); wallRitem6->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem6->IndexCount = wallRitem6->Geo->DrawArgs["wall"].IndexCount; wallRitem6->StartIndexLocation = wallRitem6->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem6->BaseVertexLocation = wallRitem6->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem6.get());
	BoundingBox collider6;
	collider6.Center = center6;
	collider6.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX6); // Rotated -90Y
	mColliders.push_back(collider6);



	// wallRitem7: BACK WALL
	auto wallRitem7 = std::make_unique<RenderItem>();
	XMFLOAT3 center7 = XMFLOAT3(-12.0f, 4.0f, -23.0f);
	float scaleX7 = 0.9f;
	XMStoreFloat4x4(&wallRitem7->World, XMMatrixScaling(scaleX7, 1.0f, 1.0f)* XMMatrixTranslation(center7.x, center7.y, center7.z));
	wallRitem7->ObjCBIndex = 25;
	wallRitem7->Mat = mMaterials["mazeWall"].get(); wallRitem7->Geo = mGeometries["wallGeo"].get(); wallRitem7->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem7->IndexCount = wallRitem7->Geo->DrawArgs["wall"].IndexCount; wallRitem7->StartIndexLocation = wallRitem7->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem7->BaseVertexLocation = wallRitem7->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem7.get());
	BoundingBox collider7;
	collider7.Center = center7;
	collider7.Extents = XMFLOAT3(15.0f * scaleX7, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider7);

	// wallRitem8: FRONT WALL
	auto wallRitem8 = std::make_unique<RenderItem>();
	XMFLOAT3 center8 = XMFLOAT3(14.0f, 4.0f, -23.0f);
	float scaleX8 = 0.45f;
	XMStoreFloat4x4(&wallRitem8->World, XMMatrixScaling(scaleX8, 1.0f, 1.0f)* XMMatrixTranslation(center8.x, center8.y, center8.z));
	wallRitem8->ObjCBIndex = 26;
	wallRitem8->Mat = mMaterials["mazeWall"].get(); wallRitem8->Geo = mGeometries["wallGeo"].get(); wallRitem8->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem8->IndexCount = wallRitem8->Geo->DrawArgs["wall"].IndexCount; wallRitem8->StartIndexLocation = wallRitem8->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem8->BaseVertexLocation = wallRitem8->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem8.get());
	BoundingBox collider8;
	collider8.Center = center8;
	collider8.Extents = XMFLOAT3(15.0f * scaleX8, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider8);


	// wallRitem9: LEFT Wall
	auto wallRitem9 = std::make_unique<RenderItem>();
	XMFLOAT3 center9 = XMFLOAT3(21.0f, 4.0f, -15.5f);
	float scaleX9 = 0.5f;
	XMStoreFloat4x4(&wallRitem9->World, XMMatrixScaling(scaleX9, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center9.x, center9.y, center9.z));
	wallRitem9->ObjCBIndex = 27;
	wallRitem9->Mat = mMaterials["mazeWall"].get(); wallRitem9->Geo = mGeometries["wallGeo"].get(); wallRitem9->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem9->IndexCount = wallRitem9->Geo->DrawArgs["wall"].IndexCount; wallRitem9->StartIndexLocation = wallRitem9->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem9->BaseVertexLocation = wallRitem9->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem9.get());
	BoundingBox collider9;
	collider9.Center = center9;
	collider9.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX9); // Rotated 90Y
	mColliders.push_back(collider9);

	// wallRitem10: FRONT WALL
	auto wallRitem10 = std::make_unique<RenderItem>();
	XMFLOAT3 center10 = XMFLOAT3(18.0f, 4.0f, -8.5f);
	float scaleX10 = 0.2f;
	XMStoreFloat4x4(&wallRitem10->World, XMMatrixScaling(scaleX10, 1.0f, 1.0f)* XMMatrixTranslation(center10.x, center10.y, center10.z));
	wallRitem10->ObjCBIndex = 28;
	wallRitem10->Mat = mMaterials["mazeWall"].get(); wallRitem10->Geo = mGeometries["wallGeo"].get(); wallRitem10->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem10->IndexCount = wallRitem10->Geo->DrawArgs["wall"].IndexCount; wallRitem10->StartIndexLocation = wallRitem10->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem10->BaseVertexLocation = wallRitem10->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem10.get());
	BoundingBox collider10;
	collider10.Center = center10;
	collider10.Extents = XMFLOAT3(15.0f * scaleX10, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider10);

	// wallRitem11: FRONT WALL
	auto wallRitem11 = std::make_unique<RenderItem>();
	XMFLOAT3 center11 = XMFLOAT3(20.5f, 4.0f, -0.5f);
	float scaleX11 = 0.4f;
	XMStoreFloat4x4(&wallRitem11->World, XMMatrixScaling(scaleX11, 1.0f, 1.0f)* XMMatrixTranslation(center11.x, center11.y, center11.z));
	wallRitem11->ObjCBIndex = 29;
	wallRitem11->Mat = mMaterials["mazeWall"].get(); wallRitem11->Geo = mGeometries["wallGeo"].get(); wallRitem11->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem11->IndexCount = wallRitem11->Geo->DrawArgs["wall"].IndexCount; wallRitem11->StartIndexLocation = wallRitem11->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem11->BaseVertexLocation = wallRitem11->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem11.get());
	BoundingBox collider11;
	collider11.Center = center11;
	collider11.Extents = XMFLOAT3(15.0f * scaleX11, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider11);


	// wallRitem12: LEFT Wall
	auto wallRitem12 = std::make_unique<RenderItem>();
	XMFLOAT3 center12 = XMFLOAT3(21.0f, 4.0f, 12.0f);
	float scaleX12 = 0.5f;
	XMStoreFloat4x4(&wallRitem12->World, XMMatrixScaling(scaleX12, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center12.x, center12.y, center12.z));
	wallRitem12->ObjCBIndex = 30;
	wallRitem12->Mat = mMaterials["mazeWall"].get(); wallRitem12->Geo = mGeometries["wallGeo"].get(); wallRitem12->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem12->IndexCount = wallRitem12->Geo->DrawArgs["wall"].IndexCount; wallRitem12->StartIndexLocation = wallRitem12->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem12->BaseVertexLocation = wallRitem12->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem12.get());
	BoundingBox collider12;
	collider12.Center = center12;
	collider12.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX12); // Rotated 90Y
	mColliders.push_back(collider12);

	// wallRitem13: LEFT Wall
	auto wallRitem13 = std::make_unique<RenderItem>();
	XMFLOAT3 center13 = XMFLOAT3(27.0f, 4.0f, 9.5f);
	float scaleX13 = 1.2f;
	XMStoreFloat4x4(&wallRitem13->World, XMMatrixScaling(scaleX13, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center13.x, center13.y, center13.z));
	wallRitem13->ObjCBIndex = 31;
	wallRitem13->Mat = mMaterials["mazeWall"].get(); wallRitem13->Geo = mGeometries["wallGeo"].get(); wallRitem13->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem13->IndexCount = wallRitem13->Geo->DrawArgs["wall"].IndexCount; wallRitem13->StartIndexLocation = wallRitem13->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem13->BaseVertexLocation = wallRitem13->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem13.get());
	BoundingBox collider13;
	collider13.Center = center13;
	collider13.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX13); // Rotated 90Y
	mColliders.push_back(collider13);


	// wallRitem14: FRONT WALL
	auto wallRitem14 = std::make_unique<RenderItem>();
	XMFLOAT3 center14 = XMFLOAT3(39.0f, 4.0f, -15.0f);
	float scaleX14 = 1.2f;
	XMStoreFloat4x4(&wallRitem14->World, XMMatrixScaling(scaleX14, 1.0f, 1.0f)* XMMatrixTranslation(center14.x, center14.y, center14.z));
	wallRitem14->ObjCBIndex = 32;
	// ... setup ...
	wallRitem14->Mat = mMaterials["mazeWall"].get(); wallRitem14->Geo = mGeometries["wallGeo"].get(); wallRitem14->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem14->IndexCount = wallRitem14->Geo->DrawArgs["wall"].IndexCount; wallRitem14->StartIndexLocation = wallRitem14->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem14->BaseVertexLocation = wallRitem14->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem14.get());
	BoundingBox collider14;
	collider14.Center = center14;
	collider14.Extents = XMFLOAT3(15.0f * scaleX14, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider14);


	// wallRitem15: LEFT Wall
	auto wallRitem15 = std::make_unique<RenderItem>();
	XMFLOAT3 center15 = XMFLOAT3(1.0f, 4.0f, -25.6f);
	float scaleX15 = 0.2f;
	XMStoreFloat4x4(&wallRitem15->World, XMMatrixScaling(scaleX15, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center15.x, center15.y, center15.z));
	wallRitem15->ObjCBIndex = 33;
	wallRitem15->Mat = mMaterials["mazeWall"].get(); wallRitem15->Geo = mGeometries["wallGeo"].get(); wallRitem15->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem15->IndexCount = wallRitem15->Geo->DrawArgs["wall"].IndexCount; wallRitem15->StartIndexLocation = wallRitem15->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem15->BaseVertexLocation = wallRitem15->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem15.get());
	BoundingBox collider15;
	collider15.Center = center15;
	collider15.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX15); // Rotated 90Y
	mColliders.push_back(collider15);

	// wallRitem16: FRONT WALL
	auto wallRitem16 = std::make_unique<RenderItem>();
	XMFLOAT3 center16 = XMFLOAT3(-3.0f, 4.0f, -28.6f);
	float scaleX16 = 2.0f;
	XMStoreFloat4x4(&wallRitem16->World, XMMatrixScaling(scaleX16, 1.0f, 1.0f)* XMMatrixTranslation(center16.x, center16.y, center16.z));
	wallRitem16->ObjCBIndex = 34;
	wallRitem16->Mat = mMaterials["mazeWall"].get(); wallRitem16->Geo = mGeometries["wallGeo"].get(); wallRitem16->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem16->IndexCount = wallRitem16->Geo->DrawArgs["wall"].IndexCount; wallRitem16->StartIndexLocation = wallRitem16->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem16->BaseVertexLocation = wallRitem16->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem16.get());
	BoundingBox collider16;
	collider16.Center = center16;
	collider16.Extents = XMFLOAT3(15.0f * scaleX16, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider16);


	// wallRitem17: LEFT Wall
	auto wallRitem17 = std::make_unique<RenderItem>();
	XMFLOAT3 center17 = XMFLOAT3(27.0f, 4.0f, -28.0f);
	float scaleX17 = 0.45f;
	XMStoreFloat4x4(&wallRitem17->World, XMMatrixScaling(scaleX17, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center17.x, center17.y, center17.z));
	wallRitem17->ObjCBIndex = 35;
	wallRitem17->Mat = mMaterials["mazeWall"].get(); wallRitem17->Geo = mGeometries["wallGeo"].get(); wallRitem17->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem17->IndexCount = wallRitem17->Geo->DrawArgs["wall"].IndexCount; wallRitem17->StartIndexLocation = wallRitem17->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem17->BaseVertexLocation = wallRitem17->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem17.get());
	BoundingBox collider17;
	collider17.Center = center17;
	collider17.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX17); // Rotated 90Y
	mColliders.push_back(collider17);

	// wallRitem18: LEFT Wall
	auto wallRitem18 = std::make_unique<RenderItem>();
	XMFLOAT3 center18 = XMFLOAT3(50.0f, 4.0f, -28.0f);
	float scaleX18 = 0.45f;
	XMStoreFloat4x4(&wallRitem18->World, XMMatrixScaling(scaleX18, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center18.x, center18.y, center18.z));
	wallRitem18->ObjCBIndex = 36;
	wallRitem18->Mat = mMaterials["mazeWall"].get(); wallRitem18->Geo = mGeometries["wallGeo"].get(); wallRitem18->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem18->IndexCount = wallRitem18->Geo->DrawArgs["wall"].IndexCount; wallRitem18->StartIndexLocation = wallRitem18->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem18->BaseVertexLocation = wallRitem18->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem18.get());
	BoundingBox collider18;
	collider18.Center = center18;
	collider18.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX18); // Rotated 90Y
	mColliders.push_back(collider18);

	// wallRitem19: LEFT Wall
	auto wallRitem19 = std::make_unique<RenderItem>();
	XMFLOAT3 center19 = XMFLOAT3(35.0f, 4.0f, -36.0f);
	float scaleX19 = 0.9f;
	XMStoreFloat4x4(&wallRitem19->World, XMMatrixScaling(scaleX19, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center19.x, center19.y, center19.z));
	wallRitem19->ObjCBIndex = 37;
	wallRitem19->Mat = mMaterials["mazeWall"].get(); wallRitem19->Geo = mGeometries["wallGeo"].get(); wallRitem19->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem19->IndexCount = wallRitem19->Geo->DrawArgs["wall"].IndexCount; wallRitem19->StartIndexLocation = wallRitem19->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem19->BaseVertexLocation = wallRitem19->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem19.get());
	BoundingBox collider19;
	collider19.Center = center19;
	collider19.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX19); // Rotated 90Y
	mColliders.push_back(collider19);

	// wallRitem20: LEFT Wall (Boundary)
	auto wallRitem20 = std::make_unique<RenderItem>();
	XMFLOAT3 center20 = XMFLOAT3(58.0f, 4.0f, 0.0f);
	float scaleX20 = 3.9f; // Very long
	XMStoreFloat4x4(&wallRitem20->World, XMMatrixScaling(scaleX20, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center20.x, center20.y, center20.z));
	wallRitem20->ObjCBIndex = 38;
	wallRitem20->Mat = mMaterials["mazeWall"].get(); wallRitem20->Geo = mGeometries["wallGeo"].get(); wallRitem20->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem20->IndexCount = wallRitem20->Geo->DrawArgs["wall"].IndexCount; wallRitem20->StartIndexLocation = wallRitem20->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem20->BaseVertexLocation = wallRitem20->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem20.get());
	BoundingBox collider20;
	collider20.Center = center20;
	collider20.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX20); // Rotated 90Y
	mColliders.push_back(collider20);


	// wallRitem21: BACK WALL (Boundary)
	auto wallRitem21 = std::make_unique<RenderItem>();
	XMFLOAT3 center21 = XMFLOAT3(0.0f, 4.0f, 58.0f);
	float scaleX21 = 3.9f; // Very long
	XMStoreFloat4x4(&wallRitem21->World, XMMatrixScaling(scaleX21, 1.0f, 1.0f)* XMMatrixTranslation(center21.x, center21.y, center21.z));
	wallRitem21->ObjCBIndex = 39;
	wallRitem21->Mat = mMaterials["mazeWall"].get(); wallRitem21->Geo = mGeometries["wallGeo"].get(); wallRitem21->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem21->IndexCount = wallRitem21->Geo->DrawArgs["wall"].IndexCount; wallRitem21->StartIndexLocation = wallRitem21->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem21->BaseVertexLocation = wallRitem21->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem21.get());
	BoundingBox collider21;
	collider21.Center = center21;
	collider21.Extents = XMFLOAT3(15.0f * scaleX21, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider21);

	// wallRitem22: RIGHT WALL (Boundary)
	auto wallRitem22 = std::make_unique<RenderItem>();
	XMFLOAT3 center22 = XMFLOAT3(-58.0f, 4.0f, 0.0f);
	float scaleX22 = 3.9f; // Very long
	XMStoreFloat4x4(&wallRitem22->World, XMMatrixScaling(scaleX22, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center22.x, center22.y, center22.z));
	wallRitem22->ObjCBIndex = 40;
	wallRitem22->Mat = mMaterials["mazeWall"].get(); wallRitem22->Geo = mGeometries["wallGeo"].get(); wallRitem22->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem22->IndexCount = wallRitem22->Geo->DrawArgs["wall"].IndexCount; wallRitem22->StartIndexLocation = wallRitem22->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem22->BaseVertexLocation = wallRitem22->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem22.get());
	BoundingBox collider22;
	collider22.Center = center22;
	collider22.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX22); // Rotated -90Y
	mColliders.push_back(collider22);


	// wallRitem23: FRONTWALL (Boundary)
	auto wallRitem23 = std::make_unique<RenderItem>();
	XMFLOAT3 center23 = XMFLOAT3(-3.5f, 4.0f, -58.0f);
	float scaleX23 = 3.65f; // Very long
	XMStoreFloat4x4(&wallRitem23->World, XMMatrixScaling(scaleX23, 1.0f, 1.0f)* XMMatrixTranslation(center23.x, center23.y, center23.z));
	wallRitem23->ObjCBIndex = 41;
	wallRitem23->Mat = mMaterials["mazeWall"].get(); wallRitem23->Geo = mGeometries["wallGeo"].get(); wallRitem23->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem23->IndexCount = wallRitem23->Geo->DrawArgs["wall"].IndexCount; wallRitem23->StartIndexLocation = wallRitem23->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem23->BaseVertexLocation = wallRitem23->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem23.get());
	BoundingBox collider23;
	collider23.Center = center23;
	collider23.Extents = XMFLOAT3(15.0f * scaleX23, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider23);

	// SECOND WATER (WAVE)
	auto wavesRitem2 = std::make_unique<RenderItem>();
	wavesRitem2->World = MathHelper::Identity4x4();
	XMMATRIX wavesWorld2 = XMMatrixTranslation(-69.0f, 0.1f, 0.0f); // Positioned on the opposite side
	XMStoreFloat4x4(&wavesRitem2->World, wavesWorld2);
	XMStoreFloat4x4(&wavesRitem2->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem2->ObjCBIndex = 42; // Make sure this is unique
	wavesRitem2->Mat = mMaterials["water"].get();
	wavesRitem2->Geo = mGeometries["waterGeo"].get();
	wavesRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem2->IndexCount = wavesRitem2->Geo->DrawArgs["grid"].IndexCount;
	wavesRitem2->StartIndexLocation = wavesRitem2->Geo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem2->BaseVertexLocation = wavesRitem2->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem2.get());

	// THIRD WAVE (WAVE)
	auto wavesRitem3 = std::make_unique<RenderItem>();
	wavesRitem3->World = MathHelper::Identity4x4();
	// Rotate 90 degrees around Y-axis and position it perpendicular to the other waves
	XMMATRIX wavesWorld3 = XMMatrixRotationY(XMConvertToRadians(90.0f)) *
		XMMatrixTranslation(0.0f, 0.1f, 69.0f);
	XMStoreFloat4x4(&wavesRitem3->World, wavesWorld3);
	XMStoreFloat4x4(&wavesRitem3->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem3->ObjCBIndex = 43; // Make sure this is unique
	wavesRitem3->Mat = mMaterials["water"].get();
	wavesRitem3->Geo = mGeometries["waterGeo"].get();
	wavesRitem3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem3->IndexCount = wavesRitem3->Geo->DrawArgs["grid"].IndexCount;
	wavesRitem3->StartIndexLocation = wavesRitem3->Geo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem3->BaseVertexLocation = wavesRitem3->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem3.get());

	// FOURTH WAVE (WAVE)
	auto wavesRitem4 = std::make_unique<RenderItem>();
	wavesRitem4->World = MathHelper::Identity4x4();
	// Rotate 90 degrees around Y-axis and position it perpendicular to the other waves
	XMMATRIX wavesWorld4 = XMMatrixRotationY(XMConvertToRadians(90.0f)) *
		XMMatrixTranslation(0.0f, 0.1f, -69.0f);
	XMStoreFloat4x4(&wavesRitem4->World, wavesWorld4);
	XMStoreFloat4x4(&wavesRitem4->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem4->ObjCBIndex = 44; // Make sure this is unique
	wavesRitem4->Mat = mMaterials["water"].get();
	wavesRitem4->Geo = mGeometries["waterGeo"].get();
	wavesRitem4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem4->IndexCount = wavesRitem4->Geo->DrawArgs["grid"].IndexCount;
	wavesRitem4->StartIndexLocation = wavesRitem4->Geo->DrawArgs["grid"].StartIndexLocation;
	wavesRitem4->BaseVertexLocation = wavesRitem4->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Transparent].push_back(wavesRitem4.get());
	
	// wallRitem24: LEFT Wall
	auto wallRitem24 = std::make_unique<RenderItem>();
	XMFLOAT3 center24 = XMFLOAT3(43.0f, 4.0f, -30.0f);
	float scaleX24 = 1.0f; // No X scale specified, assume 1
	XMStoreFloat4x4(&wallRitem24->World, XMMatrixScaling(scaleX24, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center24.x, center24.y, center24.z));
	wallRitem24->ObjCBIndex = 45;
	wallRitem24->Mat = mMaterials["mazeWall"].get(); wallRitem24->Geo = mGeometries["wallGeo"].get(); wallRitem24->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem24->IndexCount = wallRitem24->Geo->DrawArgs["wall"].IndexCount; wallRitem24->StartIndexLocation = wallRitem24->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem24->BaseVertexLocation = wallRitem24->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem24.get());
	BoundingBox collider24;
	collider24.Center = center24;
	collider24.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX24); // Rotated 90Y
	mColliders.push_back(collider24);


	// wallRitem25: FRONTWALL
	auto wallRitem25 = std::make_unique<RenderItem>();
	XMFLOAT3 center25 = XMFLOAT3(0.0f, 4.0f, -50.0f);
	float scaleX25 = 3.4f;
	XMStoreFloat4x4(&wallRitem25->World, XMMatrixScaling(scaleX25, 1.0f, 1.0f)* XMMatrixTranslation(center25.x, center25.y, center25.z));
	wallRitem25->ObjCBIndex = 46;
	wallRitem25->Mat = mMaterials["mazeWall"].get(); wallRitem25->Geo = mGeometries["wallGeo"].get(); wallRitem25->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem25->IndexCount = wallRitem25->Geo->DrawArgs["wall"].IndexCount; wallRitem25->StartIndexLocation = wallRitem25->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem25->BaseVertexLocation = wallRitem25->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem25.get());
	BoundingBox collider25;
	collider25.Center = center25;
	collider25.Extents = XMFLOAT3(15.0f * scaleX25, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider25);

	// wallRitem26: LEFT Wall
	auto wallRitem26 = std::make_unique<RenderItem>();
	XMFLOAT3 center26 = XMFLOAT3(50.5f, 4.0f, -46.0f);
	float scaleX26 = 0.3f;
	XMStoreFloat4x4(&wallRitem26->World, XMMatrixScaling(scaleX26, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center26.x, center26.y, center26.z));
	wallRitem26->ObjCBIndex =47;
	wallRitem26->Mat = mMaterials["mazeWall"].get(); wallRitem26->Geo = mGeometries["wallGeo"].get(); wallRitem26->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem26->IndexCount = wallRitem26->Geo->DrawArgs["wall"].IndexCount; wallRitem26->StartIndexLocation = wallRitem26->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem26->BaseVertexLocation = wallRitem26->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem26.get());
	BoundingBox collider26;
	collider26.Center = center26;
	collider26.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX26); // Rotated 90Y
	mColliders.push_back(collider26);

	// wallRitem27: FRONTWALL
	auto wallRitem27 = std::make_unique<RenderItem>();
	XMFLOAT3 center27 = XMFLOAT3(46.5f, 4.0f, -34.3f);
	float scaleX27 = 0.2f;
	XMStoreFloat4x4(&wallRitem27->World, XMMatrixScaling(scaleX27, 1.0f, 1.0f)* XMMatrixTranslation(center27.x, center27.y, center27.z));
	wallRitem27->ObjCBIndex = 48;
	wallRitem27->Mat = mMaterials["mazeWall"].get(); wallRitem27->Geo = mGeometries["wallGeo"].get(); wallRitem27->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem27->IndexCount = wallRitem27->Geo->DrawArgs["wall"].IndexCount; wallRitem27->StartIndexLocation = wallRitem27->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem27->BaseVertexLocation = wallRitem27->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem27.get());
	BoundingBox collider27;
	collider27.Center = center27;
	collider27.Extents = XMFLOAT3(15.0f * scaleX27, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider27);

	// wallRitem28: FRONTWALL
	auto wallRitem28 = std::make_unique<RenderItem>();
	XMFLOAT3 center28 = XMFLOAT3(23.0f, 4.0f, -44.0f);
	float scaleX28 = 0.3f;
	XMStoreFloat4x4(&wallRitem28->World, XMMatrixScaling(scaleX28, 1.0f, 1.0f)* XMMatrixTranslation(center28.x, center28.y, center28.z));
	wallRitem28->ObjCBIndex = 49;
	wallRitem28->Mat = mMaterials["mazeWall"].get(); wallRitem28->Geo = mGeometries["wallGeo"].get(); wallRitem28->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem28->IndexCount = wallRitem28->Geo->DrawArgs["wall"].IndexCount; wallRitem28->StartIndexLocation = wallRitem28->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem28->BaseVertexLocation = wallRitem28->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem28.get());
	BoundingBox collider28;
	collider28.Center = center28;
	collider28.Extents = XMFLOAT3(15.0f * scaleX28, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider28);

	// wallRitem29: LEFT Wall
	auto wallRitem29 = std::make_unique<RenderItem>();
	XMFLOAT3 center29 = XMFLOAT3(18.0f, 4.0f, -40.0f);
	float scaleX29 = 0.3f;
	XMStoreFloat4x4(&wallRitem29->World, XMMatrixScaling(scaleX29, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center29.x, center29.y, center29.z));
	wallRitem29->ObjCBIndex = 50;
	wallRitem29->Mat = mMaterials["mazeWall"].get(); wallRitem29->Geo = mGeometries["wallGeo"].get(); wallRitem29->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem29->IndexCount = wallRitem29->Geo->DrawArgs["wall"].IndexCount; wallRitem29->StartIndexLocation = wallRitem29->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem29->BaseVertexLocation = wallRitem29->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem29.get());
	BoundingBox collider29;
	collider29.Center = center29;
	collider29.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX29); // Rotated 90Y
	mColliders.push_back(collider29);

	// wallRitem30: FRONTWALL
	auto wallRitem30 = std::make_unique<RenderItem>();
	XMFLOAT3 center30 = XMFLOAT3(-11.0f, 4.0f, -44.0f);
	float scaleX30 = 1.45f;
	XMStoreFloat4x4(&wallRitem30->World, XMMatrixScaling(scaleX30, 1.0f, 1.0f)* XMMatrixTranslation(center30.x, center30.y, center30.z));
	wallRitem30->ObjCBIndex = 51;
	wallRitem30->Mat = mMaterials["mazeWall"].get(); wallRitem30->Geo = mGeometries["wallGeo"].get(); wallRitem30->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem30->IndexCount = wallRitem30->Geo->DrawArgs["wall"].IndexCount; wallRitem30->StartIndexLocation = wallRitem30->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem30->BaseVertexLocation = wallRitem30->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem30.get());
	BoundingBox collider30;
	collider30.Center = center30;
	collider30.Extents = XMFLOAT3(15.0f * scaleX30, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider30);

	// wallRitem31: FRONTWALL
	auto wallRitem31 = std::make_unique<RenderItem>();
	XMFLOAT3 center31 = XMFLOAT3(-8.0f, 4.0f, -37.0f);
	float scaleX31 = 1.25f;
	XMStoreFloat4x4(&wallRitem31->World, XMMatrixScaling(scaleX31, 1.0f, 1.0f)* XMMatrixTranslation(center31.x, center31.y, center31.z));
	wallRitem31->ObjCBIndex = 52;
	wallRitem31->Mat = mMaterials["mazeWall"].get(); wallRitem31->Geo = mGeometries["wallGeo"].get(); wallRitem31->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem31->IndexCount = wallRitem31->Geo->DrawArgs["wall"].IndexCount; wallRitem31->StartIndexLocation = wallRitem31->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem31->BaseVertexLocation = wallRitem31->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem31.get());
	BoundingBox collider31;
	collider31.Center = center31;
	collider31.Extents = XMFLOAT3(15.0f * scaleX31, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider31);

	// wallRitem32: LEFT Wall
	auto wallRitem32 = std::make_unique<RenderItem>();
	XMFLOAT3 center32 = XMFLOAT3(-32.5f, 4.0f, -36.5f);
	float scaleX32 = 0.51f;
	XMStoreFloat4x4(&wallRitem32->World, XMMatrixScaling(scaleX32, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center32.x, center32.y, center32.z));
	wallRitem32->ObjCBIndex = 53;
	wallRitem32->Mat = mMaterials["mazeWall"].get(); wallRitem32->Geo = mGeometries["wallGeo"].get(); wallRitem32->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem32->IndexCount = wallRitem32->Geo->DrawArgs["wall"].IndexCount; wallRitem32->StartIndexLocation = wallRitem32->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem32->BaseVertexLocation = wallRitem32->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem32.get());
	BoundingBox collider32;
	collider32.Center = center32;
	collider32.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX32); // Rotated 90Y
	mColliders.push_back(collider32);


	// wallRitem33: LEFT Wall
	auto wallRitem33 = std::make_unique<RenderItem>();
	XMFLOAT3 center33 = XMFLOAT3(-50.5f, 4.0f, -36.5f);
	float scaleX33 = 0.51f;
	XMStoreFloat4x4(&wallRitem33->World, XMMatrixScaling(scaleX33, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(90.0f))*
		XMMatrixTranslation(center33.x, center33.y, center33.z));
	wallRitem33->ObjCBIndex = 54;
	wallRitem33->Mat = mMaterials["mazeWall"].get(); wallRitem33->Geo = mGeometries["wallGeo"].get(); wallRitem33->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem33->IndexCount = wallRitem33->Geo->DrawArgs["wall"].IndexCount; wallRitem33->StartIndexLocation = wallRitem33->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem33->BaseVertexLocation = wallRitem33->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem33.get());
	BoundingBox collider33;
	collider33.Center = center33;
	collider33.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX33); // Rotated 90Y
	mColliders.push_back(collider33);

	// wallRitem34: FRONTWALL
	auto wallRitem34 = std::make_unique<RenderItem>();
	XMFLOAT3 center34 = XMFLOAT3(-39.0f, 4.0f, -37.0f);
	float scaleX34 = 0.4f;
	XMStoreFloat4x4(&wallRitem34->World, XMMatrixScaling(scaleX34, 1.0f, 1.0f)* XMMatrixTranslation(center34.x, center34.y, center34.z));
	wallRitem34->ObjCBIndex = 55;
	wallRitem34->Mat = mMaterials["mazeWall"].get(); wallRitem34->Geo = mGeometries["wallGeo"].get(); wallRitem34->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem34->IndexCount = wallRitem34->Geo->DrawArgs["wall"].IndexCount; wallRitem34->StartIndexLocation = wallRitem34->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem34->BaseVertexLocation = wallRitem34->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem34.get());
	BoundingBox collider34;
	collider34.Center = center34;
	collider34.Extents = XMFLOAT3(15.0f * scaleX34, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider34);

	// wallRitem35: FRONTWALL
	auto wallRitem35 = std::make_unique<RenderItem>();
	XMFLOAT3 center35 = XMFLOAT3(-45.0f, 4.0f, -28.5f);
	float scaleX35 = 0.4f;
	XMStoreFloat4x4(&wallRitem35->World, XMMatrixScaling(scaleX35, 1.0f, 1.0f)* XMMatrixTranslation(center35.x, center35.y, center35.z));
	wallRitem35->ObjCBIndex = 56;
	wallRitem35->Mat = mMaterials["mazeWall"].get(); wallRitem35->Geo = mGeometries["wallGeo"].get(); wallRitem35->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem35->IndexCount = wallRitem35->Geo->DrawArgs["wall"].IndexCount; wallRitem35->StartIndexLocation = wallRitem35->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem35->BaseVertexLocation = wallRitem35->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem35.get());
	BoundingBox collider35;
	collider35.Center = center35;
	collider35.Extents = XMFLOAT3(15.0f * scaleX35, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider35);

	// wallRitem36: FRONTWALL
	auto wallRitem36 = std::make_unique<RenderItem>();
	XMFLOAT3 center36 = XMFLOAT3(-45.0f, 4.0f, -44.0f);
	float scaleX36 = 0.4f;
	XMStoreFloat4x4(&wallRitem36->World, XMMatrixScaling(scaleX36, 1.0f, 1.0f)* XMMatrixTranslation(center36.x, center36.y, center36.z));
	wallRitem36->ObjCBIndex = 57;
	wallRitem36->Mat = mMaterials["mazeWall"].get(); wallRitem36->Geo = mGeometries["wallGeo"].get(); wallRitem36->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem36->IndexCount = wallRitem36->Geo->DrawArgs["wall"].IndexCount; wallRitem36->StartIndexLocation = wallRitem36->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem36->BaseVertexLocation = wallRitem36->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem36.get());
	BoundingBox collider36;
	collider36.Center = center36;
	collider36.Extents = XMFLOAT3(15.0f * scaleX36, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider36);


	// wallRitem37: RIGHT WALL
	auto wallRitem37 = std::make_unique<RenderItem>();
	XMFLOAT3 center37 = XMFLOAT3(-33.0f, 4.0f, -1.5f);
	float scaleX37 = 1.45f;
	XMStoreFloat4x4(&wallRitem37->World, XMMatrixScaling(scaleX37, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center37.x, center37.y, center37.z));
	wallRitem37->ObjCBIndex = 58;
	wallRitem37->Mat = mMaterials["mazeWall"].get(); wallRitem37->Geo = mGeometries["wallGeo"].get(); wallRitem37->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem37->IndexCount = wallRitem37->Geo->DrawArgs["wall"].IndexCount; wallRitem37->StartIndexLocation = wallRitem37->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem37->BaseVertexLocation = wallRitem37->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem37.get());
	BoundingBox collider37;
	collider37.Center = center37;
	collider37.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX37); // Rotated -90Y
	mColliders.push_back(collider37);

	// wallRitem38: RIGHT WALL
	auto wallRitem38 = std::make_unique<RenderItem>();
	XMFLOAT3 center38 = XMFLOAT3(-41.0f, 4.0f, -5.0f);
	float scaleX38 = 1.2f;
	XMStoreFloat4x4(&wallRitem38->World, XMMatrixScaling(scaleX38, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center38.x, center38.y, center38.z));
	wallRitem38->ObjCBIndex = 59;
	wallRitem38->Mat = mMaterials["mazeWall"].get(); wallRitem38->Geo = mGeometries["wallGeo"].get(); wallRitem38->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem38->IndexCount = wallRitem38->Geo->DrawArgs["wall"].IndexCount; wallRitem38->StartIndexLocation = wallRitem38->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem38->BaseVertexLocation = wallRitem38->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem38.get());
	BoundingBox collider38;
	collider38.Center = center38;
	collider38.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX38); // Rotated -90Y
	mColliders.push_back(collider38);

	// wallRitem39: RIGHT WALL
	auto wallRitem39 = std::make_unique<RenderItem>();
	XMFLOAT3 center39 = XMFLOAT3(-48.0f, 4.0f, 2.0f);
	float scaleX39 = 1.2f;
	XMStoreFloat4x4(&wallRitem39->World, XMMatrixScaling(scaleX39, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center39.x, center39.y, center39.z));
	wallRitem39->ObjCBIndex = 60;
	wallRitem39->Mat = mMaterials["mazeWall"].get(); wallRitem39->Geo = mGeometries["wallGeo"].get(); wallRitem39->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem39->IndexCount = wallRitem39->Geo->DrawArgs["wall"].IndexCount; wallRitem39->StartIndexLocation = wallRitem39->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem39->BaseVertexLocation = wallRitem39->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem39.get());
	BoundingBox collider39;
	collider39.Center = center39;
	collider39.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX39); // Rotated -90Y
	mColliders.push_back(collider39);


	// wallRitem40: FRONTWALL
	auto wallRitem40 = std::make_unique<RenderItem>();
	XMFLOAT3 center40 = XMFLOAT3(-46.0f, 4.0f, -22.5f);
	float scaleX40 = 0.34f;
	XMStoreFloat4x4(&wallRitem40->World, XMMatrixScaling(scaleX40, 1.0f, 1.0f)* XMMatrixTranslation(center40.x, center40.y, center40.z));
	wallRitem40->ObjCBIndex = 61;
	wallRitem40->Mat = mMaterials["mazeWall"].get(); wallRitem40->Geo = mGeometries["wallGeo"].get(); wallRitem40->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem40->IndexCount = wallRitem40->Geo->DrawArgs["wall"].IndexCount; wallRitem40->StartIndexLocation = wallRitem40->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem40->BaseVertexLocation = wallRitem40->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem40.get());
	BoundingBox collider40;
	collider40.Center = center40;
	collider40.Extents = XMFLOAT3(15.0f * scaleX40, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider40);

	// wallRitem41: FRONTWALL
	auto wallRitem41 = std::make_unique<RenderItem>();
	XMFLOAT3 center41 = XMFLOAT3(-40.5f, 4.0f, 20.0f);
	float scaleX41 = 0.53f;
	XMStoreFloat4x4(&wallRitem41->World, XMMatrixScaling(scaleX41, 1.0f, 1.0f)* XMMatrixTranslation(center41.x, center41.y, center41.z));
	wallRitem41->ObjCBIndex = 62;
	wallRitem41->Mat = mMaterials["mazeWall"].get(); wallRitem41->Geo = mGeometries["wallGeo"].get(); wallRitem41->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem41->IndexCount = wallRitem41->Geo->DrawArgs["wall"].IndexCount; wallRitem41->StartIndexLocation = wallRitem41->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem41->BaseVertexLocation = wallRitem41->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem41.get());
	BoundingBox collider41;
	collider41.Center = center41;
	collider41.Extents = XMFLOAT3(15.0f * scaleX41, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider41);


	// wallRitem42: FRONTWALL
	auto wallRitem42 = std::make_unique<RenderItem>();
	XMFLOAT3 center42 = XMFLOAT3(-11.5f, 4.0f, 29.0f);
	float scaleX42 = 2.6f;
	XMStoreFloat4x4(&wallRitem42->World, XMMatrixScaling(scaleX42, 1.0f, 1.0f)* XMMatrixTranslation(center42.x, center42.y, center42.z));
	wallRitem42->ObjCBIndex = 63;
	wallRitem42->Mat = mMaterials["mazeWall"].get(); wallRitem42->Geo = mGeometries["wallGeo"].get(); wallRitem42->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem42->IndexCount = wallRitem42->Geo->DrawArgs["wall"].IndexCount; wallRitem42->StartIndexLocation = wallRitem42->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem42->BaseVertexLocation = wallRitem42->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem42.get());
	BoundingBox collider42;
	collider42.Center = center42;
	collider42.Extents = XMFLOAT3(15.0f * scaleX42, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider42);

	// wallRitem43: FRONTWALL
	auto wallRitem43 = std::make_unique<RenderItem>();
	XMFLOAT3 center43 = XMFLOAT3(-27.5f, 4.0f, 39.0f);
	float scaleX43 = 2.0f;
	XMStoreFloat4x4(&wallRitem43->World, XMMatrixScaling(scaleX43, 1.0f, 1.0f)* XMMatrixTranslation(center43.x, center43.y, center43.z));
	wallRitem43->ObjCBIndex = 64;
	wallRitem43->Mat = mMaterials["mazeWall"].get(); wallRitem43->Geo = mGeometries["wallGeo"].get(); wallRitem43->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem43->IndexCount = wallRitem43->Geo->DrawArgs["wall"].IndexCount; wallRitem43->StartIndexLocation = wallRitem43->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem43->BaseVertexLocation = wallRitem43->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem43.get());
	BoundingBox collider43;
	collider43.Center = center43;
	collider43.Extents = XMFLOAT3(15.0f * scaleX43, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider43);

	// wallRitem44: FRONTWALL
	auto wallRitem44 = std::make_unique<RenderItem>();
	XMFLOAT3 center44 = XMFLOAT3(-20.5f, 4.0f, 48.0f);
	float scaleX44 = 2.0f;
	XMStoreFloat4x4(&wallRitem44->World, XMMatrixScaling(scaleX44, 1.0f, 1.0f)* XMMatrixTranslation(center44.x, center44.y, center44.z));
	wallRitem44->ObjCBIndex = 65;
	wallRitem44->Mat = mMaterials["mazeWall"].get(); wallRitem44->Geo = mGeometries["wallGeo"].get(); wallRitem44->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem44->IndexCount = wallRitem44->Geo->DrawArgs["wall"].IndexCount; wallRitem44->StartIndexLocation = wallRitem44->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem44->BaseVertexLocation = wallRitem44->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem44.get());
	BoundingBox collider44;
	collider44.Center = center44;
	collider44.Extents = XMFLOAT3(15.0f * scaleX44, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider44);


	// wallRitem45: RIGHT WALL
	auto wallRitem45 = std::make_unique<RenderItem>();
	XMFLOAT3 center45 = XMFLOAT3(9.0f, 4.0f, 38.5f);
	float scaleX45 = 0.65f;
	XMStoreFloat4x4(&wallRitem45->World, XMMatrixScaling(scaleX45, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center45.x, center45.y, center45.z));
	wallRitem45->ObjCBIndex = 66;
	wallRitem45->Mat = mMaterials["mazeWall"].get(); wallRitem45->Geo = mGeometries["wallGeo"].get(); wallRitem45->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem45->IndexCount = wallRitem45->Geo->DrawArgs["wall"].IndexCount; wallRitem45->StartIndexLocation = wallRitem45->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem45->BaseVertexLocation = wallRitem45->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem45.get());
	BoundingBox collider45;
	collider45.Center = center45;
	collider45.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX45); // Rotated -90Y
	mColliders.push_back(collider45);

	// wallRitem46: RIGHT WALL
	auto wallRitem46 = std::make_unique<RenderItem>();
	XMFLOAT3 center46 = XMFLOAT3(27.0f, 4.0f, 37.5f);
	float scaleX46 = 0.7f;
	XMStoreFloat4x4(&wallRitem46->World, XMMatrixScaling(scaleX46, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center46.x, center46.y, center46.z));
	wallRitem46->ObjCBIndex = 67;
	wallRitem46->Mat = mMaterials["mazeWall"].get(); wallRitem46->Geo = mGeometries["wallGeo"].get(); wallRitem46->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem46->IndexCount = wallRitem46->Geo->DrawArgs["wall"].IndexCount; wallRitem46->StartIndexLocation = wallRitem46->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem46->BaseVertexLocation = wallRitem46->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem46.get());
	BoundingBox collider46;
	collider46.Center = center46;
	collider46.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX46); // Rotated -90Y
	mColliders.push_back(collider46);

	// wallRitem47: RIGHT WALL
	auto wallRitem47 = std::make_unique<RenderItem>();
	XMFLOAT3 center47 = XMFLOAT3(18.0f, 4.0f, 47.5f);
	float scaleX47 = 0.7f;
	XMStoreFloat4x4(&wallRitem47->World, XMMatrixScaling(scaleX47, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center47.x, center47.y, center47.z));
	wallRitem47->ObjCBIndex = 68;
	wallRitem47->Mat = mMaterials["mazeWall"].get(); wallRitem47->Geo = mGeometries["wallGeo"].get(); wallRitem47->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem47->IndexCount = wallRitem47->Geo->DrawArgs["wall"].IndexCount; wallRitem47->StartIndexLocation = wallRitem47->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem47->BaseVertexLocation = wallRitem47->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem47.get());
	BoundingBox collider47;
	collider47.Center = center47;
	collider47.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX47); // Rotated -90Y
	mColliders.push_back(collider47);

	// wallRitem48: FRONTWALL
	auto wallRitem48 = std::make_unique<RenderItem>();
	XMFLOAT3 center48 = XMFLOAT3(38.5f, 4.0f, 48.0f);
	float scaleX48 = 0.78f;
	XMStoreFloat4x4(&wallRitem48->World, XMMatrixScaling(scaleX48, 1.0f, 1.0f)* XMMatrixTranslation(center48.x, center48.y, center48.z));
	wallRitem48->ObjCBIndex = 69;
	wallRitem48->Mat = mMaterials["mazeWall"].get(); wallRitem48->Geo = mGeometries["wallGeo"].get(); wallRitem48->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem48->IndexCount = wallRitem48->Geo->DrawArgs["wall"].IndexCount; wallRitem48->StartIndexLocation = wallRitem48->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem48->BaseVertexLocation = wallRitem48->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem48.get());
	BoundingBox collider48;
	collider48.Center = center48;
	collider48.Extents = XMFLOAT3(15.0f * scaleX48, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider48);

	// wallRitem49: FRONTWALL
	auto wallRitem49 = std::make_unique<RenderItem>();
	XMFLOAT3 center49 = XMFLOAT3(46.7f, 4.0f, 38.0f);
	float scaleX49 = 0.76f;
	XMStoreFloat4x4(&wallRitem49->World, XMMatrixScaling(scaleX49, 1.0f, 1.0f)* XMMatrixTranslation(center49.x, center49.y, center49.z));
	wallRitem49->ObjCBIndex = 70;
	wallRitem49->Mat = mMaterials["mazeWall"].get(); wallRitem49->Geo = mGeometries["wallGeo"].get(); wallRitem49->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem49->IndexCount = wallRitem49->Geo->DrawArgs["wall"].IndexCount; wallRitem49->StartIndexLocation = wallRitem49->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem49->BaseVertexLocation = wallRitem49->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem49.get());
	BoundingBox collider49;
	collider49.Center = center49;
	collider49.Extents = XMFLOAT3(15.0f * scaleX49, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider49);


	// wallRitem50: FRONTWALL
	auto wallRitem50 = std::make_unique<RenderItem>();
	XMFLOAT3 center50 = XMFLOAT3(38.5f, 4.0f, 28.0f);
	float scaleX50 = 0.78f;
	XMStoreFloat4x4(&wallRitem50->World, XMMatrixScaling(scaleX50, 1.0f, 1.0f)* XMMatrixTranslation(center50.x, center50.y, center50.z));
	wallRitem50->ObjCBIndex = 71;
	wallRitem50->Mat = mMaterials["mazeWall"].get(); wallRitem50->Geo = mGeometries["wallGeo"].get(); wallRitem50->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem50->IndexCount = wallRitem50->Geo->DrawArgs["wall"].IndexCount; wallRitem50->StartIndexLocation = wallRitem50->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem50->BaseVertexLocation = wallRitem50->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem50.get());
	BoundingBox collider50;
	collider50.Center = center50;
	collider50.Extents = XMFLOAT3(15.0f * scaleX50, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider50);


	// wallRitem51: FRONTWALL
	auto wallRitem51 = std::make_unique<RenderItem>();
	XMFLOAT3 center51 = XMFLOAT3(38.8f, 4.0f, -9.0f);
	float scaleX51 = 0.82f;
	XMStoreFloat4x4(&wallRitem51->World, XMMatrixScaling(scaleX51, 1.0f, 1.0f)* XMMatrixTranslation(center51.x, center51.y, center51.z));
	wallRitem51->ObjCBIndex = 72;
	wallRitem51->Mat = mMaterials["mazeWall"].get(); wallRitem51->Geo = mGeometries["wallGeo"].get(); wallRitem51->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem51->IndexCount = wallRitem51->Geo->DrawArgs["wall"].IndexCount; wallRitem51->StartIndexLocation = wallRitem51->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem51->BaseVertexLocation = wallRitem51->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem51.get());
	BoundingBox collider51;
	collider51.Center = center51;
	collider51.Extents = XMFLOAT3(15.0f * scaleX51, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider51);

	// wallRitem52: FRONTWALL
	auto wallRitem52 = std::make_unique<RenderItem>();
	XMFLOAT3 center52 = XMFLOAT3(45.8f, 4.0f, -2.0f);
	float scaleX52 = 0.82f;
	XMStoreFloat4x4(&wallRitem52->World, XMMatrixScaling(scaleX52, 1.0f, 1.0f)* XMMatrixTranslation(center52.x, center52.y, center52.z));
	wallRitem52->ObjCBIndex = 73;
	wallRitem52->Mat = mMaterials["mazeWall"].get(); wallRitem52->Geo = mGeometries["wallGeo"].get(); wallRitem52->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem52->IndexCount = wallRitem52->Geo->DrawArgs["wall"].IndexCount; wallRitem52->StartIndexLocation = wallRitem52->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem52->BaseVertexLocation = wallRitem52->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem52.get());
	BoundingBox collider52;
	collider52.Center = center52;
	collider52.Extents = XMFLOAT3(15.0f * scaleX52, 4.0f, 0.5f); // Scaled X
	mColliders.push_back(collider52);

	// wallRitem53: RIGHT WALL
	auto wallRitem53 = std::make_unique<RenderItem>();
	XMFLOAT3 center53 = XMFLOAT3(34.0f, 4.0f, 10.0f);
	float scaleX53 = 0.8f;
	XMStoreFloat4x4(&wallRitem53->World, XMMatrixScaling(scaleX53, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center53.x, center53.y, center53.z));
	wallRitem53->ObjCBIndex = 74;
	wallRitem53->Mat = mMaterials["mazeWall"].get(); wallRitem53->Geo = mGeometries["wallGeo"].get(); wallRitem53->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem53->IndexCount = wallRitem53->Geo->DrawArgs["wall"].IndexCount; wallRitem53->StartIndexLocation = wallRitem53->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem53->BaseVertexLocation = wallRitem53->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem53.get());
	BoundingBox collider53;
	collider53.Center = center53;
	collider53.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX53); // Rotated -90Y
	mColliders.push_back(collider53);


	// wallRitem54: RIGHT WALL
	auto wallRitem54 = std::make_unique<RenderItem>();
	XMFLOAT3 center54 = XMFLOAT3(49.8f, 4.0f, 16.0f);
	float scaleX54 = 0.8f;
	XMStoreFloat4x4(&wallRitem54->World, XMMatrixScaling(scaleX54, 1.0f, 1.0f)*
		XMMatrixRotationY(XMConvertToRadians(-90.0f))*
		XMMatrixTranslation(center54.x, center54.y, center54.z));
	wallRitem54->ObjCBIndex = 75;
	wallRitem54->Mat = mMaterials["mazeWall"].get(); wallRitem54->Geo = mGeometries["wallGeo"].get(); wallRitem54->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; wallRitem54->IndexCount = wallRitem54->Geo->DrawArgs["wall"].IndexCount; wallRitem54->StartIndexLocation = wallRitem54->Geo->DrawArgs["wall"].StartIndexLocation; wallRitem54->BaseVertexLocation = wallRitem54->Geo->DrawArgs["wall"].BaseVertexLocation;
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem54.get());
	BoundingBox collider54;
	collider54.Center = center54;
	collider54.Extents = XMFLOAT3(0.5f, 4.0f, 15.0f * scaleX54); // Rotated -90Y
	mColliders.push_back(collider54);


	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem1.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem2.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem3.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem4.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem5.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem6.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem7.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem8.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem9.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem10.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem11.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem12.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem13.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem14.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem15.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem16.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem17.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem18.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem19.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem20.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem21.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem22.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem23.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem24.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem25.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem26.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem27.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem28.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem29.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem30.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem31.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem32.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem33.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem34.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem35.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem36.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem37.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem38.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem39.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem40.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem41.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem42.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem43.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem44.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem45.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem46.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem47.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem48.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem49.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem50.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem51.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem52.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem53.get());
	mRitemLayer[(int)RenderLayer::Opaque].push_back(wallRitem54.get());

    mAllRitems.push_back(std::move(wavesRitem));
	mAllRitems.push_back(std::move(wavesRitem2));
	mAllRitems.push_back(std::move(wavesRitem3));
	mAllRitems.push_back(std::move(wavesRitem4));
    mAllRitems.push_back(std::move(gridRitem));
	mAllRitems.push_back(std::move(boxRitem));
	mAllRitems.push_back(std::move(treeSpritesRitem));
	mAllRitems.push_back(std::move(boxRitem2));
	mAllRitems.push_back(std::move(coneRitem));
	mAllRitems.push_back(std::move(coneRitem2));
	mAllRitems.push_back(std::move(coneRitem3));
	mAllRitems.push_back(std::move(coneRitem4));
	mAllRitems.push_back(std::move(cylinderRitem));
	mAllRitems.push_back(std::move(cylinderRitem2));
	mAllRitems.push_back(std::move(cylinderRitem3));
	mAllRitems.push_back(std::move(cylinderRitem4));
	mAllRitems.push_back(std::move(pyramidRitem));
	mAllRitems.push_back(std::move(wedgeRitem));
	mAllRitems.push_back(std::move(wedgeRitem2));
	mAllRitems.push_back(std::move(torusRitem));
	mAllRitems.push_back(std::move(diamondRitem));
	mAllRitems.push_back(std::move(prismRitem));
	mAllRitems.push_back(std::move(wallRitem1));
	mAllRitems.push_back(std::move(wallRitem2));
	mAllRitems.push_back(std::move(wallRitem3));
	mAllRitems.push_back(std::move(wallRitem4));
	mAllRitems.push_back(std::move(wallRitem5));
	mAllRitems.push_back(std::move(wallRitem6));
	mAllRitems.push_back(std::move(wallRitem7));
	mAllRitems.push_back(std::move(wallRitem8));
	mAllRitems.push_back(std::move(wallRitem9));
	mAllRitems.push_back(std::move(wallRitem10));
	mAllRitems.push_back(std::move(wallRitem11));
	mAllRitems.push_back(std::move(wallRitem12));
	mAllRitems.push_back(std::move(wallRitem13));
	mAllRitems.push_back(std::move(wallRitem14));
	mAllRitems.push_back(std::move(wallRitem15));
	mAllRitems.push_back(std::move(wallRitem16));
	mAllRitems.push_back(std::move(wallRitem17));
	mAllRitems.push_back(std::move(wallRitem18));
	mAllRitems.push_back(std::move(wallRitem19));
	mAllRitems.push_back(std::move(wallRitem20));
	mAllRitems.push_back(std::move(wallRitem21));
	mAllRitems.push_back(std::move(wallRitem22));
	mAllRitems.push_back(std::move(wallRitem23));
	mAllRitems.push_back(std::move(wallRitem24));
	mAllRitems.push_back(std::move(wallRitem25));
	mAllRitems.push_back(std::move(wallRitem26));
	mAllRitems.push_back(std::move(wallRitem27));
	mAllRitems.push_back(std::move(wallRitem28));
	mAllRitems.push_back(std::move(wallRitem29));
	mAllRitems.push_back(std::move(wallRitem30));
	mAllRitems.push_back(std::move(wallRitem31));
	mAllRitems.push_back(std::move(wallRitem32));
	mAllRitems.push_back(std::move(wallRitem33));
	mAllRitems.push_back(std::move(wallRitem34));
	mAllRitems.push_back(std::move(wallRitem35));
	mAllRitems.push_back(std::move(wallRitem36));
	mAllRitems.push_back(std::move(wallRitem37));
	mAllRitems.push_back(std::move(wallRitem38));
	mAllRitems.push_back(std::move(wallRitem39));
	mAllRitems.push_back(std::move(wallRitem40));
	mAllRitems.push_back(std::move(wallRitem41));
	mAllRitems.push_back(std::move(wallRitem42));
	mAllRitems.push_back(std::move(wallRitem43));
	mAllRitems.push_back(std::move(wallRitem44));
	mAllRitems.push_back(std::move(wallRitem45));
	mAllRitems.push_back(std::move(wallRitem46));
	mAllRitems.push_back(std::move(wallRitem47));
	mAllRitems.push_back(std::move(wallRitem48));
	mAllRitems.push_back(std::move(wallRitem49));
	mAllRitems.push_back(std::move(wallRitem50));
	mAllRitems.push_back(std::move(wallRitem51));
	mAllRitems.push_back(std::move(wallRitem52));
	mAllRitems.push_back(std::move(wallRitem53));
	mAllRitems.push_back(std::move(wallRitem54));
}

void TreeBillboardsApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

    // For each render item...
    for(size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];

        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		//step3
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex*objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex*matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
        cmdList->SetGraphicsRootConstantBufferView(20, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> TreeBillboardsApp::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return { 
		pointWrap, pointClamp,
		linearWrap, linearClamp, 
		anisotropicWrap, anisotropicClamp };
}

float TreeBillboardsApp::GetHillsHeight(float x, float z)const
{
    return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

XMFLOAT3 TreeBillboardsApp::GetHillsNormal(float x, float z)const
{
    // n = (-df/dx, 1, -df/dz)
    XMFLOAT3 n(
        -0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
        1.0f,
        -0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

    XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
    XMStoreFloat3(&n, unitNormal);

    return n;
}
