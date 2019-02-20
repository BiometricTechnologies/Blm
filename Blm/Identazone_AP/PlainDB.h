#pragma once

#include <Windows.h>

#include <stdio.h>
#include <memory>
#include "fp_device.h"

// DigitalPersona API
#include <dpfpdd.h>
#include <dpfj.h>

#include "rapidxml-1.13\rapidxml.hpp"
using namespace rapidxml;

#define MAX_USR_NAME 255

class CPlainDB
{
private:
	void DecryptOpenAES(int size);
public:
	typedef enum{
		LOGIN_UNKNOWN,
		LOGIN_BIOMETRICS,
		LOGIN_PASSWORD,
		LOGIN_MIXED
	}LOGIN_TYPE;

	typedef enum{
		ENCRYPT_NONE,
		ENCRYPT_AES_OPEN_SSL,
		ENCRYPT_AES_OPEN_AES
	}ENCRYPT_TYPE;
	CPlainDB(void);
	~CPlainDB(void);

	void Init(void);
	void Init(TCHAR * config, ENCRYPT_TYPE encryptType);

	bool IsActive();
	bool GetUsername(TCHAR * name, size_t maxLen);
	bool GetPassword(TCHAR * password, size_t maxLen);
	bool GetSID(TCHAR* SID, size_t maxLen);
	bool GetLoginType(LOGIN_TYPE * loginType);
	bool GetFullName(TCHAR * name, size_t maxLen);
	bool GetBiosecEntropy(std::vector<char>& entropy);
	int GetFmt(std::vector<std::shared_ptr<blm_login::FingerprintTemplate>>&);
	int GetFmt(std::vector<std::shared_ptr<blm_login::FingerprintTemplate>>&, char* provider);
	void ResetUsername();
private:
	bool _isActive;
	// Pointer to string, containing DB
	char * _xml;
	// XML document
	xml_document<> _doc;    // character type defaults to char
	// Pointer to current user
	xml_node<> * _usr;
};

