#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "windowscodecs.lib")

#include "basewin.h"

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget	*pRenderTarget,
	IWICImagingFactory	*pIWICFactory,
	PCWSTR				uri,
	ID2D1Bitmap			**ppBitmap
);

class MainWindow : public BaseWindow<MainWindow>
{
	ID2D1Factory            *pFactory;
	ID2D1HwndRenderTarget   *pRenderTarget;

	ID2D1Bitmap				*pBlankController;
	ID2D1Bitmap				*pAButton;
	ID2D1Bitmap				*pBButton;
	ID2D1Bitmap				*pXButton;
	ID2D1Bitmap				*pYButton;
	ID2D1Bitmap				*pStartButton;
	ID2D1Bitmap				*pLeftTrigger;
	ID2D1Bitmap				*pRightTrigger;
	ID2D1Bitmap				*pZButton;
	ID2D1Bitmap				*pDUp;
	ID2D1Bitmap				*pDLeft;
	ID2D1Bitmap				*pDDown;
	ID2D1Bitmap				*pDRight;
	ID2D1Bitmap				*pJoy;
	ID2D1Bitmap				*pCStick;

	BOOL					bA = FALSE;
	BOOL					bB = FALSE;
	BOOL					bX = FALSE;
	BOOL					bY = FALSE;

	void    CalculateLayout();
	HRESULT CreateGraphicsResources();
	void    DiscardGraphicsResources();
	void    OnPaint();
	void    Resize();
	void    OnKeyDown(WPARAM wParam);
	void	OnKeyUp(WPARAM wParam);

public:

	MainWindow() : pFactory(NULL), pRenderTarget(NULL),
		pBlankController(NULL), pAButton(NULL), pBButton(NULL), pStartButton(NULL),
		pXButton(NULL), pYButton(NULL), pLeftTrigger(NULL), pRightTrigger(NULL),
		pZButton(NULL), pDUp(NULL), pDLeft(NULL), pDDown(NULL), pDRight(NULL),
		pJoy(NULL), pCStick(NULL)
	{
	}

	PCWSTR  ClassName() const { return L"Circle Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// Recalculate drawing layout when the size of the window changes.

void MainWindow::CalculateLayout()
{
	if (pRenderTarget != NULL)
	{
		D2D1_SIZE_F size = pRenderTarget->GetSize();
		const float x = size.width / 2;
		const float y = size.height / 2;
		const float radius = min(x, y) - 10;
	}
}

HRESULT MainWindow::CreateGraphicsResources() {

	HRESULT hr = S_OK;
	if (pRenderTarget == NULL) {

		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&pRenderTarget);

		if (SUCCEEDED(hr)) {

			IWICImagingFactory	*pIWICFactory = NULL;

			hr = CoCreateInstance(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IWICImagingFactory,
				(LPVOID*)&pIWICFactory
			);

			if (SUCCEEDED(hr)) {

				//Load blank controller
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\gamecube_controller_blank.png",
					&pBlankController
				);

				//Load A Button
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\a.png",
					&pAButton
				);

				//Load B Button
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\b.png",
					&pBButton
				);

				//Load X Button
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\x.png",
					&pXButton
				);

				//Load Y Button
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\y.png",
					&pYButton
				);

				//Load Start Button
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\start.png",
					&pStartButton
				);

				//Load Left Trigger
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\l.png",
					&pLeftTrigger
				);

				//Load Right Trigger
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\r.png",
					&pRightTrigger
				);

				//Load Z Button
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\z.png",
					&pZButton
				);

				//Load DPAD Up Arrow
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\dup.png",
					&pDUp
				);

				//Load DPAD Down Arrow
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\ddown.png",
					&pDDown
				);

				//Load DPAD Left Arrow
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\dleft.png",
					&pDLeft
				);

				//Load DPAD Right Arrow
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\dright.png",
					&pDRight
				);

				//Load Left Control Stick
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\joy.png",
					&pJoy
				);

				//Load Right C Stick
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\cstick.png",
					&pCStick
				);

				CalculateLayout();
			}
		}
	}
	return hr;
}

void MainWindow::DiscardGraphicsResources()
{
	SafeRelease(&pRenderTarget);
	SafeRelease(&pAButton);
	SafeRelease(&pBButton);
	SafeRelease(&pXButton);
	SafeRelease(&pYButton);
	SafeRelease(&pStartButton);
	SafeRelease(&pLeftTrigger);
	SafeRelease(&pRightTrigger);
	SafeRelease(&pZButton);
	SafeRelease(&pDUp);
	SafeRelease(&pDLeft);
	SafeRelease(&pDDown);
	SafeRelease(&pDRight);
	SafeRelease(&pJoy);
	SafeRelease(&pCStick);
}

void MainWindow::OnPaint()
{
	HRESULT hr = CreateGraphicsResources();
	if (SUCCEEDED(hr))
	{
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);

		pRenderTarget->BeginDraw();

		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		//Draw the entire controller

		D2D1_RECT_F rPosition = D2D1::Rect(0, 50, (2652 / 5), (1879 / 5) + 50);

		if (pLeftTrigger != NULL) {
			pRenderTarget->DrawBitmap(pLeftTrigger, rPosition, 1.f);
		}

		if (pRightTrigger != NULL) {
			pRenderTarget->DrawBitmap(pRightTrigger, rPosition, 1.f);
		}

		if (pZButton != NULL) {
			pRenderTarget->DrawBitmap(pZButton, rPosition, 1.f);
		}

		if (pBlankController != NULL) {
			pRenderTarget->DrawBitmap(pBlankController, rPosition, 1.f);
		}

		if (pAButton != NULL) {
			pRenderTarget->DrawBitmap(pAButton, rPosition, (bA) ? 1.f : .5f);
		}

		if (pBButton != NULL) {
			pRenderTarget->DrawBitmap(pBButton, rPosition, (bB) ? 1.f : .5f);
		}

		if (pXButton != NULL) {
			pRenderTarget->DrawBitmap(pXButton, rPosition, (bX) ? 1.f : .5f);
		}

		if (pYButton != NULL) {
			pRenderTarget->DrawBitmap(pYButton, rPosition, (bY) ? 1.f : .5f);
		}

		if (pStartButton != NULL) {
			pRenderTarget->DrawBitmap(pStartButton, rPosition, 1.f);
		}

		if (pDUp != NULL) {
			pRenderTarget->DrawBitmap(pDUp, rPosition, 1.f);
		}

		if (pDDown != NULL) {
			pRenderTarget->DrawBitmap(pDDown, rPosition, 1.f);
		}

		if (pDLeft != NULL) {
			pRenderTarget->DrawBitmap(pDLeft, rPosition, 1.f);
		}

		if (pDRight != NULL) {
			pRenderTarget->DrawBitmap(pDRight, rPosition, 1.f);
		}

		if (pJoy != NULL) {
			pRenderTarget->DrawBitmap(pJoy, rPosition, 1.f);
		}

		if (pCStick != NULL) {
			pRenderTarget->DrawBitmap(pCStick, rPosition, 1.f);
		}

		hr = pRenderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
		{
			DiscardGraphicsResources();
		}
		EndPaint(m_hwnd, &ps);
	}
}

void MainWindow::Resize()
{
	if (pRenderTarget != NULL)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		pRenderTarget->Resize(size);
		CalculateLayout();
		InvalidateRect(m_hwnd, NULL, FALSE);
	}
}

void MainWindow::OnKeyDown(WPARAM wParam) {
	switch (wParam) {
	case 0x41:
		InvalidateRect(m_hwnd, NULL, FALSE);
		bA = TRUE;
		break;
	case 0x42:
		InvalidateRect(m_hwnd, NULL, FALSE);
		bB = TRUE;
		break;
	case 0x58:
		InvalidateRect(m_hwnd, NULL, FALSE);
		bX = TRUE;
		break;
	case 0x59:
		InvalidateRect(m_hwnd, NULL, FALSE);
		bY = TRUE;
		break;
	}
}

void MainWindow::OnKeyUp(WPARAM wParam) {
	switch (wParam) {
	case 0x41:
		InvalidateRect(m_hwnd, NULL, FALSE);
		bA = FALSE;
		break;
	case 0x42:
		InvalidateRect(m_hwnd, NULL, FALSE);
		bB = FALSE;
		break;
	case 0x58:
		InvalidateRect(m_hwnd, NULL, FALSE);
		bX = FALSE;
		break;
	case 0x59:
		InvalidateRect(m_hwnd, NULL, FALSE);
		bY = FALSE;
		break;
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	MainWindow win;

	if (!win.Create(L"Circle", WS_OVERLAPPEDWINDOW))
	{
		return 0;
	}

	ShowWindow(win.Window(), nCmdShow);

	// Run the message loop.

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		if (FAILED(D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
		{
			return -1;  // Fail CreateWindowEx.
		}
		return 0;

	case WM_DESTROY:
		DiscardGraphicsResources();
		SafeRelease(&pFactory);
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		OnPaint();
		return 0;


	case WM_SIZE:
		Resize();
		return 0;

	case WM_KEYDOWN:
		OnKeyDown(wParam);
		return 0;

	case WM_KEYUP:
		OnKeyUp(wParam);
		return 0;
	}

	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget	*pRenderTarget,
	IWICImagingFactory	*pIWICFactory,
	PCWSTR				uri,
	ID2D1Bitmap			**ppBitmap
)
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);

	if (SUCCEEDED(hr)) {
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr)) {

		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);

	}

	if (SUCCEEDED(hr)) {
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}

	if (SUCCEEDED(hr)) {

		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}