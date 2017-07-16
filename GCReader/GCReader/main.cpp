#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "dwrite.lib")

#include "basewin.h"
#include "SerialClass.h"

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

//Linear map to find the relative position of the analog sticks
INT StickMap(UCHAR val, FLOAT scale) {
	return INT((FLOAT((val * 600) / 256) - 300) / scale);
}

INT TriggerMap(UCHAR val, FLOAT scale) {
	return INT((1026-(FLOAT(val * 1026) / 255)) / scale);
}

class MainWindow : public BaseWindow<MainWindow>
{
	ID2D1Factory				*pFactory;
	ID2D1HwndRenderTarget		*pRenderTarget;
	ID2D1SolidColorBrush		*pBlackBrush;
	ID2D1SolidColorBrush		*pGrayOverlay;
	ID2D1GradientStopCollection *pGradientStops;
	ID2D1LinearGradientBrush	*pLMeterBrush;
	ID2D1LinearGradientBrush	*pRMeterBrush;
	IDWriteTextFormat			*pTextFormat;

	ID2D1Bitmap				*pFullController;
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

	Serial					*pPort;
	BOOL					bDetected = FALSE;
	BOOL					bConnected = FALSE;

	BOOL					bA = FALSE;
	BOOL					bB = FALSE;
	BOOL					bX = FALSE;
	BOOL					bY = FALSE;
	BOOL					bZ = FALSE;
	BOOL					bL = FALSE;
	BOOL					bR = FALSE;
	BOOL					bSTART = FALSE;
	BOOL					bDUp = FALSE;
	BOOL					bDDown = FALSE;
	BOOL					bDLeft = FALSE;
	BOOL					bDRight = FALSE;

	UCHAR					ucJoyX = 128;
	UCHAR					ucJoyY = 128;
	UCHAR					ucCX = 128;
	UCHAR					ucCY = 128;
	UCHAR					ucLeftTrigger = 20;
	UCHAR					ucRightTrigger = 20;

	void    CalculateLayout();
	HRESULT CreateGraphicsResources();
	void    DiscardGraphicsResources();
	void    OnPaint();
	void    Resize();
	void    OnKeyDown(WPARAM wParam);
	void	OnKeyUp(WPARAM wParam);
	void	tPollPort();

	CHAR	buff;

	FLOAT fWidth = 0;
	FLOAT fHeight = 0;
	FLOAT scale = 0;

	D2D1_RECT_F rPosition = D2D1::Rect(0, 250, 2652, 2129);
	D2D1_RECT_F rJoyPosition = D2D1::Rect(0, 250, 2652, 2129);
	D2D1_RECT_F rCPosition = D2D1::Rect(0, 250, 2652, 2129);

	D2D1_RECT_F rLMeter = D2D1::RectF(150, 150, 1176, 250);
	D2D1_RECT_F rRMeter = D2D1::RectF(1476, 150, 2502, 250);
	D2D1_RECT_F rLMeterFill = D2D1::RectF(150, 150, 1176, 250);
	D2D1_RECT_F rRMeterFill = D2D1::RectF(1476, 150, 2502, 250);

public:

	MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBlackBrush(NULL),
		pLMeterBrush(NULL), pRMeterBrush(NULL), pGradientStops(NULL),
		pBlankController(NULL), pAButton(NULL), pBButton(NULL), pStartButton(NULL),
		pXButton(NULL), pYButton(NULL), pLeftTrigger(NULL), pRightTrigger(NULL),
		pZButton(NULL), pDUp(NULL), pDLeft(NULL), pDDown(NULL), pDRight(NULL),
		pJoy(NULL), pCStick(NULL), pPort(NULL), pTextFormat(NULL)
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
		
		fWidth = size.width;
		fHeight = size.height;

		//Handle if scale becomes 0?

		//If the width is greater, scale by the height
		if (fWidth *2129 > fHeight *2652) {

			scale = 2129 / fHeight;
			INT newWidth = INT(2652 / scale);
			INT offset = INT((fWidth - newWidth) / 2); //Calculate offset to center the rectangle
			rPosition = D2D1::Rect(offset, int(250 / scale), newWidth+offset, int(1879 / scale) + int(250 / scale));
			rLMeter = D2D1::RectF((150 / scale) + offset, 150 / scale, (1176 / scale) + offset, 250 / scale);
			rRMeter = D2D1::RectF((1476 / scale) + offset, 150 / scale, (2502 / scale) + offset, 250 / scale);
		}

		//If the height is greater, scale by the width
		else {
			scale = 2652 / fWidth;
			INT newHeight = INT(2129 / scale);
			INT offset = INT((fHeight - newHeight) / 2); //Calculate offset to center the rectangle
			rPosition = D2D1::Rect(0, int(250 / scale) + offset, int(2652 / scale), int(1879 / scale) + int(250 / scale) + offset);
			rLMeter = D2D1::RectF(150 / scale, 150 / scale, 1176 / scale, 250 / scale);
			rRMeter = D2D1::RectF(1476 / scale, 150 / scale, 2502 / scale, 250 / scale);
		}

		INT iJoyXOffset = StickMap(ucJoyX, scale);
		INT iJoyYOffset = StickMap(ucJoyY, scale);
		rJoyPosition = D2D1::Rect(
			rPosition.left + iJoyXOffset, 
			rPosition.top - iJoyYOffset, 
			rPosition.right + iJoyXOffset,
			rPosition.bottom - iJoyYOffset);

		INT iCXOffset = StickMap(ucCX, scale);
		INT iCYOffset = StickMap(ucCY, scale);
		rCPosition = D2D1::Rect(
			rPosition.left + iCXOffset,
			rPosition.top - iCYOffset,
			rPosition.right + iCXOffset,
			rPosition.bottom - iCYOffset);

		INT LOffset = TriggerMap(ucLeftTrigger, scale);
		rLMeterFill = D2D1::Rect(
			rLMeter.left,
			rLMeter.top,
			rLMeter.right - LOffset,
			rLMeter.bottom);

		INT ROffset = TriggerMap(ucRightTrigger, scale);
		rRMeterFill = D2D1::Rect(
			rRMeter.left,
			rRMeter.top,
			rRMeter.right - ROffset,
			rRMeter.bottom);

		pRenderTarget->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(rLMeter.left, 0),
				D2D1::Point2F(rLMeter.right, 0)),
			pGradientStops,
			&pLMeterBrush
		);

		pRenderTarget->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(rRMeter.left, 0),
				D2D1::Point2F(rRMeter.right, 0)),
			pGradientStops,
			&pRMeterBrush
		);

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

		IDWriteFactory *pDWriteFactory;

		if (SUCCEEDED(hr)) {
			hr = DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(pDWriteFactory),
				reinterpret_cast<IUnknown **>(&pDWriteFactory)
			);
		}

		if (SUCCEEDED(hr))
		{
			// Create a DirectWrite text format object.
			hr = pDWriteFactory->CreateTextFormat(
				L"Consolas",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				20.f,
				L"", //locale
				&pTextFormat
			);
		}

		if (SUCCEEDED(hr))
		{
			// Center the text horizontally and vertically.
			pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		}

		if (SUCCEEDED(hr)) {
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black, 1.0f),
				&pBlackBrush
			);
		}

		if (SUCCEEDED(hr)) {
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Gray, .9f),
				&pGrayOverlay
			);
		}

		if (SUCCEEDED(hr)) {

			D2D1_GRADIENT_STOP gradientStops[2];
			gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Red, 1);
			gradientStops[0].position = 0.0f;
			gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::DarkRed, 1);
			gradientStops[1].position = 1.0f;

			hr = pRenderTarget->CreateGradientStopCollection(
				gradientStops,
				2,
				D2D1_GAMMA_2_2,
				D2D1_EXTEND_MODE_CLAMP,
				&pGradientStops
			);

			SafeRelease(&pLMeterBrush);
			if (SUCCEEDED(hr)) {
				hr = pRenderTarget->CreateLinearGradientBrush(
					D2D1::LinearGradientBrushProperties(
						D2D1::Point2F(rLMeter.left, 0),
						D2D1::Point2F(rLMeter.right, 0)),
					pGradientStops,
					&pLMeterBrush
				);
			}

			SafeRelease(&pRMeterBrush);
			if (SUCCEEDED(hr)) {
				hr = pRenderTarget->CreateLinearGradientBrush(
					D2D1::LinearGradientBrushProperties(
						D2D1::Point2F(rRMeter.left, 0),
						D2D1::Point2F(rRMeter.right, 0)),
					pGradientStops,
					&pRMeterBrush
				);
			}
		}

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

				//Load full controller
				LoadBitmapFromFile(
					pRenderTarget,
					pIWICFactory,
					L"images\\gamecube_controller_full.png",
					&pFullController
				);

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
	SafeRelease(&pFullController);
	SafeRelease(&pBlankController);
	SafeRelease(&pBlackBrush);
	SafeRelease(&pGrayOverlay);
	SafeRelease(&pGradientStops);
	SafeRelease(&pLMeterBrush);
	SafeRelease(&pRMeterBrush);
	SafeRelease(&pTextFormat);
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

		if (pBlackBrush != NULL) {
			FLOAT hDiv = fHeight / 11;
			if (hDiv < 1) hDiv = 1;
			for (FLOAT div = 0; div < fWidth; div += hDiv) {
				pRenderTarget->DrawLine(
					D2D1::Point2F(div,0),
					D2D1::Point2F(div,fHeight),
					pBlackBrush);
			}
			for (FLOAT div = 0; div < fHeight; div += hDiv) {
				pRenderTarget->DrawLine(
					D2D1::Point2F(0,div),
					D2D1::Point2F(fWidth,div),
					pBlackBrush);
			}
		}

		//Draw the entire controller

		if (!bConnected || !bDetected) {

			pRenderTarget->DrawBitmap(pFullController, rPosition);

			pRenderTarget->FillRectangle(
				D2D1::RectF(0, 0, fWidth, fHeight),
				pGrayOverlay);

			if (!bDetected) {
				WCHAR text[] = L"Arduino not detected!\nInsert USB or change port.";
				pRenderTarget->DrawText(
					text,
					ARRAYSIZE(text) - 1,
					pTextFormat,
					D2D1::RectF(0, 0, fWidth, fHeight),
					pBlackBrush
				);
			}
			else {
				WCHAR text[] = L"Controller not detected!";
				pRenderTarget->DrawText(
					text,
					ARRAYSIZE(text) - 1,
					pTextFormat,
					D2D1::RectF(0, 0, fWidth, fHeight),
					pBlackBrush
				);
			}
		}

		else {
			
			pRenderTarget->FillRectangle(rLMeterFill, pLMeterBrush);
			pRenderTarget->FillRectangle(rRMeterFill, pRMeterBrush);
			pRenderTarget->DrawRectangle(rLMeter, pBlackBrush, 2.5f);
			pRenderTarget->DrawRectangle(rRMeter, pBlackBrush, 2.5f);

			if (pLeftTrigger != NULL) {
				pRenderTarget->DrawBitmap(pLeftTrigger, rPosition, (bR) ? 1.f : .5f);
			}

			if (pRightTrigger != NULL) {
				pRenderTarget->DrawBitmap(pRightTrigger, rPosition, (bL) ? 1.f : .5f);
			}

			if (pZButton != NULL) {
				pRenderTarget->DrawBitmap(pZButton, rPosition, (bZ) ? 1.f : .5f);
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
				pRenderTarget->DrawBitmap(pStartButton, rPosition, (bSTART) ? 1.f : .5f);
			}

			if (pDUp != NULL) {
				pRenderTarget->DrawBitmap(pDUp, rPosition, (bDUp) ? 1.f : 0.f);
			}

			if (pDDown != NULL) {
				pRenderTarget->DrawBitmap(pDDown, rPosition, (bDDown) ? 1.f : 0.f);
			}

			if (pDLeft != NULL) {
				pRenderTarget->DrawBitmap(pDLeft, rPosition, (bDLeft) ? 1.f : 0.f);
			}

			if (pDRight != NULL) {
				pRenderTarget->DrawBitmap(pDRight, rPosition, (bDRight) ? 1.f : 0.f);
			}

			if (pJoy != NULL) {
				pRenderTarget->DrawBitmap(pJoy, rJoyPosition, 1.f);
			}

			if (pCStick != NULL) {
				pRenderTarget->DrawBitmap(pCStick, rCPosition, 1.f);
			}

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
	case 0x41: //A
		InvalidateRect(m_hwnd, NULL, FALSE);
		bA = FALSE;
		break;
	case 0x42: //B
		InvalidateRect(m_hwnd, NULL, FALSE);
		bB = FALSE;
		break;
	case 0x58: //X
		InvalidateRect(m_hwnd, NULL, FALSE);
		bX = FALSE;
		break;
	case 0x59: //Y
		InvalidateRect(m_hwnd, NULL, FALSE);
		bY = FALSE;
		break;
	}
}

void MainWindow::tPollPort() {

	//Attempt to open port
	if (pPort == NULL) {
		pPort = new Serial(L"COM3");
	}

	if (pPort->IsConnected()) {

		bDetected = TRUE;

		CHAR msg = 'A';

		if (pPort->WriteData(&msg, 1)) {

			UCHAR ucIncomingData[256] = "";
			INT iDataLength = 255;
			INT iReadResult = 0;

			iReadResult = pPort->ReadData(ucIncomingData, iDataLength);
			ucIncomingData[iReadResult] = 0;

			if (ucIncomingData[0] == 0xFF || ucIncomingData[0] == 0x00) {
				bConnected = FALSE;
			}
			else {
				bConnected = TRUE;

				bA = ucIncomingData[0] & 0x01;
				bB = ucIncomingData[0] & 0x02;
				bX = ucIncomingData[0] & 0x04;
				bY = ucIncomingData[0] & 0x08;
				bSTART = ucIncomingData[0] & 0x10;
				bDLeft = ucIncomingData[1] & 0x01;
				bDRight = ucIncomingData[1] & 0x02;
				bDDown = ucIncomingData[1] & 0x04;
				bDUp = ucIncomingData[1] & 0x08;
				bZ = ucIncomingData[1] & 0x10;
				bL = ucIncomingData[1] & 0x20;
				bR = ucIncomingData[1] & 0x40;

				ucJoyX = ucIncomingData[2];
				ucJoyY = ucIncomingData[3];
				ucCX = ucIncomingData[4];
				ucCY = ucIncomingData[5];
				ucLeftTrigger = ucIncomingData[6];
				ucRightTrigger = ucIncomingData[7];

				INT iJoyXOffset = StickMap(ucJoyX, scale);
				INT iJoyYOffset = StickMap(ucJoyY, scale);
				rJoyPosition = D2D1::Rect(
					rPosition.left + iJoyXOffset,
					rPosition.top - iJoyYOffset,
					rPosition.right + iJoyXOffset,
					rPosition.bottom - iJoyYOffset);

				INT iCXOffset = StickMap(ucCX, scale);
				INT iCYOffset = StickMap(ucCY, scale);
				rCPosition = D2D1::Rect(
					rPosition.left + iCXOffset,
					rPosition.top - iCYOffset,
					rPosition.right + iCXOffset,
					rPosition.bottom - iCYOffset);

				INT LOffset = TriggerMap(ucLeftTrigger, scale);
				rLMeterFill = D2D1::Rect(
					rLMeter.left,
					rLMeter.top,
					rLMeter.right - LOffset,
					rLMeter.bottom);

				INT ROffset = TriggerMap(ucRightTrigger, scale);
				rRMeterFill = D2D1::Rect(
					rRMeter.left,
					rRMeter.top,
					rRMeter.right - ROffset,
					rRMeter.bottom);

				InvalidateRect(m_hwnd, NULL, FALSE);
			}
		}
		else { //Write failed. Assuming disconnection
			bDetected = FALSE;
			delete pPort;
			pPort = NULL;
		}
	}
	else { //If port was not opened, prepare to try again
		bDetected = FALSE;
		delete pPort;
		pPort = NULL;
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	MainWindow win;

	if (!win.Create(L"GameCube Controller Reader v0.5", WS_OVERLAPPEDWINDOW, 0 , CW_USEDEFAULT, CW_USEDEFAULT, 550, 500))
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
		SetTimer(
			m_hwnd,
			1,
			30,
			(TIMERPROC)NULL
		);
		return 0;

	case WM_DESTROY:
		DiscardGraphicsResources();
		KillTimer(m_hwnd, 72);
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

	case WM_TIMER:
		tPollPort();
		InvalidateRect(m_hwnd, NULL, FALSE);
		return 0;

	case WM_SETCURSOR:
		HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
		if (LOWORD(lParam) == HTCLIENT) {
			SetCursor(hCursor);
			return TRUE;
		}
		break;

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