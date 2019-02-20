//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// This file contains some global variables that describe what our
// sample tile looks like.  For example, it defines what fields a tile has 
// and which fields show in which states of LogonUI.

#pragma once
#include "helpers.h"

// The indexes of each of the fields in our credential provider's tiles.
enum SAMPLE_FIELD_ID 
{
    SFI_TILE_IMAGE       = 0,
    SFI_USERNAME        = 1,
    SFI_PASSWORD        = 2,
	SFI_SUBMIT_BUTTON   = 3, 
	SFI_REFRESH_BUTTON   = 4, 
    SFI_NUM_FIELDS      = 5,  // Note: if new fields are added, keep NUM_FIELDS last.  This is used as a count of the number of fields
};

// Same as SAMPLE_FIELD_ID above, but for the CMultiloginCredential.
enum SAMPLE_MESSAGE_FIELD_ID 
{
	SMFI_TILE_IMAGE     = 0,
	SMFI_MESSAGE        = 1, 
	SMFI_SEL_MESSAGE    = 2, 
	SMFI_REFRESH_PROMT  = 3, 
	SMFI_REFRESH_BUTTON = 4, 
	SMFI_NUM_FIELDS     = 5,  // Note: if new fields are added, keep NUM_FIELDS last.  This is used as a count of the number of fields
};

// The first value indicates when the tile is displayed (selected, not selected)
// the second indicates things like whether the field is enabled, whether it has key focus, etc.
struct FIELD_STATE_PAIR
{
    CREDENTIAL_PROVIDER_FIELD_STATE cpfs;
    CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE cpfis;
};

// These two arrays are seperate because a credential provider might
// want to set up a credential with various combinations of field state pairs 
// and field descriptors.

// The field state value indicates whether the field is displayed
// in the selected tile, the deselected tile, or both.
// The Field interactive state indicates when 
static const FIELD_STATE_PAIR s_rgFieldStatePairs[] = 
{
    { CPFS_DISPLAY_IN_BOTH, CPFIS_NONE },                   // SFI_TILEIMAGE
    { CPFS_DISPLAY_IN_BOTH, CPFIS_NONE },                   // SFI_USERNAME
	{ CPFS_HIDDEN, CPFIS_READONLY},						// SFI_PASSWORD
	{ CPFS_HIDDEN, CPFIS_FOCUSED   },       // SFI_SUBMIT_BUTTON   
	{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE },                   // SFI_USERNAME
};

// Same as s_rgFieldStatePairs above, but for the CMultiloginCredential.
static const FIELD_STATE_PAIR s_rgMessageFieldStatePairs[] = 
{
	{ CPFS_DISPLAY_IN_BOTH,			   CPFIS_NONE },                   // SMFI_MESSAGE
	{ CPFS_DISPLAY_IN_DESELECTED_TILE, CPFIS_NONE },                   // SMFI_MESSAGE
	{ CPFS_DISPLAY_IN_SELECTED_TILE,   CPFIS_NONE },                   // SMFI_MESSAGE
	{ CPFS_DISPLAY_IN_SELECTED_TILE,   CPFIS_NONE },                   // SMFI_MESSAGE
	{ CPFS_DISPLAY_IN_SELECTED_TILE,   CPFIS_NONE },                   // SMFI_MESSAGE
};


// Field descriptors for unlock and logon.
// The first field is the index of the field.
// The second is the type of the field.
// The third is the name of the field, NOT the value which will appear in the field.
static const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR s_rgCredProvFieldDescriptors[] =
{
    { SFI_TILE_IMAGE, CPFT_TILE_IMAGE, L"Image" },
    { SFI_USERNAME, CPFT_LARGE_TEXT, L"Username" },
    { SFI_PASSWORD, CPFT_PASSWORD_TEXT, L"Password" },
	{ SFI_SUBMIT_BUTTON, CPFT_SUBMIT_BUTTON, L"Submit" },
	{ SFI_REFRESH_BUTTON, CPFT_COMMAND_LINK, L"Refresh" },
};

// Same as s_rgCredProvFieldDescriptors above, but for the CMultiloginCredential.
static const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR s_rgMessageCredProvFieldDescriptors[] =
{
	{ SMFI_TILE_IMAGE,		   CPFT_TILE_IMAGE, L"Image" },
	{ SMFI_MESSAGE,		       CPFT_LARGE_TEXT, L"Identify" },
	{ SMFI_SEL_MESSAGE,		   CPFT_LARGE_TEXT, L"Touch" },
	{ SMFI_REFRESH_PROMT,  CPFT_SMALL_TEXT, L"Promt" },
	{ SMFI_REFRESH_BUTTON, CPFT_COMMAND_LINK, L"Refresh" },
};
