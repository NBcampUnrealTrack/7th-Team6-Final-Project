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

// DemoChannels.h
/// \file
/// Contains the declaration for the DemoChannels class.

#pragma once

#include "Page.h"

/// This class represents the Channels Demo page. This page offers the playback
/// of audio sources of various channel configurations.
class DemoChannels : public Page
{
public:

	/// DemoChannels class constructor
	DemoChannels( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return true if successful and false otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	/// Override of the Page::Draw() method.
	virtual void Draw();
private:

	/// Initializes the controls on the page.
	virtual void InitControls();
	void PositionControls();

	/// Delegate functions for the "Play" buttons.
	void Play_1_0( void* in_pSender, ControlEvent* in_pEvent );
	void Play_2_0( void* in_pSender, ControlEvent* in_pEvent );
	void Play_5_1( void* in_pSender, ControlEvent* in_pEvent );
	void Play_7_1( void* in_pSender, ControlEvent* in_pEvent );
	void Play_7_1_4( void* in_pSender, ControlEvent* in_pEvent );

	/// Game Object ID for the "Channels".
	static const AkGameObjectID GAME_OBJECT_CHANNELS = 123;
};
