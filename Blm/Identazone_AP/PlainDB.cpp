#include "PlainDB.h"
#include "encryptor.h"
#include <iostream>
////////
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

typedef unsigned char BYTE;

static inline bool is_base64(BYTE c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}



std::vector<BYTE> base64_decode(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	BYTE char_array_4[4], char_array_3[3];
	std::vector<BYTE> ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
	}

	return ret;
}


CPlainDB::CPlainDB(void) {
    _isActive = false;
    _usr =  NULL;
    _xml = NULL;
}


CPlainDB::~CPlainDB(void) {
    delete[] _xml;
}

void CPlainDB::DecryptOpenAES(int size) {
    encryptor::WinapiEncryptor testEncryptor(encryptor::AES_128);
    encryptor::AbstractEncryptor* abstractEncryptor = &testEncryptor;
    DATA_BLOB dataIn;
    DATA_BLOB dataOut;
    DATA_BLOB entropy;


    dataIn.cbData = size;
    dataIn.pbData = (BYTE*)_xml;

    unsigned char entropyBytes[] = {0x00, 0x01, 0x02, 0x01, 0x00, 0x04, 0xBB, 0x17, 0x8b, 0xf6, 0xa2, 0x15, 0xe2, 0x64, 0x11, 0x9a};
    entropy.cbData = sizeof(entropyBytes);
    entropy.pbData = entropyBytes;

    dataOut.cbData = size;
    dataOut.pbData = new unsigned char[size];

    try {
        abstractEncryptor->decrypt(&dataIn, &entropy, &dataOut);
        // add '0' to fileEnd
        //dataOut.pbData[dataOut.cbData] = 0;
        try {
            _doc.parse<0>((char*)dataOut.pbData);    // 0 means default parse flags
            //_doc.parse<0>((char*)_xml);
            delete _xml;
            _xml = (char*) dataOut.pbData;
        } catch(rapidxml::parse_error e) {
            //LOG(ERROR) << "Rapidxml parse error " << e.what() << " in " << e.where<char>();
	    }
    } catch(const std::exception &e) {
        delete dataOut.pbData;
        DWORD lastError = GetLastError();
        //LOG(ERROR) << "Decrypt parse error " << e.what() << " ErrorCode" << lastError;
    }
}

void CPlainDB::Init(TCHAR* config, ENCRYPT_TYPE encryptType) {
    TCHAR sysPath[MAX_PATH];
    TCHAR path[MAX_PATH];
    GetSystemWindowsDirectory(sysPath, MAX_PATH);
    FILE* fp = NULL;

    // LOCK file
    wcscpy_s(path, MAX_PATH, sysPath);
    wcscat_s(path, MAX_PATH, L"\\System32\\IdentaZone\\lock.txt");
    fp = NULL;
    _wfopen_s(&fp, path, L"r");
    if(NULL != fp) {
        _isActive = true;
        fclose(fp);
    } else {
        _isActive = false;
    }


    int size = 0;
    wcscpy_s(path, MAX_PATH, config);
    fp = NULL;
    errno_t err  = _wfopen_s(&fp, path, L"rb");
    if(NULL != fp) {
        //LOG(INFO) << "Userdb opened";
        fseek(fp, 0L, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        _xml = new char[size + 3]; // UTF-8 file endian
        size = fread(_xml, 1, size, fp);
        fclose(fp);


        switch(encryptType) {
            case ENCRYPT_AES_OPEN_AES:
                DecryptOpenAES(size);
                break;
            case ENCRYPT_NONE:
                _xml[size] = 0;
                try {
                    _doc.parse<0>((char*)_xml);    // 0 means default parse flags
                } catch(rapidxml::parse_error e) {
                    //LOG(ERROR) << "Rapidxml parse error " << e.what() << " in " << e.where<char>();
                }
                break;
        }

    } else {
        //LOG(ERROR) << "Can't open user db " << path;
    }

    return;
}

void CPlainDB::Init(void) {
    TCHAR sysPath[MAX_PATH];
    TCHAR path[MAX_PATH];
    GetSystemWindowsDirectory(sysPath, MAX_PATH);
    wcscpy_s(path, MAX_PATH, sysPath);
    wcscat_s(path, MAX_PATH, L"\\System32\\IdentaZone\\usrdb");
    Init(path, CPlainDB::ENCRYPT_AES_OPEN_AES);
    return;
}


bool CPlainDB::IsActive(void) {
    return _isActive;
}

bool CPlainDB::GetPassword(TCHAR* password, size_t maxLen) {
    if(NULL == _usr) {
        return false;
    } else {
        xml_node<>* passNode = _usr->first_node("Password");
        if(passNode == NULL) {
            return false;
        }
        size_t charNum;
        mbstowcs_s(&charNum, password, maxLen, passNode->value(), passNode->value_size());
        return true;
    }
}

bool CPlainDB::GetSID(TCHAR* SID, size_t maxLen) {
	if(NULL == _usr) {
		return false;
	} else {
		xml_node<>* SIDNode = _usr->first_node("SID");
		if(SIDNode == NULL) {
			return false;
		}
		size_t charNum;
		mbstowcs_s(&charNum, SID, maxLen, SIDNode->value(), SIDNode->value_size());
		return true;
	}
}
bool CPlainDB::GetFullName(TCHAR* name, size_t maxLen) {
    if(NULL == _usr) {
        return false;
    } else {
        xml_node<>* fullNameNode = _usr->first_node("FullName");
        if(NULL == fullNameNode) {
            return false;
        }
        if(fullNameNode->value_size() == 0) {
            return false;
        }

        size_t charNum;
        mbstowcs_s(&charNum, name, maxLen, fullNameNode->value(), fullNameNode->value_size());
    }
    return true;
}

bool CPlainDB::GetLoginType(LOGIN_TYPE* loginType) {
    if(NULL == _usr) {
        return false;
    } else {
        xml_node<>* loginNode = _usr->first_node("Identification");
        if(NULL == loginNode) {
            return false;
        }
        *loginType = LOGIN_UNKNOWN;
        if(strcmp(loginNode->value(), "Password") == 0) {
            *loginType = LOGIN_PASSWORD;
        }
        if(strcmp(loginNode->value(), "Biometrics") == 0) {
            *loginType = LOGIN_BIOMETRICS;
        }
        if(strcmp(loginNode->value(), "Password OR Biometrics") == 0) {
            *loginType = LOGIN_MIXED;
        }
        return true;
    }
}

bool CPlainDB::GetUsername(TCHAR* name, size_t maxLen) {
    if(NULL == _usr) {
        xml_node<>* tmpNode = _doc.first_node();
        if(tmpNode == NULL) {
            return false;
        }
        tmpNode = tmpNode->first_node("WinUsers");
        if(tmpNode == NULL) {
            return false;
        }
        _usr = tmpNode->first_node("User");
    } else {
        _usr = _usr->next_sibling("User", strlen("User"), true);
    }
    if(NULL == _usr) {
        //LOG(INFO) << "Username iterator no such USER node";
        return false;
    } else {
        xml_node<>* usrname = _usr->first_node("Name");
        if(NULL == name) {
            //LOG(ERROR) << "Username iterator no such NAME node";
            return false;
        } else {
            size_t charNum;
            mbstowcs_s(&charNum, name, maxLen, usrname->value(), usrname->value_size());
        }
        //LOG(INFO) << "Username" << usrname->value();
        return true;
    }
}

void CPlainDB::ResetUsername() {
    _usr = NULL;
    //LOG(INFO) << "Username iterator reset";
}

int CPlainDB::GetFmt(std::vector<std::shared_ptr<blm_login::FingerprintTemplate>>& templateVector) {
    std::vector<BYTE> decode;
    unsigned int size;
    unsigned int fingerCount = 0;
    unsigned int fingerIndex = 0;
    if(NULL == _usr) {
        return -1;
    } else {
        xml_node<>* credentials = _usr->first_node("credentials_List");
        xml_node<>* credential;
        if(credentials == NULL) {
            return -1;
        }

        for(credential = credentials->first_node("credential"); credential != NULL; credential = credential->next_sibling()) {
            xml_node<>* fingers = credential->first_node("Fingers");
            if(fingers == NULL) {
                return -3;
            }
            xml_node<>* finger = fingers->first_node("Finger");
            while(finger != NULL) {
                xml_node<>* fingerNum = finger->first_node("FingerNum");				
                xml_node<>* bytes = finger->first_node("Bytes");
                xml_node<>* typeName = finger->first_node("Type");
                if(NULL == fingerNum || NULL == bytes){
					//LOG(ERROR) << "Can't read fingerNum or bytes from db";
                    return -4;
                }
		     	if(NULL == typeName){
					//LOG(ERROR) << "Can't read typename from db";
				    return -4;
		        }

                sscanf_s(fingerNum->value(), "%d", &fingerIndex);
                if(fingerIndex > 11){ // || fingerCount > 9) {
                    return -5;
                }

                
				//decode.resize(bytes->value_size());
				//if(decode.empty()){
				//	return -6;
				//}

				decode = base64_decode((char*)bytes->value());
				size = decode.size();
				//LOG(INFO) << "decoded FMD from base64 with " << size << " size";

				std::shared_ptr<blm_login::FingerprintTemplate> fpTemplate(new blm_login::FingerprintTemplate);
				fpTemplate->data.assign(decode.begin(),decode.end());
				if(fpTemplate->data.empty()){
					return -6;
				}
				if(size<MAX_FMD_SIZE){
					fpTemplate->data.resize(MAX_FMD_SIZE);
				}
                //*fmtLen = size;


                // copy type
                fpTemplate->tmplType.assign((char*)typeName->value());
				templateVector.push_back(fpTemplate);

                finger = finger->next_sibling("Finger");
                fingerCount++;
            }
        }
    }
    return fingerCount;
}


int CPlainDB::GetFmt(std::vector<std::shared_ptr<blm_login::FingerprintTemplate>>& templateVector, char* provider) {
    std::vector<BYTE> decode;
    unsigned int size;
    unsigned int fingerCount = 0;
    unsigned int fingerIndex = 0;
    if(NULL == _usr) {
        return -1;
    } else {
        xml_node<>* credentials = _usr->first_node("credentials_List");
        xml_node<>* credential;
        if(credentials == NULL) {
            return -2;
        }

        for(credential = credentials->first_node("credential"); credential != NULL; credential = credential->next_sibling()) {
            //if(!strcmp(credential->first_attribute("xsi:type")->value(), provider))
            //  break;

            xml_node<>* fingers = credential->first_node("Fingers");
            if(fingers == NULL) {
                return -3;
            }
            xml_node<>* finger = fingers->first_node("finger");
            while(finger != NULL) {
                xml_node<>* fingerNum = finger->first_node("fingerNum");
                xml_node<>* bytes = finger->first_node("bytes");
                xml_node<>* typeName = finger->first_node("type");
                if(NULL == fingerNum || NULL == bytes || NULL == typeName) {
                    return -4;
                }
                sscanf_s(fingerNum->value(), "%d", &fingerIndex);
                if(fingerIndex > 9){ // || fingerCount > 9) {
                    return -5;
                }
				//decode.resize(bytes->value_size());
				//if(decode.empty()){
				//	return -6;
				//}

				decode = base64_decode((char*)bytes->value());
				size = decode.size();
				std::shared_ptr<blm_login::FingerprintTemplate> fpTemplate(new blm_login::FingerprintTemplate);
				fpTemplate->data.assign(decode.begin(),decode.end());
				if(0 == fpTemplate->data.size()){
					return -6;
				}
                //*fmtLen = size;

                // copy type
                fpTemplate->tmplType.assign((char*)typeName->value());
				templateVector.push_back(fpTemplate);
                //
                
                finger = finger->next_sibling("finger");
                fingerCount++;
            }

        }
        return fingerCount;
    }

}

bool CPlainDB::GetBiosecEntropy( std::vector<char>& entropy )
{
	if(NULL == _usr) {
		return false;
	} else {
		xml_node<>* entropyNode = _usr->first_node("BioSecureEntropy");
		if(NULL == entropyNode) {
			return false;
		}

		char* entropyChar = static_cast<char*>(entropyNode->value());
		std::vector<BYTE> decode;
		decode = base64_decode(entropyChar);
		auto decodeResultArraySize = decode.size();

		entropy.assign(decode.begin(), decode.begin()+decodeResultArraySize);
		return true;
	}
}

