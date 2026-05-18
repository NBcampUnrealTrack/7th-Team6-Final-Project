/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

// Drawing.cpp
/// \file 
// Implements the Drawing.h functions for the new Integration Demo renderer

/////////////////////////
//  INCLUDES
/////////////////////////

#include "stdafx.h"

#include "Drawing.h"
#include "UniversalInput.h"
#include "Render.h"
#include "FreetypeGraphicRenderer.h"

#if !defined (OLD_INTDEMO_RENDER)

/////////////////////////
//  GLOBAL OBJECTS
/////////////////////////

static FreetypeGraphicRenderer* g_pFreetypeRenderer = NULL;
static IDynamicTexture2D* g_pFreetypeTexture = NULL;

static float kLineColor[3] = { 0.25f, 0.25f, 0.80f };

/////////////////////////
//  FUNCTIONS
/////////////////////////

static AkUInt32 g_iLayoutWidth = 0;
static AkUInt32 g_iLayoutHeight = 0;


// Gamepad Overlay padding (in pixels)
const int kOverlayPadding = 20;

// Size of the virtual gamepad panel (in line height units)
const int kOverlaySizeHeight = 6;

static VirtualGamepadLayout* g_pOverlay = nullptr;

AkUInt32 GetLayoutWidth()
{
	return g_iLayoutWidth;
}

AkUInt32 GetLayoutHeight()
{
	return g_iLayoutHeight;
}

bool InitDrawing(
	AkOSChar* in_szErrorBuffer,
	unsigned int in_unErrorBufferCharCount,
	AkUInt32 in_canvasWidth,
	AkUInt32 in_canvasHeight,
	AkUInt32 in_layoutWidth,
	AkUInt32 in_layoutHeight
)
{
	g_pFreetypeRenderer = new FreetypeGraphicRenderer;

	bool bRet = g_pFreetypeRenderer->InitDrawing(in_szErrorBuffer,
		in_unErrorBufferCharCount,
		in_canvasWidth,
		in_canvasHeight,
		in_layoutWidth,
		in_layoutHeight);

	if (!bRet)
	{
		return bRet;
	}

	g_pFreetypeTexture = RenderCreateDynamicTexture(in_canvasWidth, in_canvasHeight);
	if (g_pFreetypeTexture)
	{
		g_pFreetypeTexture->AddRef();
	}

	g_iLayoutWidth = in_layoutWidth;
	g_iLayoutHeight = in_layoutHeight;

	return true;
}

void TermDrawing()
{
	if (g_pFreetypeRenderer != NULL)
	{
		g_pFreetypeRenderer->TermDrawing();
		delete g_pFreetypeRenderer;
		g_pFreetypeRenderer = NULL;
	}

	if (g_pFreetypeTexture)
	{
		g_pFreetypeTexture->Release();
		delete g_pFreetypeTexture;
		g_pFreetypeTexture = NULL;
	}
}

void BeginDrawing()
{
	if (g_pFreetypeRenderer)
		g_pFreetypeRenderer->BeginDrawing();
}

#define RENDER_TEST 0

#if RENDER_TEST

#include <cmath>

//////////////////////////////////////////////////////////////////////////
// Helper functions for testing parts of the renderer

void DrawTextureTest(IDynamicTexture2D& texture, DrawRect src, DrawRect dest)
{
	static int frameCount = 300;

	++frameCount;

	float centerX = std::cos(frameCount / 300.0f) / 4.0f + 0.5f;
	float centerY = std::sin(frameCount / 300.0f) / 2.25f + 0.5f;

	float boxLeft = centerX - 0.042f;
	float boxTop = centerY - 0.075f;
	float boxRight = centerX + 0.042f;
	float boxBottom = centerY + 0.075f;

	// Top left
	RenderDrawTexture2D(texture,
		DrawRect( 0.0f, 0.0f, boxLeft, boxTop ),
		DrawRect( 0.0f, 0.0f, boxLeft, boxTop ));

	// Top mid
	RenderDrawTexture2D(texture,
		DrawRect( boxLeft, 0.0f, boxRight, boxTop ),
		DrawRect( boxLeft, 0.0f, boxRight, boxTop ));

	// Top right
	RenderDrawTexture2D(texture,
		DrawRect( boxRight, 0.0f, 1.0f, boxTop ),
		DrawRect( boxRight, 0.0f, 1.0f, boxTop ));

	// Mid left
	RenderDrawTexture2D(texture,
		DrawRect( 0.0f, boxTop, boxLeft, boxBottom ),
		DrawRect( 0.0f, boxTop, boxLeft, boxBottom ));

	// Mid mid
	RenderDrawTexture2D(texture,
		DrawRect( boxLeft, boxBottom, boxRight, boxTop ),
		DrawRect( boxLeft, boxTop, boxRight, boxBottom ));
	float color[3] = { 1.0f, 0.0f, 0.0f };
	RenderDrawLine2D(boxLeft, boxTop, boxRight, boxTop, color);
	RenderDrawLine2D(boxRight, boxTop, boxRight, boxBottom, color);
	RenderDrawLine2D(boxRight, boxBottom, boxLeft, boxBottom, color);
	RenderDrawLine2D(boxLeft, boxBottom, boxLeft, boxTop, color);

	// Mid right
	RenderDrawTexture2D(texture,
		DrawRect( boxRight, boxTop, 1.0f, boxBottom ),
		DrawRect( boxRight, boxTop, 1.0f, boxBottom ));

	// Bot left
	RenderDrawTexture2D(texture,
		DrawRect( 0.0f, boxBottom, boxLeft, 1.0f ),
		DrawRect( 0.0f, boxBottom, boxLeft, 1.0f ));

	// Bot mid
	RenderDrawTexture2D(texture,
		DrawRect( boxLeft, boxBottom, boxRight, 1.0f ),
		DrawRect( boxLeft, boxBottom, boxRight, 1.0f ));

	// Bot right
	RenderDrawTexture2D(texture,
		DrawRect( boxRight, boxBottom, 1.0f, 1.0f ),
		DrawRect( boxRight, boxBottom, 1.0f, 1.0f ));
}

void RenderDrawLineTest()
{
	static int frameCount = 300;

	++frameCount;

	struct Matrix4x4
	{
		float v[4][4];
		Matrix4x4()
		{
			memset(v, 0, 4 * 4 * sizeof(float));
			v[0][0] = 1.0f;
			v[1][1] = 1.0f;
			v[2][2] = 1.0f;
			v[3][3] = 1.0f;
		}
	};

	struct Vector4
	{
		float v[4];

		static Vector4 mul(Matrix4x4 m, Vector4 v)
		{
			Vector4 ret =
			{
				m.v[0][0] * v.v[0] + m.v[0][1] * v.v[1] + m.v[0][2] * v.v[2] + m.v[0][3] * v.v[3],
				m.v[1][0] * v.v[0] + m.v[1][1] * v.v[1] + m.v[1][2] * v.v[2] + m.v[1][3] * v.v[3],
				m.v[2][0] * v.v[0] + m.v[2][1] * v.v[1] + m.v[2][2] * v.v[2] + m.v[2][3] * v.v[3],
				m.v[3][0] * v.v[0] + m.v[3][1] * v.v[1] + m.v[3][2] * v.v[2] + m.v[3][3] * v.v[3]
			};
			return ret;
		}

		static Vector4 project(Vector4 v)
		{
			Vector4 ret =
			{
				v.v[0] / v.v[3],
				v.v[1] / v.v[3],
				v.v[2] / v.v[3],
				v.v[3] / v.v[3]
			};
			return ret;
		}
	};

	Vector4 points[8] = {
		{0.0f, 0.0f, 0.0f, 1.0f}, // 0
		{0.0f, 0.0f, 1.0f, 1.0f}, // 1
		{0.0f, 1.0f, 0.0f, 1.0f}, // 2
		{0.0f, 1.0f, 1.0f, 1.0f}, // 3 
		{1.0f, 0.0f, 0.0f, 1.0f}, // 4
		{1.0f, 0.0f, 1.0f, 1.0f}, // 5
		{1.0f, 1.0f, 0.0f, 1.0f}, // 6
		{1.0f, 1.0f, 1.0f, 1.0f}  // 7
	};

	Matrix4x4 translate, view, proj;

	// set up translate
	{
		translate.v[0][3] = -0.5f;
		translate.v[1][3] = -0.5f;
		translate.v[2][3] = -0.5f;
	}

	// set up view
	{
		float xTheta = frameCount / 360.f;
		float yTheta = frameCount / -512.f;
		float zTheta = frameCount / 1200.f;

		float cosX = std::cos(xTheta);
		float sinX = std::sin(xTheta);
		float cosY = std::cos(yTheta);
		float sinY = std::sin(yTheta);
		float cosZ = std::cos(zTheta);
		float sinZ = std::sin(zTheta);

		view.v[0][0] = cosY * cosZ;
		view.v[0][1] = sinX * sinY * cosZ - cosX * sinZ;
		view.v[0][2] = sinX * sinZ + cosX * sinY * cosZ;
		view.v[1][0] = cosY * sinZ;
		view.v[1][1] = cosX * cosZ + sinX * sinY * sinZ;
		view.v[1][2] = cosX * sinY * sinZ - sinX * cosZ;
		view.v[2][0] = 0 - sinY;
		view.v[2][1] = sinX * cosY;
		view.v[2][2] = cosX * cosY;

	}

	// set up proj
	{
		proj.v[0][0] = 6 / (16.f);
		proj.v[1][1] = 6 / (9.f);
	}

	// transform points as needed
	for (int i = 0; i < 8; ++i)
	{
		points[i] = Vector4::mul(translate, points[i]);
		points[i] = Vector4::mul(view, points[i]);
		points[i] = Vector4::mul(proj, points[i]);
		points[i] = Vector4::project(points[i]);
	}

	// convert them to linepts
	int linePts[8][2];
	for (int i = 0; i < 8; ++i)
	{
		linePts[i][0] = (int)(points[i].v[0] * 160.f + 80.f);
		linePts[i][1] = (int)(points[i].v[1] * 90.f + 90.f);
	}

	// draw all of the lines using indices provided
	int idx[12][2] = {
		{0,1}, {0,2}, {0,4},
		{1,3}, {1,5},
		{2,3}, {2,6},
		{3,7},
		{4,5}, {4,6},
		{5,7},
		{6,7}
	};

	for (int i = 0; i < 12; ++i)
	{
		DrawLineOnScreen(
			linePts[idx[i][0]][0], linePts[idx[i][0]][1],
			linePts[idx[i][1]][0], linePts[idx[i][1]][1],
			i % 2 == 0 ? DrawStyle_Selected : DrawStyle_Control);
	}
}
#endif

static void DrawRectangle(float x1, float y1, float x2, float y2)
{
	RenderDrawLine2D(x1, y1, x2, y1, kLineColor);
	RenderDrawLine2D(x2, y1, x2, y2, kLineColor);
	RenderDrawLine2D(x2, y2, x1, y2, kLineColor);
	RenderDrawLine2D(x1, y2, x1, y1, kLineColor);
}

static void DrawVirtualGamepadLayout(const VirtualGamepadLayout& overlay)
{
	for (auto it = overlay.buttons.cbegin(); it != overlay.buttons.cend(); ++it)
	{
		const VirtualButtonLayout& btn = *it;
		const char labelStr[2] = { btn.label, 0 };
		DrawRectangle(btn.rx1, btn.ry1, btn.rx2, btn.ry2);
		DrawTextOnScreen(labelStr, (int)(btn.gx * g_iLayoutWidth), (int)(btn.gy * g_iLayoutHeight), DrawStyle_Control);
	}
}

void DoneDrawing()
{
	if (g_pOverlay)
	{
		DrawVirtualGamepadLayout(*g_pOverlay);
	}
	if (g_pFreetypeRenderer)
	{
		g_pFreetypeRenderer->DoneDrawing();
		if (g_pFreetypeTexture)
		{
			g_pFreetypeTexture->UpdateResource(g_pFreetypeRenderer->GetCanvasBuffer(),
				g_pFreetypeRenderer->GetCanvasBufferPitch(),
				g_pFreetypeRenderer->GetCanvasBufferLength());

			DrawRect src ( 0.0f, 0.0f, 1.0f, 1.0f );
			DrawRect dest( 0.0f, 0.0f, 1.0f, 1.0f );
#if RENDER_TEST
			RenderDrawLineTest();
			DrawTextureTest(*g_pFreetypeTexture, src, dest);
#else
			RenderDrawTexture2D(*g_pFreetypeTexture, src, dest);
#endif
		}
	}
}


void DrawTextOnScreen(const char* in_szText, int in_iXPos, int in_iYPos, DrawStyle in_eDrawStyle)
{
	if (g_pFreetypeRenderer)
	{
		g_pFreetypeRenderer->DrawTextOnScreen(in_szText, in_iXPos, in_iYPos, in_eDrawStyle);
	}
}

void DrawLineOnScreen(int in_iXPosStart, int in_iYPosStart, int in_iXPosEnd, int in_iYPosEnd, DrawStyle in_eDrawStyle)
{
	float color[3];
	switch (in_eDrawStyle)
	{
	case DrawStyle_SelectedText:
	case DrawStyle_Selected:
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 0.0f;
		break;
	case DrawStyle_Error:
		color[0] = 1.0f;
		color[1] = 0.0f;
		color[2] = 0.0f;
		break;
	case DrawStyle_Text:
	case DrawStyle_Control:
	case DrawStyle_Title:
	default:
		color[0] = 1.0f;
		color[1] = 1.0f;
		color[2] = 1.0f;
		break;
	}

	RenderDrawLine2D(
		in_iXPosStart / (float)g_iLayoutWidth,
		in_iYPosStart / (float)g_iLayoutHeight,
		in_iXPosEnd / (float)g_iLayoutWidth,
		in_iYPosEnd / (float)g_iLayoutHeight,
		color);
}

void DrawLineOnScreen(int in_iXPosStart, int in_iYPosStart, int in_iXPosEnd, int in_iYPosEnd, float color[3])
{
	RenderDrawLine2D(
		in_iXPosStart / (float)g_iLayoutWidth,
		in_iYPosStart / (float)g_iLayoutHeight,
		in_iXPosEnd / (float)g_iLayoutWidth,
		in_iYPosEnd / (float)g_iLayoutHeight,
		color);
}

int GetLineHeight(DrawStyle in_eDrawStyle)
{
	if (g_pFreetypeRenderer == NULL)
		return 0;

	return g_pFreetypeRenderer->GetLayoutLineHeight(in_eDrawStyle);
}

static VirtualButtonLayout ComputeOverlayButton(UGButton code, int x1, int y1, int length, char label)
{
	// Our FreeType renderer doesn't have a way to render centered text, but we can approximate by querying the glyph's width
	int c_x1, c_y1, c_x2, c_y2;
	g_pFreetypeRenderer->GetGlyphBoundingBox(label, c_x1, c_y1, c_x2, c_y2, DrawStyle_Control);
	int glyphHalfWidth = (c_x2 - c_x1) / 2;
	int glyphHalfHeight = GetLineHeight(DrawStyle_Control) / 2;
	return VirtualButtonLayout{
		code,
		(float)x1 / g_iLayoutWidth,
		(float)y1 / g_iLayoutHeight,
		(float)(x1 + length) / g_iLayoutWidth,
		(float)(y1 + length) / g_iLayoutHeight,
		(float)(x1 + (length / 2) - glyphHalfWidth) / g_iLayoutWidth,
		(float)(y1 + (length / 2) - glyphHalfHeight) / g_iLayoutHeight,
		label
	};
}

// Compute the descriptors for four buttons in a typical cross arrangement bounded by a square area
// labels and button codes should be in clockwise order starting from top
// You can skip a button by specifying AK_INVALID_UGBUTTON
static void ComputeCrossButtons(VirtualGamepadLayout& overlay, int x1, int y1, int length, const char* labels, UGButton* buttonCodes)
{
	int fT1 = length / 3;
	int fT2 = fT1 * 2;
	if (buttonCodes[0] != AK_INVALID_UGBUTTON)
		overlay.buttons.push_back(ComputeOverlayButton(buttonCodes[0], x1 + fT1, y1, fT1, labels[0]));
	if (buttonCodes[1] != AK_INVALID_UGBUTTON)
		overlay.buttons.push_back(ComputeOverlayButton(buttonCodes[1], x1 + fT2, y1 + fT1, fT1, labels[1]));
	if (buttonCodes[2] != AK_INVALID_UGBUTTON)
		overlay.buttons.push_back(ComputeOverlayButton(buttonCodes[2], x1 + fT1, y1 + fT2, fT1, labels[2]));
	if (buttonCodes[3] != AK_INVALID_UGBUTTON)
		overlay.buttons.push_back(ComputeOverlayButton(buttonCodes[3], x1, y1 + fT1, fT1, labels[3]));
}

VirtualGamepadLayout* Drawing_CreateGamepadLayout()
{
	AKASSERT(g_iLayoutWidth > 0 && g_iLayoutHeight > 0); // It is an error to call this function before InitDrawing().

	VirtualGamepadLayout* pOverlay = new VirtualGamepadLayout();
	if (!pOverlay) return nullptr;

	int iPanelHeight = GetLineHeight(DrawStyle_Control) * kOverlaySizeHeight;

	pOverlay->fHeight = (float)iPanelHeight / g_iLayoutHeight;

	// D-Pad
	UGButton cross1[4] = { UG_DPAD_UP, UG_DPAD_RIGHT, UG_DPAD_DOWN, UG_DPAD_LEFT };
	ComputeCrossButtons(*pOverlay, kOverlayPadding, g_iLayoutHeight - iPanelHeight - kOverlayPadding, iPanelHeight, "URDL", cross1);

	// Action buttons
	UGButton cross2[4] = { UG_BUTTON4, UG_BUTTON2, UG_BUTTON1, UG_BUTTON3 };
	ComputeCrossButtons(*pOverlay, g_iLayoutWidth - iPanelHeight - kOverlayPadding, g_iLayoutHeight - iPanelHeight - kOverlayPadding, iPanelHeight, "YBAX", cross2);

	// Start/Select
	UGButton cross3[4] = { AK_INVALID_UGBUTTON, UG_BUTTON7, AK_INVALID_UGBUTTON, UG_BUTTON8 };
	ComputeCrossButtons(*pOverlay, (g_iLayoutWidth / 2) - (iPanelHeight / 2), g_iLayoutHeight - iPanelHeight - kOverlayPadding, iPanelHeight, " > <", cross3);
	return pOverlay;
}

void Drawing_DestroyGamepadLayout(VirtualGamepadLayout* pOverlay)
{
	if (pOverlay)
		delete pOverlay;
}

void Drawing_SetVirtualGamepadLayout(VirtualGamepadLayout* pOverlay)
{
	g_pOverlay = pOverlay;
}

#endif