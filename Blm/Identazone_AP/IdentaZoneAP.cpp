#include "stdafx.h"


//#define WINVER 0x0600
//#include <ntstatus.h>
//#include <windows.h>
#include <wininet.h>
#include <lmcons.h>
#include <iptypes.h>
#include <ntsecapi.h>
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <sspi.h>
#include <NTSecPKG.h>


#include <schannel.h>

#include <stdio.h>

// SID formatting
#include <Sddl.h>

#include "IdentaZoneAP.h"
#include "logging.h"

/*
---------------------------------------------------------------------------------------------
Static Variables held for all functions to use
---------------------------------------------------------------------------------------------
*/

// LSA Mode variables

static ULONG_PTR SpInitializePackageId = 0;
static PSECPKG_PARAMETERS SpInitializeParameters;
static PLSA_SECPKG_FUNCTION_TABLE LsaFunctions;
static PLSA_DISPATCH_TABLE LsaDispatchTable;
static SECPKG_FUNCTION_TABLE secpkgFunctionTable[1];
static ULONG LsaVersion = 0;

// User Mode Variable
static SECPKG_USER_FUNCTION_TABLE secpkgUserFunctionTable[1];

/* For proxying through to Microsoft's NTLM SSP */
static HMODULE DLLModuleMSV1_0 = NULL;
static SECPKG_FUNCTION_TABLE *msv1_0FunctionTable;
static SECPKG_USER_FUNCTION_TABLE *msv1_0UserFunctionTable;


SEC_WCHAR PACKAGE_NAME[] = L"IdentaZone AP";
SEC_WCHAR PACKAGE_COMMENT[] = L"Biometrics authentification provider";

/* For proxying through to Microsot

/*
* ---------------------------------------------------------------------------------------------
*
* AP Functions
*
* The following functions are all the AP functions required for the Authentication Provider
* components
*
* ---------------------------------------------------------------------------------------------
*/


/*
---------------------------------------------------------------------------------------------
LsaApInitializePackage
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI LsaApInitializePackage(
	_In_      ULONG AuthenticationPackageId,
	_In_      PLSA_DISPATCH_TABLE InputLsaDispatchTable,
	_In_opt_  PLSA_STRING Database,
	_In_opt_  PLSA_STRING Confidentiality,
	_Out_     PLSA_STRING *AuthenticationPackageName)

{
	LSA_AP_INITIALIZE_PACKAGE * MSVLsaApInitializePackage = NULL;
	//	NTSTATUS Status;

	iz_log("LsaApInitializePackage called. PackageId = %llu", AuthenticationPackageId);

	/* Useful stuff to keep */
	LsaDispatchTable = InputLsaDispatchTable;

	// If defined in the function table - use it.  Otherwise find the export
	/*
	MSVLsaApInitializePackage = msv1_0FunctionTable[0].InitializePackage;
	if (MSVLsaApInitializePackage == NULL)
	{
	/* Check for msv1_0 (NTLM) equivalent
	if (DLLModuleMSV1_0 == NULL || 
	((MSVLsaApInitializePackage = (LSA_AP_INITIALIZE_PACKAGE *) 
	GetProcAddress(DLLModuleMSV1_0, "LsaApInitializePackage")) == NULL))
	{
	iz_log("LsaApInitializePackage - error loading NTLM module or finding LsaApInitializePackage");
	return STATUS_DLL_NOT_FOUND;
	}
	}

	if ((Status = MSVLsaApInitializePackage(AuthenticationPackageId, InputLsaDispatchTable, 
	Database, Confidentiality, AuthenticationPackageName)) != STATUS_SUCCESS)
	{
	iz_log("LsaApInitializePackage - Error returned from NTLM package.  Error # = %lu", Status);
	return Status;
	}
	*/
	return STATUS_SUCCESS;

}

/*
---------------------------------------------------------------------------------------------
LsaApCallPackage
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI LsaApCallPackage(
	_In_   PLSA_CLIENT_REQUEST ClientRequest,
	_In_   PVOID ProtocolSubmitBuffer,
	_In_   PVOID ClientBufferBase,
	_In_   ULONG SubmitBufferLength,
	_Out_  PVOID *ProtocolReturnBuffer,
	_Out_  PULONG ReturnBufferLength,
	_Out_  PNTSTATUS ProtocolStatus)

{
	LSA_AP_CALL_PACKAGE * MSVLsaApCallPackage;
	NTSTATUS Status;

	iz_log("LsaApCallPackage called. ");
	/*
	MSVLsaApCallPackage = msv1_0FunctionTable[0].CallPackage;
	if (MSVLsaApCallPackage == NULL)
	{
	/* Check for msv1_0 (NTLM) equivalent 
	if (DLLModuleMSV1_0 == NULL || 
	((MSVLsaApCallPackage = (LSA_AP_CALL_PACKAGE *) 
	GetProcAddress(DLLModuleMSV1_0, "LsaApCallPackage")) == NULL))
	{
	iz_log("LsaApCallPackage - error loading NTLM module or finding LsaApCallPackage");
	return STATUS_DLL_NOT_FOUND;
	}
	}

	if ((Status = MSVLsaApCallPackage(ClientRequest, ProtocolSubmitBuffer, ClientBufferBase, SubmitBufferLength,
	ProtocolReturnBuffer, ReturnBufferLength, ProtocolStatus)) != STATUS_SUCCESS)
	{
	iz_log("LsaApCallPackage - Error returned from NTLM package.  Error # = %lu", Status);
	return Status;
	}
	*/
	return STATUS_SUCCESS;
}

/*
---------------------------------------------------------------------------------------------
LsaApCallPackagePassthrough
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI LsaApCallPackagePassthrough(
	_In_   PLSA_CLIENT_REQUEST ClientRequest,
	_In_   PVOID ProtocolSubmitBuffer,
	_In_   PVOID ClientBufferBase,
	_In_   ULONG SubmitBufferLength,
	_Out_  PVOID *ProtocolReturnBuffer,
	_Out_  PULONG ReturnBufferLength,
	_Out_  PNTSTATUS ProtocolStatus)

{
	LSA_AP_CALL_PACKAGE_PASSTHROUGH * MSVLsaApCallPackagePassthrough;
	NTSTATUS Status;

	iz_log("LsaApCallPackagePassthrough called. ");
	/*
	MSVLsaApCallPackagePassthrough = msv1_0FunctionTable[0].CallPackagePassthrough;
	if (MSVLsaApCallPackagePassthrough == NULL)
	{
	/* Check for msv1_0 (NTLM) equivalent 
	if (DLLModuleMSV1_0 == NULL || 
	((MSVLsaApCallPackagePassthrough = (LSA_AP_CALL_PACKAGE_PASSTHROUGH *) 
	GetProcAddress(DLLModuleMSV1_0, "LsaApCallPackagePassthrough")) == NULL))
	{
	iz_log("LsaApCallPackagePassthrough - error loading NTLM module or finding LsaApCallPackagePassthrough");
	return STATUS_DLL_NOT_FOUND;
	}
	}

	if ((Status = MSVLsaApCallPackagePassthrough(ClientRequest, ProtocolSubmitBuffer, ClientBufferBase, SubmitBufferLength,
	ProtocolReturnBuffer, ReturnBufferLength, ProtocolStatus)) != STATUS_SUCCESS)
	{
	iz_log("LsaApCallPackagePassthrough - Error returned from NTLM package.  Error # = %lu", Status);
	return Status;
	}*/

	return STATUS_SUCCESS;
}

/*
---------------------------------------------------------------------------------------------
LsaApCallPackageUntrusted
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI LsaApCallPackageUntrusted(
	_In_   PLSA_CLIENT_REQUEST ClientRequest,
	_In_   PVOID ProtocolSubmitBuffer,
	_In_   PVOID ClientBufferBase,
	_In_   ULONG SubmitBufferLength,
	_Out_  PVOID *ProtocolReturnBuffer,
	_Out_  PULONG ReturnBufferLength,
	_Out_  PNTSTATUS ProtocolStatus)

{
	LSA_AP_CALL_PACKAGE_UNTRUSTED * MSVLsaApCallPackageUntrusted;
	NTSTATUS Status;

	iz_log("LsaApCallPackageUntrusted called. ");
	/*

	MSVLsaApCallPackageUntrusted = msv1_0FunctionTable[0].CallPackageUntrusted;
	if (MSVLsaApCallPackageUntrusted == NULL)
	{
	/* Check for msv1_0 (NTLM) equivalent 
	if (DLLModuleMSV1_0 == NULL || 
	((MSVLsaApCallPackageUntrusted = (LSA_AP_CALL_PACKAGE_PASSTHROUGH *) 
	GetProcAddress(DLLModuleMSV1_0, "LsaApCallPackageUntrusted")) == NULL))
	{
	iz_log("LsaApCallPackageUntrusted - error loading NTLM module or finding LsaApCallPackageUntrusted");
	return STATUS_DLL_NOT_FOUND;
	}
	}

	if ((Status = MSVLsaApCallPackageUntrusted(ClientRequest, ProtocolSubmitBuffer, ClientBufferBase, SubmitBufferLength,
	ProtocolReturnBuffer, ReturnBufferLength, ProtocolStatus)) != STATUS_SUCCESS)
	{
	iz_log("LsaApCallPackageUntrusted - Error returned from NTLM package.  Error # = %lu", Status);
	return Status;
	}*/

	return STATUS_SUCCESS;
}

/*
---------------------------------------------------------------------------------------------
LsaApLogonTerminated
---------------------------------------------------------------------------------------------
*/

VOID NTAPI LsaApLogonTerminated(
	_In_  PLUID LogonId)

{
	LSA_AP_LOGON_TERMINATED * MSVLsaApLogonTerminated;

	iz_log("LsaApLogonTerminated called. ");
	/*
	MSVLsaApLogonTerminated = msv1_0FunctionTable[0].LogonTerminated;
	if (MSVLsaApLogonTerminated == NULL)
	{
	/* Check for msv1_0 (NTLM) equivalent 
	if (DLLModuleMSV1_0 == NULL || 
	((MSVLsaApLogonTerminated = (LSA_AP_LOGON_TERMINATED *) 
	GetProcAddress(DLLModuleMSV1_0, "LsaApLogonTerminated")) == NULL))
	{
	iz_log("LsaApLogonTerminated - error loading NTLM module or finding LsaApLogonTerminated");
	return;
	}
	}

	MSVLsaApLogonTerminated(LogonId);
	*/
}

/*
---------------------------------------------------------------------------------------------
LsaApLogonUserEx2
---------------------------------------------------------------------------------------------
*/

HRESULT GetSid(
	LPCWSTR wszAccName,
	PSID * ppSid
	) 
{

	// Validate the input parameters.
	if (wszAccName == NULL || ppSid == NULL)
	{
		return -1;
	}


	// Create buffers that may be large enough.
	// If a buffer is too small, the count parameter will be set to the size needed.
	DWORD INITIAL_SIZE;
	INITIAL_SIZE = 32;
	DWORD cbSid = 0;
	DWORD dwSidBufferSize = INITIAL_SIZE;
	DWORD cchDomainName = 0;
	DWORD dwDomainBufferSize = INITIAL_SIZE;
	WCHAR * wszDomainName = NULL;
	SID_NAME_USE eSidType;
	DWORD dwErrorCode = 0;
	HRESULT hr = S_OK;


	// Create buffers for the SID and the domain name.
	*ppSid = (PSID) new BYTE[dwSidBufferSize];
	if (*ppSid == NULL)
	{
		return -2;
	}
	memset(*ppSid, 0, dwSidBufferSize);
	wszDomainName = new WCHAR[dwDomainBufferSize];
	if (wszDomainName == NULL)
	{
		return -3;
	}
	memset(wszDomainName, 0, dwDomainBufferSize*sizeof(WCHAR));


	// Obtain the SID for the account name passed.
	for ( ; ; )
	{

		// Set the count variables to the buffer sizes and retrieve the SID.
		cbSid = dwSidBufferSize;
		cchDomainName = dwDomainBufferSize;
		if (LookupAccountNameW(
			NULL,            // Computer name. NULL for the local computer
			wszAccName,
			*ppSid,          // Pointer to the SID buffer. Use NULL to get the size needed,
			&cbSid,          // Size of the SID buffer needed.
			wszDomainName,   // wszDomainName,
			&cchDomainName,
			&eSidType
			))
		{
			if (IsValidSid(*ppSid) == FALSE)
			{
				wprintf(L"The SID for %s is invalid.\n", wszAccName);
				dwErrorCode = -4;
			}
			break;
		}
		dwErrorCode = GetLastError();


		// Check if one of the buffers was too small.
		if (dwErrorCode == ERROR_INSUFFICIENT_BUFFER)
		{
			if (cbSid > dwSidBufferSize)
			{

				// Reallocate memory for the SID buffer.
				wprintf(L"The SID buffer was too small. It will be reallocated.\n");
				FreeSid(*ppSid);
				*ppSid = (PSID) new BYTE[cbSid];
				if (*ppSid == NULL)
				{
					return -5;
				}
				memset(*ppSid, 0, cbSid);
				dwSidBufferSize = cbSid;
			}
			if (cchDomainName > dwDomainBufferSize)
			{

				// Reallocate memory for the domain name buffer.
				wprintf(L"The domain name buffer was too small. It will be reallocated.\n");
				delete [] wszDomainName;
				wszDomainName = new WCHAR[cchDomainName];
				if (wszDomainName == NULL)
				{
					return -6;
				}
				memset(wszDomainName, 0, cchDomainName*sizeof(WCHAR));
				dwDomainBufferSize = cchDomainName;
			}
		}
		else
		{
			//wprintf(L"LookupAccountNameW failed. GetLastError returned: %d\n", dwErrorCode);
			hr = HRESULT_FROM_WIN32(dwErrorCode);
			break;
		}
	}

	delete [] wszDomainName;
	return hr; 
}


#define CLIENT_BUFFER_SIZE 64

typedef struct TokenLoad_{
	SID sid;
	TOKEN_GROUPS groups;
	SID primaryGroup;
}TokenLoad;

void fillString(PUNICODE_STRING * pStr, const wchar_t * source)
{
	PUNICODE_STRING str = (PUNICODE_STRING) LsaFunctions->AllocateLsaHeap(sizeof(PUNICODE_STRING ));
	str->Length = (wcslen(source) ) * (sizeof(wchar_t));
	str->MaximumLength = str->Length + sizeof(UNICODE_NULL);
	str->Buffer = (PWSTR) LsaFunctions->AllocateLsaHeap(str->MaximumLength);
	wcscpy(str->Buffer,source);
	*pStr = str;
}


LSA_HANDLE GetPolicyHandle(PWCHAR SystemName)
{
	LSA_OBJECT_ATTRIBUTES ObjectAttributes;
	USHORT SystemNameLength;
	LSA_UNICODE_STRING lusSystemName;
	NTSTATUS ntsResult;
	LSA_HANDLE lsahPolicyHandle;

	// Object attributes are reserved, so initialize to zeros.
	ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

	//Initialize an LSA_UNICODE_STRING to the server name.
	SystemNameLength = wcslen(SystemName);
	lusSystemName.Buffer = SystemName;
	lusSystemName.Length = SystemNameLength * sizeof(WCHAR);
	lusSystemName.MaximumLength = (SystemNameLength+1) * sizeof(WCHAR);

	// Get a handle to the Policy object.
	ntsResult = LsaOpenPolicy(
		&lusSystemName,    //Name of the target system.
		&ObjectAttributes, //Object attributes.
		POLICY_ALL_ACCESS, //Desired access permissions.
		&lsahPolicyHandle  //Receives the policy handle.
		);

	if (ntsResult != STATUS_SUCCESS)
	{
		// An error occurred. Display it as a win32 error code.
		iz_log("OpenPolicy returned %ls",
			LsaNtStatusToWinError(ntsResult));
		return NULL;
	} 
	return lsahPolicyHandle;
}

bool InitLsaString(
	PLSA_UNICODE_STRING pLsaString,
	LPCWSTR pwszString
	)
{
	DWORD dwLen = 0;

	if (NULL == pLsaString)
		return FALSE;

	if (NULL != pwszString) 
	{
		dwLen = wcslen(pwszString);
		if (dwLen > 0x7ffe)   // String is too large
			return FALSE;
	}

	// Store the string.
	pLsaString->Buffer = (WCHAR *)pwszString;
	pLsaString->Length =  (USHORT)dwLen * sizeof(WCHAR);
	pLsaString->MaximumLength= (USHORT)(dwLen+1) * sizeof(WCHAR);

	return TRUE;
}

void GetSIDInformation (LPWSTR AccountName,LSA_HANDLE PolicyHandle, PSID * ppSid)
{
	LSA_UNICODE_STRING lucName;
	PLSA_TRANSLATED_SID2 ltsTranslatedSID;
	PLSA_REFERENCED_DOMAIN_LIST   lrdlDomainList;
	LSA_TRUST_INFORMATION myDomain;
	NTSTATUS ntsResult;
	PWCHAR DomainString = NULL;

	// Initialize an LSA_UNICODE_STRING with the name.
	if (!InitLsaString(&lucName, AccountName))
	{
		iz_log("Failed InitLsaString");
		return;
	}

	ntsResult = LsaLookupNames2(
		PolicyHandle,     // handle to a Policy object
		0x80000000,	      //LSA_LOOKUP_ISOLATED_AS_LOCAL
		1,                // number of names to look up
		&lucName,         // pointer to an array of names
		&lrdlDomainList,  // receives domain information
		&ltsTranslatedSID // receives relative SIDs
		);
	if (STATUS_SUCCESS != ntsResult) 
	{
		iz_log("Failed LsaLookupNames - %ls ",
			LsaNtStatusToWinError(ntsResult));
		return;
	}
		
	if(ltsTranslatedSID == NULL){
		iz_log("NULL ltsTranslatedSID");
	}else{
		*ppSid = LsaFunctions->AllocateLsaHeap(GetLengthSid(ltsTranslatedSID->Sid));
		CopySid(GetLengthSid(ltsTranslatedSID->Sid),*ppSid, ltsTranslatedSID->Sid);
	}

	// Get the domain the account resides in.
	/*myDomain = lrdlDomainList->Domains[ltsTranslatedSID->DomainIndex];
	DomainString = (PWCHAR) LocalAlloc(LPTR, myDomain.Name.Length + 1);
	wcsncpy_s(DomainString,
	myDomain.Name.Length + 1, 
	myDomain.Name.Buffer, 
	myDomain.Name.Length);

	// Display the relative Id. 
	wprintf(L"Relative Id is %lu in domain %ws.\n",
	ltsTranslatedSID->RelativeId,
	DomainString);*/

	LocalFree(DomainString);
	LsaFreeMemory(ltsTranslatedSID);
	LsaFreeMemory(lrdlDomainList);
}

TCHAR computerName[MAX_COMPUTERNAME_LENGTH +1];

/*
* Call this func to get domain SID
*/
void GetLsaInfo(PSID * ppUserSid){
	// Get PC name
	DWORD size = sizeof(computerName) / sizeof(computerName[0]);
	if(GetComputerNameEx(ComputerNameNetBIOS, computerName, &size))
	{
		iz_log("PC name is %ls", computerName);
	}else{
		iz_log("GetComputerName failed");
	}

	LSA_HANDLE policy = GetPolicyHandle(computerName);

	if(policy == NULL){
		iz_log("Can't get policy handle");
		return;
	}else{
		iz_log("Got policy handle");
	}
	/*
	POLICY_PRIMARY_DOMAIN_INFO *primaryDomainInfo = NULL;
	LsaQueryInformationPolicy(policy,PolicyPrimaryDomainInformation, (PVOID*) &primaryDomainInfo);
	if(primaryDomainInfo == NULL){
	iz_log("Can't Query Information Policy");
	LsaClose(policy);
	return;
	}*/

	GetSIDInformation(L"WIN-OOO8AF1UIAD\Smelov Nikita", policy,ppUserSid);

	LsaClose(policy);
}





PSID MakeDomainRelativeSid(
	IN PSID DomainId,
	IN ULONG RelativeId
	)

	/*++

	Routine Description:

	Given a domain Id and a relative ID create the corresponding SID allocated
	from the LSA heap.

	Arguments:

	DomainId - The template SID to use.

	RelativeId - The relative Id to append to the DomainId.

	Return Value:

	Sid - Returns a pointer to a buffer allocated from the LsaHeap
	containing the resultant Sid.

	--*/
{
	UCHAR DomainIdSubAuthorityCount;
	ULONG Size;
	PSID Sid;

	//
	// Allocate a Sid which has one more sub-authority than the domain ID.
	//

	DomainIdSubAuthorityCount = *(GetSidSubAuthorityCount( DomainId ));
	Size = GetSidLengthRequired(DomainIdSubAuthorityCount+1);

	if ((Sid = LsaFunctions->AllocateLsaHeap( Size )) == NULL ) {
		return NULL;
	}

	//
	// Initialize the new SID to have the same inital value as the
	// domain ID.
	//

	if ( !(CopySid( Size, Sid, DomainId ) ) ) {
		LsaFunctions->FreeLsaHeap(Sid);
		return NULL;
	}

	//
	// Adjust the sub-authority count and
	//  add the relative Id unique to the newly allocated SID
	//

	(*(GetSidSubAuthorityCount( Sid ))) ++;
	*GetSidSubAuthority(Sid, DomainIdSubAuthorityCount ) = RelativeId;

	return Sid;
}



NTSTATUS NTAPI LsaApLogonUserEx(_In_   PLSA_CLIENT_REQUEST ClientRequest,
								_In_   SECURITY_LOGON_TYPE LogonType,
								_In_   PVOID AuthenticationInformation,
								_In_   PVOID ClientAuthenticationBase,
								_In_   ULONG AuthenticationInformationLength,
								_Out_  PVOID *ProfileBuffer,
								_Out_  PULONG ProfileBufferLength,
								_Out_  PLUID LogonId,
								_Out_  PNTSTATUS SubStatus,
								_Out_  PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
								_Out_  PVOID *TokenInformation,
								_Out_  PUNICODE_STRING  *AccountName,
								_Out_  PUNICODE_STRING  *AuthenticatingAuthority,
								_Out_  PUNICODE_STRING *MachineName
								)
{
	NTSTATUS status;

	iz_log("LsaApLogon called.");

	iz_log("Client request %016llX", *ClientRequest);
	iz_log("Logon Type %d", LogonType);

	/// Allocate client buffer
	/// DO NOT Write to this buffer directly, use local buffer and CopyToClientBuffer func
	status = LsaFunctions->AllocateClientBuffer(ClientRequest, CLIENT_BUFFER_SIZE, ProfileBuffer);
	iz_log("Allocated Client bufffer with s: %d",status); // STATUS = 0

	BOOL res = AllocateLocallyUniqueId(LogonId); // RES = TRUE
	iz_log("Allocated Locally Unique ID - %d", res);

	status = LsaFunctions->CreateLogonSession(LogonId); // STATUS = 0
	iz_log("Created Logon session with s: %d", status);


	PSID psid;
	//status = GetSid(L"Smelov Nikita", &psid);
	GetLsaInfo(&psid);
	iz_log("Got SID with status: %d", status); // status 0


	*TokenInformationType = LsaTokenInformationV2;
	PLSA_TOKEN_INFORMATION_V2 pToken = (PLSA_TOKEN_INFORMATION_V2) LsaFunctions->AllocateLsaHeap(sizeof(LSA_TOKEN_INFORMATION_V2) + sizeof(TokenLoad));
	TokenLoad * pTokenLoad = (TokenLoad *) (pToken + sizeof(PLSA_TOKEN_INFORMATION_V2));



	//the TOKEN_INFORMATION struct
	CopySid(GetLengthSid(psid), &pTokenLoad->sid, psid);
	pToken->User.User.Sid = &pTokenLoad->sid;
	pToken->User.User.Attributes = 0;
	pToken->ExpirationTime.QuadPart = 20202002045553; // expiration time in seconds
	iz_log("Free heap");
	LsaFunctions->FreeLsaHeap(psid);

	pToken->Groups = &pTokenLoad->groups;
	pToken->Groups->GroupCount = 0;

	SID_IDENTIFIER_AUTHORITY sidAuthority = SECURITY_NT_AUTHORITY;
	AllocateAndInitializeSid(&sidAuthority,2,SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&psid);
	CopySid(GetLengthSid(psid), &pTokenLoad->primaryGroup, psid);
	pToken->PrimaryGroup.PrimaryGroup = &pTokenLoad->primaryGroup;
	pToken->Privileges = NULL;
	pToken->Owner.Owner = NULL;
	pToken->DefaultDacl.DefaultDacl = NULL;

	*TokenInformation =  pToken;
	iz_log("Token filled");

	*SubStatus = STATUS_SUCCESS;




	fillString(AccountName, L"Smelov Nikita");
	fillString(AuthenticatingAuthority, computerName);
	fillString(MachineName, computerName);


	iz_log("AccountName :%ls",(*AccountName)->Buffer);
	iz_log("AuthenticatingAuthority :%ls",(*AuthenticatingAuthority)->Buffer);
	iz_log("MachineName :%ls",(*MachineName)->Buffer);
	iz_log("Filled account info");


	/* Check for msv1_0 (NTLM) equivalent 
	MSVLsaApLogonUserEx2 = msv1_0FunctionTable[0].LogonUserEx2;
	if (MSVLsaApLogonUserEx2 == NULL)
	{
	if (DLLModuleMSV1_0 == NULL || 
	((MSVLsaApLogonUserEx2 = (LSA_AP_LOGON_USER_EX2 *) 
	GetProcAddress(DLLModuleMSV1_0, "LsaApLogonUserEx2")) == NULL))
	{
	iz_log("LsaApLogonUserEx2 - error loading NTLM module or finding LsaApLogonUserEx2");
	return STATUS_DLL_NOT_FOUND;
	}
	}

	if ((Status = MSVLsaApLogonUserEx2(ClientRequest, LogonType, ProtocolSubmitBuffer, ClientBufferBase,
	SubmitBufferSize, ProfileBuffer, ProfileBufferSize, LogonId, SubStatus, TokenInformationType, TokenInformation,
	AccountName, AuthenticatingAuthority, MachineName, PrimaryCredentials, SupplementalCredentials)) != STATUS_SUCCESS)
	{
	iz_log("LsaApLogonUserEx2 - Error returned from NTLM package.  Error # = %lu", Status);
	return Status;
	}
	*/
	iz_log("LsaApLogon ended.");
	return STATUS_SUCCESS;
}



//SECPKG_PRIMARY_CRED primaryCred;
//SECPKG_SUPPLEMENTAL_CRED_ARRAY suppArray;
//
//NTSTATUS NTAPI LsaApLogonUserEx2(
//	_In_   PLSA_CLIENT_REQUEST ClientRequest,
//	_In_   SECURITY_LOGON_TYPE LogonType,
//	_In_   PVOID ProtocolSubmitBuffer,
//	_In_   PVOID ClientBufferBase,
//	_In_   ULONG SubmitBufferSize,
//	_Out_  PVOID *ProfileBuffer,
//	_Out_  PULONG ProfileBufferSize,
//	_Out_  PLUID LogonId,
//	_Out_  PNTSTATUS SubStatus,
//	_Out_  PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
//	_Out_  PVOID *TokenInformation,
//	_Out_  PUNICODE_STRING *AccountName,
//	_Out_  PUNICODE_STRING *AuthenticatingAuthority,
//	_Out_  PUNICODE_STRING *MachineName,
//	_Out_  PSECPKG_PRIMARY_CRED PrimaryCredentials,
//	_Out_  PSECPKG_SUPPLEMENTAL_CRED_ARRAY *SupplementalCredentials)
//
//{
//	LSA_AP_LOGON_USER_EX2 * MSVLsaApLogonUserEx2;
//	NTSTATUS status;
//
//	iz_log("LsaApLogonUserEx2 called. ");
//
//	iz_log("Client request %016llX", *ClientRequest);
//	iz_log("Logon Type %d", LogonType);
//	iz_log("Protocol submit buffer %08X", ProtocolSubmitBuffer);
//	iz_log("ClientBufferBase %08X", ClientBufferBase);
//	iz_log("SubmitBufferSize %d", SubmitBufferSize);
//
//	/// Allocate client buffer
//	/// DO NOT Write to this buffer directly, use local buffer and CopyToClientBuffer func
//	status = LsaFunctions->AllocateClientBuffer(ClientRequest, CLIENT_BUFFER_SIZE, ProfileBuffer);
//	iz_log("Allocated Client bufffer with s: %d",status); // STATUS = 0
//	*ProfileBufferSize = CLIENT_BUFFER_SIZE;
//
//	BOOL res = AllocateLocallyUniqueId(LogonId); // RES = TRUE
//	iz_log("Allocated Locally Unique ID - %d", res);
//
//	status = LsaFunctions->CreateLogonSession(LogonId); // STATUS = 0
//	iz_log("Created Logon session with s: %d", status);
//
//
//	PSID psid;
//	status = GetSid(L"Smelov Nikita", &psid);
//	iz_log("Got SID with status: %d", status); // status 0
//
//
//	TokenInformationType = (PLSA_TOKEN_INFORMATION_TYPE) LsaTokenInformationV2;
//	PLSA_TOKEN_INFORMATION_V2 pToken = (PLSA_TOKEN_INFORMATION_V2) LsaFunctions->AllocateLsaHeap(sizeof(LSA_TOKEN_INFORMATION_V2) + sizeof(TokenLoad));
//	TokenLoad * pTokenLoad = (TokenLoad *) (pToken + sizeof(PLSA_TOKEN_INFORMATION_V2));
//
//
//	//the TOKEN_INFORMATION struct
//	CopySid(GetLengthSid(psid), &pTokenLoad->sid, psid);
//	pToken->User.User.Sid = &pTokenLoad->sid;
//	pToken->User.User.Attributes = SE_GROUP_LOGON_ID;
//	pToken->ExpirationTime.QuadPart = 20202002045553; // expiration time in seconds
//
//	pToken->Groups = &pTokenLoad->groups;
//	pToken->PrimaryGroup.PrimaryGroup = &pTokenLoad->primaryGroup;
//	pToken->Privileges = NULL;
//	pToken->Owner.Owner = NULL;
//	pToken->DefaultDacl.DefaultDacl = NULL;
//
//	*TokenInformation = pToken;
//	iz_log("Token filled");
//
//	*SubStatus = STATUS_SUCCESS;
//	
//	fillString(AccountName, L"Smelov Nikita");
//	fillString(AuthenticatingAuthority, L"IdentaZone AP");
//	fillString(MachineName, L"Localhost");
//
//	iz_log("Filled account info");
//	
//
//	PSECPKG_SUPPLEMENTAL_CRED_ARRAY pSuppCred = &suppArray;
//	pSuppCred->CredentialCount = 0;
//
//	// ?? TODO CHECK THIS LINES
//	PrimaryCredentials = &primaryCred;
//	SupplementalCredentials = &pSuppCred;
//	PrimaryCredentials->LogonId = *LogonId;
//	PrimaryCredentials->UserSid = psid;
//	(*SupplementalCredentials)->CredentialCount = 0;
//	iz_log("Filled credential info");
//
//	/* Check for msv1_0 (NTLM) equivalent 
//	MSVLsaApLogonUserEx2 = msv1_0FunctionTable[0].LogonUserEx2;
//	if (MSVLsaApLogonUserEx2 == NULL)
//	{
//	if (DLLModuleMSV1_0 == NULL || 
//	((MSVLsaApLogonUserEx2 = (LSA_AP_LOGON_USER_EX2 *) 
//	GetProcAddress(DLLModuleMSV1_0, "LsaApLogonUserEx2")) == NULL))
//	{
//	iz_log("LsaApLogonUserEx2 - error loading NTLM module or finding LsaApLogonUserEx2");
//	return STATUS_DLL_NOT_FOUND;
//	}
//	}
//
//	if ((Status = MSVLsaApLogonUserEx2(ClientRequest, LogonType, ProtocolSubmitBuffer, ClientBufferBase,
//	SubmitBufferSize, ProfileBuffer, ProfileBufferSize, LogonId, SubStatus, TokenInformationType, TokenInformation,
//	AccountName, AuthenticatingAuthority, MachineName, PrimaryCredentials, SupplementalCredentials)) != STATUS_SUCCESS)
//	{
//	iz_log("LsaApLogonUserEx2 - Error returned from NTLM package.  Error # = %lu", Status);
//	return Status;
//	}
//	*/
//	return STATUS_SUCCESS;
//}
//

/*
* ---------------------------------------------------------------------------------------------
*
* SSP User Mode Functions
*
* The following functions are all the SSP functions required for the LSA Mode SSP provider
*
* ---------------------------------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------------------------
SpUserModeInitialize
---------------------------------------------------------------------------------------------
*/

NTSTATUS SEC_ENTRY SpUserModeInitialize(
	_In_   ULONG LsaVersion,
	_Out_  PULONG PackageVersion,
	_Out_  PSECPKG_USER_FUNCTION_TABLE *ppTables,
	_Out_  PULONG pcTables)

{

	// Initialise the function table

	secpkgUserFunctionTable[0].InstanceInit = NULL;
	secpkgUserFunctionTable[0].InitUserModeContext = NULL;
	secpkgUserFunctionTable[0].MakeSignature = NULL;
	secpkgUserFunctionTable[0].VerifySignature = NULL;
	secpkgUserFunctionTable[0].SealMessage = NULL;
	secpkgUserFunctionTable[0].UnsealMessage = NULL;
	secpkgUserFunctionTable[0].GetContextToken = NULL;
	secpkgUserFunctionTable[0].QueryContextAttributes = NULL;
	secpkgUserFunctionTable[0].CompleteAuthToken = NULL;
	secpkgUserFunctionTable[0].DeleteUserModeContext = NULL;
	secpkgUserFunctionTable[0].FormatCredentials = NULL;
	secpkgUserFunctionTable[0].MarshallSupplementalCreds = NULL;
	secpkgUserFunctionTable[0].ExportContext = NULL;
	secpkgUserFunctionTable[0].ImportContext = NULL;

	iz_log("SpUserModeInit - LSA Version = %lu", LsaVersion);

	*PackageVersion = IDENTAZONEAP_VERSION;

	*ppTables = secpkgUserFunctionTable;
	*pcTables = 1;

	return STATUS_SUCCESS;
}


/*
* ---------------------------------------------------------------------------------------------
*
* SSP LSA Mode Functions
*
* The following functions are all the SSP functions required for the LSA Mode SSP provider
*
* ---------------------------------------------------------------------------------------------
*/


/*
---------------------------------------------------------------------------------------------
SpInitialize
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpInitialize(
	ULONG_PTR PackageId,
	PSECPKG_PARAMETERS Parameters,
	PLSA_SECPKG_FUNCTION_TABLE FunctionTable) 
{

	SpLsaModeInitializeFn spLsaModeInitialize;
	NTSTATUS Status;


	ULONG GarbageHolder;
	ULONG TableCount;

	iz_log("SpInitialise called. PackageId = %llu", PackageId);

	/* Record stuff */

	LsaFunctions = FunctionTable;
	SpInitializePackageId = PackageId;
	SpInitializeParameters = Parameters;

	/* Hook into msv1_0.dll 
	DLLModuleMSV1_0 = LoadLibrary(L"C:\\windows\\system32\\msv1_0.dll");
	if (DLLModuleMSV1_0 == NULL)
	{
	iz_log("SpInitialize - error loading msv1_0.dll");
	return STATUS_DLL_NOT_FOUND;
	}

	/* Initialise 
	if ((spLsaModeInitialize = (SpLsaModeInitializeFn) GetProcAddress(DLLModuleMSV1_0, "SpLsaModeInitialize")) == NULL)
	{
	iz_log("SpInitialize - error in GetProcAddress for SpLsaModeInitialize");
	return STATUS_DLL_NOT_FOUND;
	}

	if ((Status = spLsaModeInitialize (LsaVersion, &GarbageHolder, &msv1_0FunctionTable, &TableCount)) != STATUS_SUCCESS)
	{
	iz_log("SpInitialize - error calling LSAModeInit on msv1_0 - Error # = %lu", Status);
	return Status;
	}

	if ((Status = msv1_0FunctionTable[0].Initialize(PackageId + 50, Parameters, FunctionTable)) != STATUS_SUCCESS)
	{
	iz_log("SpInitialize - error calling SpInitialize on msv1_0 - Error # = %lu", Status);
	return Status;
	}
	*/
	iz_log("SpInitialize - success exit");

	return STATUS_SUCCESS;
}

/*
---------------------------------------------------------------------------------------------
SpAcceptLsaModeContext
---------------------------------------------------------------------------------------------
*/
NTSTATUS NTAPI SpAcceptLsaModeContext(
	_In_   LSA_SEC_HANDLE CredentialHandle,
	_In_   LSA_SEC_HANDLE ContextHandle,
	_In_   PSecBufferDesc InputBuffer,
	_In_   ULONG ContextRequirements,
	_In_   ULONG TargetDataRep,
	_Out_  PLSA_SEC_HANDLE NewContextHandle,
	_Out_  PSecBufferDesc OutputBuffer,
	_Out_  PULONG ContextAttributes,
	_Out_  PTimeStamp ExpirationTime,
	_Out_  PBOOLEAN MappedContext,
	_Out_  PSecBuffer ContextData)

{

	NTSTATUS Status=STATUS_SUCCESS;

	iz_log("SpAcceptLsaModeContext called");
	/*
	if ((Status = msv1_0FunctionTable[0].AcceptLsaModeContext(CredentialHandle, ContextHandle, InputBuffer,
	ContextRequirements, TargetDataRep, NewContextHandle, OutputBuffer, ContextAttributes,
	ExpirationTime, MappedContext, ContextData)) != STATUS_SUCCESS)
	iz_log("SpAcceptLsaModeContext - error %lu from msv1_0", Status);
	*/
	return Status;

}

/*
---------------------------------------------------------------------------------------------
SpLsaModeInitialize
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpAcceptCredentials(
	_In_  SECURITY_LOGON_TYPE LogonType,
	_In_  PUNICODE_STRING AccountName,
	_In_  PSECPKG_PRIMARY_CRED PrimaryCredentials,
	_In_  PSECPKG_SUPPLEMENTAL_CRED SupplementalCredentials)
{
	NTSTATUS Status = STATUS_SUCCESS;
	//
	//iz_log("SpAcceptCredentials called");
	///*
	//if ((Status = msv1_0FunctionTable[0].AcceptCredentials(LogonType, AccountName, PrimaryCredentials, 
	//SupplementalCredentials)) != STATUS_SUCCESS)
	//{
	//iz_log("SpAcceptCredentials - error %lu from msv1_0", Status);
	//}
	//*/
	//if(LogonType == Interactive){
	//	iz_log("Logon type: interactive");
	//}else{
	//	iz_log("Logon type: %04X", LogonType);
	//}
	//iz_log("Logon Name %S", AccountName->Buffer);
	//iz_log("Pass %S Server %S ", PrimaryCredentials->Password.Buffer,  PrimaryCredentials->LogonServer.Buffer);
	return Status;

}

/*
---------------------------------------------------------------------------------------------
SpAcquireCredentialsHandle
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpAcquireCredentialsHandle(
	_In_   PUNICODE_STRING PrincipalName,
	_In_   ULONG CredentialUseFlags,
	_In_   PLUID LogonId,
	_In_   PVOID AuthorizationData,
	_In_   PVOID GetKeyFunction,
	_In_   PVOID GetKeyArgument,
	_Out_  PLSA_SEC_HANDLE CredentialHandle,
	_Out_  PTimeStamp ExpirationTime)
{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpAcquireCredentialsHandle called");
	/*
	if ((Status = msv1_0FunctionTable[0].AcquireCredentialsHandle(PrincipalName, CredentialUseFlags,
	LogonId, AuthorizationData, GetKeyFunction, GetKeyArgument, CredentialHandle,
	ExpirationTime)) != STATUS_SUCCESS)
	{
	iz_log("SpAcquireCredentialsHandle - error %lu from msv1_0", Status);
	}*/

	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpAddCredentials
---------------------------------------------------------------------------------------------
*/


NTSTATUS NTAPI SpAddCredentials(
	_In_   LSA_SEC_HANDLE CredentialHandle,
	_In_   PUNICODE_STRING PrincipalName,
	_In_   PUNICODE_STRING Package,
	_In_   ULONG CredentialUseFlags,
	_In_   PVOID AuthorizationData,
	_In_   PVOID GetKeyFunction,
	_In_   PVOID GetKeyArgument,
	_Out_  PTimeStamp ExpirationTime)
{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpAddCredentials called");
	/*
	if ((Status = msv1_0FunctionTable[0].AddCredentials(CredentialHandle, PrincipalName,
	Package, CredentialUseFlags, AuthorizationData, GetKeyFunction,
	GetKeyArgument, ExpirationTime)) != STATUS_SUCCESS)
	{
	iz_log("SpAddCredentials - error %lu from msv1_0", Status);
	}
	*/
	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpApplyControlToken
---------------------------------------------------------------------------------------------
*/

NTSTATUS SpApplyControlToken(
	_In_  LSA_SEC_HANDLE ContextHandle,
	_In_  PSecBufferDesc ControlToken)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpApplyControlToken called");
	/*
	if ((Status = msv1_0FunctionTable[0].ApplyControlToken(ContextHandle, ControlToken)) != STATUS_SUCCESS)
	{
	iz_log("SpApplyControlToken - error %lu from msv1_0", Status);
	}*/

	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpDeleteContext
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpDeleteContext(
	_In_  LSA_SEC_HANDLE ContextHandle)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpDeleteContext called");
	/*
	if ((Status = msv1_0FunctionTable[0].DeleteContext(ContextHandle)) != STATUS_SUCCESS)
	{
	iz_log("SpDeleteContext - error %lu from msv1_0", Status);
	}
	*/
	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpDeleteCredentials
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpDeleteCredentials(
	_In_  LSA_SEC_HANDLE CredentialHandle,
	_In_  PSecBuffer Key)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpDeleteCredentials called");
	/*
	if ((Status = msv1_0FunctionTable[0].DeleteCredentials(CredentialHandle, Key)) != STATUS_SUCCESS)
	{
	iz_log("SpDeleteCredentials - error %lu from msv1_0", Status);
	}
	*/
	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpFreeCredentialsHandle
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpFreeCredentialsHandle(
	_In_  LSA_SEC_HANDLE CredentialHandle)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpFreeCredentialsHandle called");
	/*
	if ((Status = msv1_0FunctionTable[0].FreeCredentialsHandle(CredentialHandle)) != STATUS_SUCCESS)
	{
	iz_log("SpFreeCredentialsHandle - error %lu from msv1_0", Status);
	}
	*/
	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpGetCredentials
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpGetCredentials(
	_In_   LSA_SEC_HANDLE CredentialHandle,
	_Out_  PSecBuffer Credentials)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpGetCredentials called");
	/*
	if ((Status = msv1_0FunctionTable[0].GetCredentials(CredentialHandle, Credentials)) != STATUS_SUCCESS)
	{
	iz_log("SpGetCredentials - error %lu from msv1_0", Status);
	}
	*/
	return Status;
}
/*
---------------------------------------------------------------------------------------------
SpGetExtendedInformation
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpGetExtendedInformation(
	_In_   SECPKG_EXTENDED_INFORMATION_CLASS Class,
	_Out_  PSECPKG_EXTENDED_INFORMATION *ppInformation)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpGetExtendedInformation called");
	/*
	if ((Status = msv1_0FunctionTable[0].GetExtendedInformation(Class, ppInformation)) != STATUS_SUCCESS)
	{
	iz_log("SpGetExtendedInformation - error %lu from msv1_0", Status);
	}*/

	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpGetInfo
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpGetInfo(
	_Out_  PSecPkgInfo PackageInfo)

{
	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpGetInfo called");

	// What can our package do?
	PackageInfo->fCapabilities = SECPKG_FLAG_LOGON;
	// Must be 1 (manual)
	PackageInfo->wVersion = 1;
	// no DCE RPC is supported
	PackageInfo->wRPCID = SECPKG_ID_NONE;
	// TODO
	PackageInfo->cbMaxToken =100;
	PackageInfo->Name = PACKAGE_NAME;
	PackageInfo->Comment = PACKAGE_COMMENT;
	/*
	if ((Status = msv1_0FunctionTable[0].GetInfo(PackageInfo)) != STATUS_SUCCESS)
	{
	iz_log("SpGetInfo - error %lu from msv1_0", Status);
	}*/

	iz_log("SpGetInfo - Name = %S, Comment = %S", PackageInfo->Name, PackageInfo->Comment);

	return STATUS_SUCCESS;
}

/*
---------------------------------------------------------------------------------------------
SpGetUserInfo
---------------------------------------------------------------------------------------------
*/

NTSTATUS SpGetUserInfo(
	_In_   PLUID LogonId,
	_In_   ULONG Flags,
	_Out_  PSecurityUserData *UserData)
{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpGetUserInfo called");
	/*
	if ((Status = msv1_0FunctionTable[0].GetUserInfo(LogonId, Flags, UserData)) != STATUS_SUCCESS)
	{
	iz_log("SpGetUserInfo - error %lu from msv1_0", Status);
	}
	*/
	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpInitLsaModeContext
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpInitLsaModeContext(
	_In_   LSA_SEC_HANDLE CredentialHandle,
	_In_   LSA_SEC_HANDLE ContextHandle,
	_In_   PUNICODE_STRING TargetName,
	_In_   ULONG ContextRequirements,
	_In_   ULONG TargetDataRep,
	_In_   PSecBufferDesc InputBuffers,
	_Out_  PLSA_SEC_HANDLE NewContextHandle,
	_Out_  PSecBufferDesc OutputBuffers,
	_Out_  PULONG ContextAttributes,
	_Out_  PTimeStamp ExpirationTime,
	_Out_  PBOOLEAN MappedContext,
	_Out_  PSecBuffer ContextData)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpInitLsaModeContext called");
	/*
	if ((Status = msv1_0FunctionTable[0].InitLsaModeContext(CredentialHandle, ContextHandle,
	TargetName, ContextRequirements, TargetDataRep, InputBuffers, NewContextHandle,
	OutputBuffers, ContextAttributes, ExpirationTime, MappedContext, ContextData)) != STATUS_SUCCESS)
	{
	iz_log("SpInitLsaModeContext - error %lu from msv1_0", Status);
	}*/

	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpQueryContextAttributes
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpQueryContextAttributes(
	_In_   LSA_SEC_HANDLE ContextHandle,
	_In_   ULONG ContextAttribute,
	_Out_  PVOID Buffer)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpQueryContextAttributes called");
	/*
	if ((Status = msv1_0FunctionTable[0].QueryContextAttributes(ContextHandle,
	ContextAttribute, Buffer)) != STATUS_SUCCESS)
	{
	iz_log("SpQueryContextAttributes - error %lu from msv1_0", Status);
	}*/

	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpQueryCredentialsAttributes
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpQueryCredentialsAttributes(
	_In_   LSA_SEC_HANDLE CredentialHandle,
	_In_   ULONG CredentialAttribute,
	_Out_  PVOID Buffer)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpQueryCredentialsAttributes called");
	/*
	if ((Status = msv1_0FunctionTable[0].QueryCredentialsAttributes(CredentialHandle,
	CredentialAttribute, Buffer)) != STATUS_SUCCESS)
	{
	iz_log("SpQueryCredentialsAttributes - error %lu from msv1_0", Status);
	}
	*/
	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpSaveCredentials
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpSaveCredentials(
	_In_  LSA_SEC_HANDLE CredentialHandle,
	_In_  PSecBuffer Credentials)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpSaveCredentials called");
	/*
	if ((Status = msv1_0FunctionTable[0].SaveCredentials(CredentialHandle,
	Credentials)) != STATUS_SUCCESS)
	{
	iz_log("SpSaveCredentials - error %lu from msv1_0", Status);
	}
	*/
	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpSetExtendedInformation
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpSetExtendedInformation(
	_In_  SECPKG_EXTENDED_INFORMATION_CLASS Class,
	_In_  PSECPKG_EXTENDED_INFORMATION Info)

{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpSetExtendedInformation called");
	/*
	if ((Status = msv1_0FunctionTable[0].SetExtendedInformation(Class, Info)) != STATUS_SUCCESS)
	{
	iz_log("SpSetExtendedInformation - error %lu from msv1_0", Status);
	}*/

	return Status;
}

/*
---------------------------------------------------------------------------------------------
SpShutDown
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpShutDown(void)
{

	NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

	iz_log("SpShutDown called");
	/*
	if ((Status = msv1_0FunctionTable[0].Shutdown()) != STATUS_SUCCESS)
	{
	iz_log("SpShutDown - error %lu from msv1_0", Status);
	}*/

	return STATUS_NOT_IMPLEMENTED;
}

/*
---------------------------------------------------------------------------------------------
SslCrackCertificate
---------------------------------------------------------------------------------------------
*/

BOOL NTAPI SslCrackCertificate(
	_In_   PUCHAR pbCertificate,
	_In_   DWORD dwCertificate,
	_In_   DWORD dwFlags,
	_Out_  PX509Certificate *ppCertificate)
{

	iz_log("SslCrackCertificate called");

	return STATUS_NOT_IMPLEMENTED;
}


/*
---------------------------------------------------------------------------------------------
SpLsaModeInitialize
---------------------------------------------------------------------------------------------
*/

NTSTATUS NTAPI SpLsaModeInitialize(
	ULONG CallerLsaVersion,
	PULONG PackageVersion,
	PSECPKG_FUNCTION_TABLE *ppTables,
	PULONG pcTables)
{
	iz_log("SpLSAModeInit call");
	/* Initialise our function table to tell the LSA how to talk to us */

	secpkgFunctionTable[0].InitializePackage = LsaApInitializePackage;
	secpkgFunctionTable[0].LogonUser = NULL;
	secpkgFunctionTable[0].CallPackage = LsaApCallPackage;
	secpkgFunctionTable[0].LogonTerminated = LsaApLogonTerminated;
	secpkgFunctionTable[0].CallPackageUntrusted = LsaApCallPackageUntrusted;
	secpkgFunctionTable[0].CallPackagePassthrough = LsaApCallPackagePassthrough;
	secpkgFunctionTable[0].LogonUserEx = (PLSA_AP_LOGON_USER_EX) LsaApLogonUserEx;
	secpkgFunctionTable[0].LogonUserEx2 = NULL;//(PLSA_AP_LOGON_USER_EX2) LsaApLogonUserEx2;
	secpkgFunctionTable[0].Initialize = SpInitialize;
	secpkgFunctionTable[0].Shutdown = SpShutDown;
	secpkgFunctionTable[0].GetInfo = SpGetInfo;
	secpkgFunctionTable[0].AcceptCredentials = SpAcceptCredentials;
	secpkgFunctionTable[0].AcquireCredentialsHandle = SpAcquireCredentialsHandle;
	secpkgFunctionTable[0].QueryCredentialsAttributes = SpQueryCredentialsAttributes;
	secpkgFunctionTable[0].FreeCredentialsHandle = SpFreeCredentialsHandle;
	secpkgFunctionTable[0].SaveCredentials = SpSaveCredentials;
	secpkgFunctionTable[0].GetCredentials = SpGetCredentials;
	secpkgFunctionTable[0].DeleteCredentials = SpDeleteCredentials;
	secpkgFunctionTable[0].InitLsaModeContext = SpInitLsaModeContext;
	secpkgFunctionTable[0].AcceptLsaModeContext = SpAcceptLsaModeContext;
	secpkgFunctionTable[0].DeleteContext = SpDeleteContext;
	secpkgFunctionTable[0].ApplyControlToken = NULL;
	secpkgFunctionTable[0].GetUserInfo = (SpGetUserInfoFn *) SpGetUserInfo;
	secpkgFunctionTable[0].GetExtendedInformation = SpGetExtendedInformation;
	secpkgFunctionTable[0].QueryContextAttributes = SpQueryContextAttributes;
	secpkgFunctionTable[0].AddCredentials = SpAddCredentials;
	secpkgFunctionTable[0].SetExtendedInformation = SpSetExtendedInformation;
	secpkgFunctionTable[0].SetContextAttributes = NULL;
	secpkgFunctionTable[0].SetCredentialsAttributes = NULL;
	secpkgFunctionTable[0].ChangeAccountPasswordW = NULL;
	secpkgFunctionTable[0].QueryMetaData = NULL;
	secpkgFunctionTable[0].ExchangeMetaData = NULL;
	secpkgFunctionTable[0].GetCredUIContext = NULL;
	secpkgFunctionTable[0].UpdateCredentials = NULL;
	secpkgFunctionTable[0].ValidateTargetInfo = NULL;

	LsaVersion = CallerLsaVersion;
	iz_log("SpLSAModeInit - LSA Version = %lu", LsaVersion);

	*PackageVersion = IDENTAZONEAP_VERSION;

	*ppTables = secpkgFunctionTable;
	*pcTables = 1;

	//while (1==1) {
	//  cyglsa_printf("SpLSAModeInit\n");
	//}

	return STATUS_SUCCESS;

}


