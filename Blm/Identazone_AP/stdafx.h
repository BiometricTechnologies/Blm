/*
 * Copyright 2011-2013 Berin Lautenbach
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */


#pragma once

// Shut down annoying warnings from VC++

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)

#define WINVER 0x0600
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <windows.h>

// Other useful files
#include <strsafe.h>

#include "IdentaZoneAP.h"
