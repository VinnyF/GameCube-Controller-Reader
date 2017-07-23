/*
GCReader v1.0
By Vincent Ferrara
Created July 2017

See the GitHub wiki for full documentation
https://github.com/VinnyF/GameCube-Controller-Reader

This program will visually display the status of a GameCube controller
connected via an Arduino.

A specific program needs to be loaded onto the Arduino in order for this
to work, which can be found on github.

There are a few compiler warnings regarding conversions between different types.
These were mostly intential and shouldn't break anything
*/

#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <dwrite.h>
#include <strsafe.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "dwrite.lib")

//Identifiers for menu items
#define IDM_FILE_QUIT 100
#define IDM_HELP_ABOUT 101
#define IDM_HELP_WIKI 102
#define IDM_TOOLS_CURRENT 103
#define IDM_TOOLS_RAW 104
#define IDM_TOOLS_NONE 0

#include "resource.h"
#include "basewin.h"
#include "SerialClass.h"

//Function for safely deleting graphics resources
template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

//Loads a bitmap from a file to a bitmap pointer
HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget	*pRenderTarget,
	IWICImagingFactory	*pIWICFactory,
	PCWSTR				uri,
	ID2D1Bitmap			**ppBitmap
);

//Creates the menu
HMENU AddMenus(HWND);

//Linear map to find the relative position of the analog sticks
INT StickMap(UCHAR val, FLOAT scale) {
	return INT((FLOAT((val * 600) / 256) - 300) / scale);
}

//Linear map to determine the fill of the trigger meters
INT TriggerMap(UCHAR val, FLOAT scale) {
	return INT((1026-(FLOAT(val * 1026) / 255)) / scale);
}

//Linear map from 0-255 to 0-100
UCHAR ToPercent(UCHAR val) {
	return INT(val * 100) / 255;
}

class MainWindow : public BaseWindow<MainWindow>
{

	//Pointers to graphics resources
	ID2D1Factory				*pFactory;
	ID2D1HwndRenderTarget		*pRenderTarget;
	ID2D1SolidColorBrush		*pBlackBrush;
	ID2D1SolidColorBrush		*pLineBrush;
	ID2D1SolidColorBrush		*pGrayOverlay;
	ID2D1SolidColorBrush		*pWhiteBox;
	ID2D1GradientStopCollection *pGradientStops;
	ID2D1LinearGradientBrush	*pLMeterBrush;
	ID2D1LinearGradientBrush	*pRMeterBrush;
	IDWriteTextFormat			*pErrorText;
	IDWriteTextFormat			*pDataText;

	//Pointers to bitmaps
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

	//Pointer to the serial connection
	Serial					*pPort;

	//Flags for determining the state of the program
	BOOL					bDetected = FALSE;
	BOOL					bConnected = FALSE;

	//Booleans for determining which buttons are pressed
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

	//Unsigned Chars representing the analog values
	UCHAR					ucJoyX = 128;
	UCHAR					ucJoyY = 128;
	UCHAR					ucCX = 128;
	UCHAR					ucCY = 128;
	UCHAR					ucLeftTrigger = 20;
	UCHAR					ucRightTrigger = 20;

	void    CalculateLayout(); //Determines position of graphics on resize
	HRESULT CreateGraphicsResources();
	void    DiscardGraphicsResources();
	void    OnPaint(); //Draws all graphics and text
	void    Resize();
	void    HandleMenu(WPARAM wParam); //Handles incoming menu commands
	void	tPollPort(); //Polls the serial port for a data update

	//Floats holding information about the size of the window
	FLOAT fWidth = 0;
	FLOAT fHeight = 0;
	FLOAT scale = 0;

	//Rectangles to represent the position of the controller images
	D2D1_RECT_F rPosition = D2D1::Rect(0, 250, 2652, 2129);
	D2D1_RECT_F rJoyPosition = D2D1::Rect(0, 250, 2652, 2129);
	D2D1_RECT_F rCPosition = D2D1::Rect(0, 250, 2652, 2129);

	//Rectangles to represent the position of the trigger meters
	D2D1_RECT_F rLMeter = D2D1::RectF(150, 150, 1176, 250);
	D2D1_RECT_F rRMeter = D2D1::RectF(1476, 150, 2502, 250);
	D2D1_RECT_F rLMeterFill = D2D1::RectF(150, 150, 1176, 250);
	D2D1_RECT_F rRMeterFill = D2D1::RectF(1476, 150, 2502, 250);

	//Rectangles to represent the position of the text box
	D2D1_RECT_F rTextBox = D2D1::RectF(50, 1703, 850, 2079);
	D2D1_RECT_F rJX = D2D1::RectF(75, 1703, 850, 1797);
	D2D1_RECT_F rJY = D2D1::RectF(75, 1797, 850, 1891);
	D2D1_RECT_F rCX = D2D1::RectF(75, 1891, 850, 1985);
	D2D1_RECT_F rCY = D2D1::RectF(75, 1985, 850, 2079);

	HMENU hTools; //Handle to the Tools submenu
	BOOL bRaw = FALSE; //Checks whether or not the raw byte should be converted to a percentage
	UCHAR ucPort = 0; //Number representing the port

public:

	//Set all pointers to null
	MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBlackBrush(NULL),
		pLMeterBrush(NULL), pRMeterBrush(NULL), pGradientStops(NULL),
		pBlankController(NULL), pAButton(NULL), pBButton(NULL), pStartButton(NULL),
		pXButton(NULL), pYButton(NULL), pLeftTrigger(NULL), pRightTrigger(NULL),
		pZButton(NULL), pDUp(NULL), pDLeft(NULL), pDDown(NULL), pDRight(NULL),
		pJoy(NULL), pCStick(NULL), pPort(NULL), pErrorText(NULL), pDataText(NULL),
		hTools(NULL), pLineBrush(NULL), pWhiteBox(NULL)
	{
	}

	PCWSTR  ClassName() const { return L"Main Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

//Determine the position of graphics on a resize
void MainWindow::CalculateLayout()
{
	if (pRenderTarget != NULL)
	{
		//Get size of the window object
		D2D1_SIZE_F size = pRenderTarget->GetSize();
		
		fWidth = size.width;
		fHeight = size.height;

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

		//Use the scale to figure out the positions of the other rectangles

		//The text box should be in the bottom left corner
		rTextBox = rTextBox = D2D1::RectF((50 / scale), 1703 / scale, 850 / scale, 2079 / scale);

		//The Joystick bitmap should move relative to the blank controller image
		INT iJoyXOffset = StickMap(ucJoyX, scale);
		INT iJoyYOffset = StickMap(ucJoyY, scale);
		rJoyPosition = D2D1::Rect(
			rPosition.left + iJoyXOffset, 
			rPosition.top - iJoyYOffset, 
			rPosition.right + iJoyXOffset,
			rPosition.bottom - iJoyYOffset);

		//The C-Stick bitmap should move relative to the blank controller image
		INT iCXOffset = StickMap(ucCX, scale);
		INT iCYOffset = StickMap(ucCY, scale);
		rCPosition = D2D1::Rect(
			rPosition.left + iCXOffset,
			rPosition.top - iCYOffset,
			rPosition.right + iCXOffset,
			rPosition.bottom - iCYOffset);

		//The meter should fill from the left, scaled to the size of the full rectangle
		INT LOffset = TriggerMap(ucLeftTrigger, scale);
		rLMeterFill = D2D1::Rect(
			rLMeter.left,
			rLMeter.top,
			rLMeter.right - LOffset,
			rLMeter.bottom);

		//The meter should fill from the left, scaled to the size of the full rectangle
		INT ROffset = TriggerMap(ucRightTrigger, scale);
		rRMeterFill = D2D1::Rect(
			rRMeter.left,
			rRMeter.top,
			rRMeter.right - ROffset,
			rRMeter.bottom);

		//Joystick X text should be in the first 1/4 of the text box
		rJX = D2D1::Rect(
			rTextBox.left + (25 / scale),
			rTextBox.top,
			rTextBox.right,
			rTextBox.bottom - (282 / scale));

		//Joystick Y text should be in the second 1/4 of the text box
		rJY = D2D1::Rect(
			rTextBox.left + (25 / scale),
			rTextBox.top + (94 / scale),
			rTextBox.right,
			rTextBox.bottom - (188 / scale));

		//C-Stick X text should be in the third 1/4 of the text box
		rCX = D2D1::Rect(
			rTextBox.left + (25 / scale),
			rTextBox.top + (188 / scale),
			rTextBox.right,
			rTextBox.bottom - (94 / scale));

		//C-Stick Y text should be in the last 1/4 of the text box
		rCY = D2D1::Rect(
			rTextBox.left + (25 / scale),
			rTextBox.top + (282 / scale),
			rTextBox.right,
			rTextBox.bottom);

		//Recreate the brush for the L-Meter fill based on the position of the left rectangle
		pRenderTarget->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(
				D2D1::Point2F(rLMeter.left, 0),
				D2D1::Point2F(rLMeter.right, 0)),
			pGradientStops,
			&pLMeterBrush
		);

		//Recreate the brush for the R-Meter fill based on the position of the right rectangle
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

	//Check if resources have already been created
	if (pRenderTarget == NULL) {

		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		//Create render target
		hr = pFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&pRenderTarget);

		//Create write factory for drawing text
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
			// Create text format for error text
			hr = pDWriteFactory->CreateTextFormat(
				L"Consolas",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				25.f,
				L"", //locale
				&pErrorText
			);
		}

		if (SUCCEEDED(hr))
		{
			// Center the text horizontally and vertically.
			pErrorText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			pErrorText->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		}

		if (SUCCEEDED(hr))
		{
			// Create text format for the data text
			hr = pDWriteFactory->CreateTextFormat(
				L"Consolas",
				NULL,
				DWRITE_FONT_WEIGHT_BOLD,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				15.f,
				L"", //locale
				&pDataText
			);
		}

		//Create the black brush
		if (SUCCEEDED(hr)) {
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black, 1.f),
				&pBlackBrush
			);
		}

		//Create the slightly grayed line brush
		if (SUCCEEDED(hr)) {
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black, .5f),
				&pLineBrush
			);
		}

		//Create the gray overlay for errors
		if (SUCCEEDED(hr)) {
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Gray, .9f),
				&pGrayOverlay
			);
		}

		//Create a white brush for the text box
		if (SUCCEEDED(hr)) {
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::White, .7f),
				&pWhiteBox
			);
		}

		//Create the gradient brushes for the meter fills
		if (SUCCEEDED(hr)) {

			//Make a gradient from red to dark red
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

			//Create the L-Meter brush based on the L-Rectangle
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

			//Create the R-Meter brush based on the R-Rectangle
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

			//Crearte factory for loading bitmaps
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

//Safely delete graphics pointers
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
	SafeRelease(&pLineBrush);
	SafeRelease(&pGrayOverlay);
	SafeRelease(&pGradientStops);
	SafeRelease(&pLMeterBrush);
	SafeRelease(&pRMeterBrush);
	SafeRelease(&pErrorText);
	SafeRelease(&pDataText);
}

void MainWindow::OnPaint()
{

	//Create grahpics resourcees if necessary
	HRESULT hr = CreateGraphicsResources();

	if (SUCCEEDED(hr))
	{
		//Start painting
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);

		pRenderTarget->BeginDraw();

		//Clear the screen
		pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		//Create the black background grid.
		//Make squares based on the height, extended for the whole screen
		if (pBlackBrush != NULL) {
			FLOAT hDiv = fHeight / 11;
			if (hDiv < 1) hDiv = 1;
			for (FLOAT div = 0; div < fWidth; div += hDiv) {
				pRenderTarget->DrawLine(
					D2D1::Point2F(div,0),
					D2D1::Point2F(div,fHeight),
					pLineBrush);
			}
			for (FLOAT div = 0; div < fHeight; div += hDiv) {
				pRenderTarget->DrawLine(
					D2D1::Point2F(0,div),
					D2D1::Point2F(fWidth,div),
					pLineBrush);
			}
		}

		//If the controller is not connected
		if (!bConnected || !bDetected) {

			//Draw the full controller
			pRenderTarget->DrawBitmap(pFullController, rPosition);

			//Draw the gray overlay
			pRenderTarget->FillRectangle(
				D2D1::RectF(0, 0, fWidth, fHeight),
				pGrayOverlay);

			//Display the proper error text based on the state of the port
			if (!bDetected) {
				WCHAR text[] = L"Arduino not detected!\nInsert USB or change port.";
				pRenderTarget->DrawText(
					text,
					ARRAYSIZE(text) - 1,
					pErrorText,
					D2D1::RectF(0, 0, fWidth, fHeight),
					pBlackBrush
				);
			}
			else {
				WCHAR text[] = L"Controller not detected!";
				pRenderTarget->DrawText(
					text,
					ARRAYSIZE(text) - 1,
					pErrorText,
					D2D1::RectF(0, 0, fWidth, fHeight),
					pBlackBrush
				);
			}
		}

		//Controller is connected
		else {
			
			//Draw the trigger-meter rectangles
			pRenderTarget->FillRectangle(rLMeterFill, pLMeterBrush);
			pRenderTarget->FillRectangle(rRMeterFill, pRMeterBrush);
			pRenderTarget->DrawRectangle(rLMeter, pBlackBrush, 2.5f);
			pRenderTarget->DrawRectangle(rRMeter, pBlackBrush, 2.5f);

			//Draw the rest of the controller
			//For buttons, ternary operator is used. Draws at full opacity if button is pressed, otherwise half opacity
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

			//Draw the text box
			pRenderTarget->FillRectangle(rTextBox, pWhiteBox);
			pRenderTarget->DrawRectangle(rTextBox, pBlackBrush);

			//Set the text alignment to the middle left
			pDataText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			pDataText->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

			//Draw Joystick X text
			WCHAR wJXText[16];
			StringCchPrintf(wJXText, 16, (bRaw ? L"Joystick X: %2d" : L"Joystick X: %2d%%"), (bRaw ? ucJoyX : ToPercent(ucJoyX)));
			pRenderTarget->DrawText(
				wJXText,
				ARRAYSIZE(wJXText) - 1,
				pDataText,
				rJX,
				pBlackBrush
			);

			//Draw Joystick Y text
			WCHAR wJYText[16];
			StringCchPrintf(wJYText, 16, (bRaw ? L"Joystick Y: %2d" : L"Joystick Y: %2d%%"), (bRaw ? ucJoyY : ToPercent(ucJoyY)));
			pRenderTarget->DrawText(
				wJYText,
				ARRAYSIZE(wJYText) - 1,
				pDataText,
				rJY,
				pBlackBrush
			);

			//Draw C-Stick X text
			WCHAR wCXText[15];
			StringCchPrintf(wCXText, 15, (bRaw ? L"C-Stick X: %2d" : L"C-Stick X: %2d%%"), (bRaw ? ucCX : ToPercent(ucCX)));
			pRenderTarget->DrawText(
				wCXText,
				ARRAYSIZE(wCXText) - 1,
				pDataText,
				rCX,
				pBlackBrush
			);

			//Draw C-Stick Y text
			WCHAR wCYText[15];
			StringCchPrintf(wCYText, 15, (bRaw ? L"C-Stick Y: %2d" : L"C-Stick Y: %2d%%"), (bRaw ? ucCY : ToPercent(ucCY)));
			pRenderTarget->DrawText(
				wCYText,
				ARRAYSIZE(wCYText) - 1,
				pDataText,
				rCY,
				pBlackBrush
			);

			//Set the text alignment to the bottom left
			pDataText->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);

			//Draw L-Trigger text
			WCHAR wLText[18];
			StringCchPrintf(wLText, 18, (bRaw ? L"Left Trigger: %2d" : L"Left Trigger: %2d%%"), (bRaw ? ucLeftTrigger : ToPercent(ucLeftTrigger)));
			pRenderTarget->DrawText(
				wLText,
				ARRAYSIZE(wLText) - 1,
				pDataText,
				D2D1::Rect(
					rLMeter.left,
					rLMeter.top + rLMeter.top - rLMeter.bottom,
					rLMeter.right,
					rLMeter.top),
				pBlackBrush
			);

			//Set the text alignment to the bottom right
			pDataText->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
			
			//Draw R-Trigger text
			WCHAR wRText[19];
			StringCchPrintf(wRText, 19, (bRaw ? L"Right Trigger: %2d" : L"Right Trigger: %2d%%"), (bRaw ? ucRightTrigger : ToPercent(ucRightTrigger)));
			pRenderTarget->DrawText(
				wRText,
				ARRAYSIZE(wRText) - 1,
				pDataText,
				D2D1::Rect(
					rRMeter.left,
					rRMeter.top + rRMeter.top - rRMeter.bottom,
					rRMeter.right,
					rRMeter.top),
				pBlackBrush
			);
			
		}

		//Finish painting
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
		//Get new window size
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		//Determine new position of graphics objects
		pRenderTarget->Resize(size);
		CalculateLayout();

		//Repaint window
		InvalidateRect(m_hwnd, NULL, FALSE);
	}
}	

void MainWindow::tPollPort() {

	//Attempt to open port
	if (pPort == NULL) {
		WCHAR wPortName[11];
		StringCchPrintf(wPortName, 11, L"\\\\.\\COM%d", ucPort);
		pPort = new Serial(wPortName);
	}

	if (pPort->IsConnected()) {

		bDetected = TRUE;

		//Ping the port by writing the character 'A'
		//The arduino program should be listening for this message,
		//then respond with controller data
		CHAR msg = 'A';

		if (pPort->WriteData(&msg, 1)) {

			//Prepare an excess amount of space for incoming data
			UCHAR ucIncomingData[32] = "";
			INT iDataLength = 31;
			INT iReadResult = 0;

			//Read the port
			iReadResult = pPort->ReadData(ucIncomingData, iDataLength);

			ucIncomingData[iReadResult] = 0;

			//Verify the data to see if the controller has been connected
			if (ucIncomingData[0] == 0xFF || ucIncomingData[7] == 0x00) {
				bConnected = FALSE;
			}
			else {
				bConnected = TRUE;

				//Process the incoming data

				//Set the button booleans by bit masking
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

				//Assign the analog values
				ucJoyX = ucIncomingData[2];
				ucJoyY = ucIncomingData[3];
				ucCX = ucIncomingData[4];
				ucCY = ucIncomingData[5];
				ucLeftTrigger = ucIncomingData[6];
				ucRightTrigger = ucIncomingData[7];

				//Determine new position of Joystick
				INT iJoyXOffset = StickMap(ucJoyX, scale);
				INT iJoyYOffset = StickMap(ucJoyY, scale);
				rJoyPosition = D2D1::Rect(
					rPosition.left + iJoyXOffset,
					rPosition.top - iJoyYOffset,
					rPosition.right + iJoyXOffset,
					rPosition.bottom - iJoyYOffset);

				//Determine new position of C-Stick
				INT iCXOffset = StickMap(ucCX, scale);
				INT iCYOffset = StickMap(ucCY, scale);
				rCPosition = D2D1::Rect(
					rPosition.left + iCXOffset,
					rPosition.top - iCYOffset,
					rPosition.right + iCXOffset,
					rPosition.bottom - iCYOffset);

				//Determine new position of L-Meter fill
				INT LOffset = TriggerMap(ucLeftTrigger, scale);
				rLMeterFill = D2D1::Rect(
					rLMeter.left,
					rLMeter.top,
					rLMeter.right - LOffset,
					rLMeter.bottom);

				//Determine new position of R-Meter fill
				INT ROffset = TriggerMap(ucRightTrigger, scale);
				rRMeterFill = D2D1::Rect(
					rRMeter.left,
					rRMeter.top,
					rRMeter.right - ROffset,
					rRMeter.bottom);
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

void MainWindow::HandleMenu(WPARAM wParam) {

	DWORD idMenu = LOWORD(wParam);

	//Menu item is IDM_TOOLS_NONE
	if (idMenu == 0) {

		//Change current port
		HMENU hPort = GetSubMenu(hTools, 2);
		CheckMenuRadioItem(hPort, IDM_TOOLS_NONE, 99,
			idMenu, MF_BYCOMMAND);
		ucPort = idMenu;

		//Update the current port entry
		DeleteMenu(hTools, 3, MF_BYPOSITION);
		AppendMenuW(hTools, MF_POPUP | MF_GRAYED, IDM_TOOLS_CURRENT, L"(NONE)");
		DrawMenuBar(m_hwnd);

		//Close current port
		delete pPort;
		pPort = NULL;
	}

	//Menu item is IDM_TOOLS_COM(1-99)
	else if (idMenu <= 99) {

		//Change current port
		HMENU hPort = GetSubMenu(hTools, 2);
		CheckMenuRadioItem(hPort, IDM_TOOLS_NONE, 99,
			idMenu, MF_BYCOMMAND);
		ucPort = idMenu;

		//Update current port entry
		WCHAR wcName[8];
		GetMenuString(hPort, idMenu + 1, wcName, 7, MF_BYPOSITION);
		WCHAR wcPortName[10];
		StringCchPrintf(wcPortName, 11, L"(%s)", wcName);

		DeleteMenu(hTools, 3, MF_BYPOSITION);
		AppendMenuW(hTools, MF_POPUP | MF_GRAYED, IDM_TOOLS_CURRENT, wcPortName);
		DrawMenuBar(m_hwnd);

		//Close current port so new one can be opened
		delete pPort;
		pPort = NULL;
	}

	//Menu is not trying to change the port
	else {
		switch (idMenu) {

		//Hit "Quit", close the program.
		case IDM_FILE_QUIT:
			SendMessage(m_hwnd, WM_CLOSE, 0, 0);
			break;

		//Hit "Wiki"
		case IDM_HELP_WIKI:

			//Ask if it is ok to open up a webpage
			if (MessageBox(
				m_hwnd,
				L"Go to the Wiki?\nThis will open your default internet browser.",
				L"Launching Wiki Page",
				MB_ICONEXCLAMATION | MB_YESNO) == IDYES)

				//If ok, open the default browser to the GitHub wiki
				ShellExecute(NULL, L"open", L"https://github.com/VinnyF/GameCube-Controller-Reader/wiki", NULL, NULL, SW_SHOWNORMAL);
			break;
		
		//Hit "About"
		case IDM_HELP_ABOUT:

			//Display the about message
			MessageBox(
				m_hwnd,
				L"GCReader v1.0\nby Vincent Ferrara\nSee the GitHub wiki for full documentation.",
				L"About GameCube Controller Reader",
				MB_ICONINFORMATION);
			break;

		//Hit "Raw"
		case IDM_TOOLS_RAW:
			//Toggle bRaw and the check icon next to the menu item
			bRaw = ~bRaw;
			CheckMenuItem(hTools, 0, MF_BYPOSITION | (bRaw ? MF_CHECKED : MF_UNCHECKED));
			break;
		}
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	MainWindow win;

	//Create the window with ICON1
	if (!win.Create(L"GCReader v1.0", WS_OVERLAPPEDWINDOW, 0 , CW_USEDEFAULT, CW_USEDEFAULT, 650, 532, IDI_ICON1))
	{
		return 0;
	}

	//Load the accelerator table
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	ShowWindow(win.Window(), nCmdShow);

	// Run the message loop.
	MSG msg = {};

	//First check for an accelerator message, then proceede as normal
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(GetActiveWindow(), hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {

	//First the window is created
	case WM_CREATE:

		//Attempt to create the grahpics factory
		if (FAILED(D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory))) {
			return -1;  // Fail CreateWindowEx.
		}
		
		//Add the menu bar
		hTools = AddMenus(m_hwnd);

		//Set a timer for 30ms to poll the serial port
		SetTimer(
			m_hwnd,
			1,
			30,
			(TIMERPROC)NULL
		);
		return 0;

	//When closing the program, release all pointers and stop the timer
	case WM_DESTROY:
		DiscardGraphicsResources();
		KillTimer(m_hwnd, 72);
		SafeRelease(&pFactory);
		PostQuitMessage(0);
		return 0;

	//Repaint the window
	case WM_PAINT:
		OnPaint();
		return 0;

	//Resize the window
	case WM_SIZE:
		Resize();
		return 0;

	//Timer has expired, poll the port and redraw the window
	case WM_TIMER:
		tPollPort();
		InvalidateRect(m_hwnd, NULL, FALSE);
		return 0;
	
	//Menu item has been selected, process it
	case WM_COMMAND:
		HandleMenu(wParam);
		return 0;
	
	//Set the minimum size of the window such that the text doesn't get warped
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 443;
		lpMMI->ptMinTrackSize.y = 403;
		return 0;
	}

	//Set the cursor inside the window to a normal pointer
	case WM_SETCURSOR:
		HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
		if (LOWORD(lParam) == HTCLIENT) {
			SetCursor(hCursor);
			break;
		}
		break;
	}

	//If the message can't be intercepted, send it to the default procesure
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

//Load a bitmap and store it in a bitmap pointer
//Stolen from MSDN documentation
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

HMENU AddMenus(HWND hwnd) {

	//Initialize menu handles
	HMENU hMenubar;
	HMENU hFile;
	HMENU hTools;
	HMENU hHelp;
	HMENU hPort;

	//Create blank menus
	hMenubar = CreateMenu();
	hFile = CreateMenu();
	hTools = CreateMenu();
	hHelp = CreateMenu();
	hPort = CreatePopupMenu();

	//Add File -> Exit
	AppendMenuW(hFile, MF_STRING, IDM_FILE_QUIT, L"&Exit");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hFile, L"&File");

	//Add Tools -> Show Raw Bytes / Change Port / Current Port
	AppendMenuW(hTools, MF_POPUP, IDM_TOOLS_RAW, L"Show Raw Bytes\tB");
	AppendMenuW(hTools, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hTools, MF_POPUP, (UINT_PTR)hPort, L"Change Port...");
	AppendMenuW(hTools, MF_POPUP | MF_GRAYED, IDM_TOOLS_CURRENT, L"(NONE)");

	//Add Tools -> Change Port -> COM(0-99)
	//Save space by looping from 1 to 99, using those numbers as the identifiers
	AppendMenuW(hPort, MF_STRING, IDM_TOOLS_NONE, L"&None");
	AppendMenuW(hPort, MF_SEPARATOR, 0, NULL);
	for (UCHAR i = 1; i <= 99; i++) {
		WCHAR wcPortName[11];
		StringCchPrintf(wcPortName, 11, L"&COM%d", i);
		AppendMenuW(hPort, MF_STRING, i, wcPortName);
	}

	//Check the first item by default
	CheckMenuRadioItem(hTools, IDM_TOOLS_NONE, 99,
		IDM_TOOLS_NONE, MF_BYCOMMAND);

	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hTools, L"&Tools");

	//Add Help -> Wiki / About
	AppendMenuW(hHelp, MF_STRING, IDM_HELP_WIKI, L"&Wiki");
	AppendMenuW(hHelp, MF_STRING, IDM_HELP_ABOUT, L"&About...");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hHelp, L"&Help");

	//Display the menu
	SetMenu(hwnd, hMenubar);

	//Return a handle to the Tools menu, which will be needed later
	return hTools;
}