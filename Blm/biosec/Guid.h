/**************************************************************************
    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   (c) Microsoft Corporation. All Rights Reserved.
**************************************************************************/

// This file contains the CLSID and Property Keys used in this sample.
#define INITGUID
#include <guiddef.h>
#include <propkeydef.h>


// Command Line,
// explorer ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\::{BA16CE0E-728C-4FC9-98E5-D0B35B384597}

/*DEFINE_GUID(CLSID_FolderViewImpl,0xba16ce0e, 0x728c, 0x4fc9, 0x98, 0xe5, 0xd0, 0xb3, 0x5b, 0x38, 0x45, 0x97);*/
// {7826AAB0-9B4E-4C2A-801F-9D0B294F16AC}
DEFINE_GUID(CLSID_FolderViewImpl, 
			0x7826aab0, 0x9b4e, 0x4c2a, 0x80, 0x1f, 0x9d, 0xb, 0x29, 0x4f, 0x16, 0xac);

/*DEFINE_GUID(CLSID_FolderViewImplContextMenu, 0xbb8f539d, 0x3b97, 0x4473, 0x9e, 0x07, 0xc8, 0x24, 0x8c, 0x53, 0x24, 0x8e);*/
// {13ADE3E7-9C19-43FB-81DE-E1AB65F1FA45}
DEFINE_GUID(CLSID_FolderViewImplContextMenuSecure, 
			0x13ade3e7, 0x9c19, 0x43fb, 0x81, 0xde, 0xe1, 0xab, 0x65, 0xf1, 0xfa, 0x45);

// {8C6C7DAD-5914-48EF-A4F9-11DADD30180A}
DEFINE_GUID(CLSID_FolderViewImplContextMenuUnsecure, 
			0x8c6c7dad, 0x5914, 0x48ef, 0xa4, 0xf9, 0x11, 0xda, 0xdd, 0x30, 0x18, 0xa);


// Col 2
// name="Microsoft.SDKSample.AreaSize"
// {CE8B09DD-B2D6-4751-A207-4FFDD1A0F65C}
DEFINE_PROPERTYKEY(PKEY_Microsoft_SDKSample_AreaSize, 0xce8b09dd, 0xb2d6, 0x4751, 0xa2, 0x7, 0x4f, 0xfd, 0xd1, 0xa0, 0xf6, 0x5c, 3);

// Col 3
// name="Microsoft.SDKSample.NumberOfSides"
// {DADD3288-0380-4dd3-A5C4-CD7AE2A099F0}
DEFINE_PROPERTYKEY(PKEY_Microsoft_SDKSample_NumberOfSides, 0xdadd3288, 0x380, 0x4dd3, 0xa5, 0xc4, 0xcd, 0x7a, 0xe2, 0xa0, 0x99, 0xf0, 3);

// Col 4
// name="Microsoft.SDKSample.DirectoryLevel"
// {581CF603-2925-4acf-BB5A-3D3EB39EACD3}
DEFINE_PROPERTYKEY(PKEY_Microsoft_SDKSample_DirectoryLevel, 0x581cf603, 0x2925, 0x4acf, 0xbb, 0x5a, 0x3d, 0x3e, 0xb3, 0x9e, 0xac, 0xd3, 3);
