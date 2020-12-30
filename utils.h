// Steganos LockNote - self-modifying encrypted notepad
// Copyright (C) 2006-2010 Steganos GmbH
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#pragma once

#if _MSC_VER >= 1300 // for VC 7.0
	#ifndef _delayimp_h
	extern "C" IMAGE_DOS_HEADER __ImageBase;
	#endif
#endif

std::string GetPasswordDlg(HWND hWnd = NULL);
std::string GetNewPasswordDlg(HWND hWnd = NULL);

typedef struct wintraits_t
{
	int m_nWindowSizeX;
	int m_nWindowSizeY;
	int m_nFontSize;
	std::string m_strFontName;
} LOCKNOTEWINTRAITS, *LPLOCKNOTEWINTRAITS;

namespace Utils
{
	using namespace CryptoPP;

	HMODULE GetModuleHandle(void)
	{
#if _MSC_VER < 1300 // earlier than .NET compiler (VC 6.0)
		MEMORY_BASIC_INFORMATION mbi;
		static int dummy;
		VirtualQuery( &dummy, &mbi, sizeof(mbi) );

		return reinterpret_cast<HMODULE>(mbi.AllocationBase);
#else // VC 7.0
		return reinterpret_cast<HMODULE>(&__ImageBase);
#endif
	}

	std::string STR(UINT nResourceID)
	{
		std::string text;
		text.resize(16384);
		LoadString(GetModuleHandle(), nResourceID, (char*)text.c_str(), 16384);
		return text;
	}

	int MessageBox(HWND hWnd, const std::string& text, UINT uType)
	{
		return ::MessageBox(hWnd, text.c_str(), STR(IDR_MAINFRAME).c_str(), uType);
	}

	bool UpdateResource(const std::string& strExePath, const std::string& strResourceName, const std::string& strResourceSection, std::vector<unsigned char>& arrayBuffer)
	{
		bool bResult = false;
		HANDLE hFile = ::BeginUpdateResource(strExePath.c_str(), false);
		if (hFile)
		{
			bResult = ::UpdateResource(hFile, strResourceSection.c_str(), strResourceName.c_str(), LANG_NEUTRAL, &arrayBuffer[0], arrayBuffer.size())?true:false;

			if (!::EndUpdateResource(hFile, false))
			{
				bResult = false;
			}
		}
		return bResult;
	}

	bool UpdateResource(const std::string& strExePath, const std::string& strResourceName, const std::string& strResourceSection, const std::string& strText)
	{
		std::vector<unsigned char> arrayBuffer;
		size_t dwSize = (strText.size()+1) * sizeof(char);
		arrayBuffer.resize(dwSize);
		memcpy(&arrayBuffer[0], strText.c_str(), dwSize);
		return UpdateResource(strExePath, strResourceName, strResourceSection, arrayBuffer);
	}

	bool LoadResource(const std::string& strResourceName, const std::string& strResourceSection, std::vector<unsigned char>& arrayBuffer, HMODULE hModule = GetModuleHandle())
	{
		bool bResult = false;
		HRSRC hResInfo = ::FindResource(hModule, strResourceName.c_str(), strResourceSection.c_str());
		if (hResInfo)
		{
			DWORD dwSize = ::SizeofResource(hModule, hResInfo);
			HGLOBAL hRes = ::LoadResource(hModule, hResInfo);
			if (hRes && dwSize)
			{
				void* pData = ::LockResource(hRes);
				if (pData)
				{			
					arrayBuffer.resize(dwSize,0);
					memcpy(&arrayBuffer[0],pData,dwSize);
					bResult = true;
					UnlockResource(hRes);
				}			
			}
		}
		return bResult;
	}

	bool LoadResource(const std::string& strResourceName, const std::string& strResourceSection, std::string& strText, HMODULE hModule = GetModuleHandle())
	{
		bool bResult = false;
		std::vector<unsigned char> arrayBuffer;
		bResult = LoadResource(strResourceName, strResourceSection, arrayBuffer, hModule);
		if (bResult)
		{
			arrayBuffer[arrayBuffer.size()-1] = '\0';
			strText = (char*)&arrayBuffer[0];			
		}
		return bResult;
	}

	bool EncryptString(const std::string& strText, const std::string& strPassword, std::string& strEncryptedData)
	{
		RandomPool randPool;
		int nSeedlen=strlen(strPassword.c_str())+10;
		char *seed=new char[nSeedlen];
		memset(seed,0,nSeedlen);
		sprintf(seed,"%8.8x%s",GetTickCount(),strPassword.c_str());
		randPool.Put((byte *)seed, strlen(seed));
		AESPHM Aes;
		byte *pCipher=new byte[Aes.MaxCiphertextLen(strText.length())];
		DWORD dwCipherLen=Aes.Encrypt(randPool,strPassword,pCipher,strText);
		delete []seed;
		HexEncoder hex(new StringSink(strEncryptedData));
		hex.Put(pCipher,dwCipherLen);
		delete []pCipher;
		return true;
	}

	bool DecryptString(const std::string& strEncryptedData, const std::string& strPassword, std::string& strText)
	{
		
		AESPHM Aes;
		DWORD dwCipherTextLength=strlen(strEncryptedData.c_str())/2;
		byte *pCipher=new byte[dwCipherTextLength];
		HexDecoder hex(new ArraySink(pCipher,dwCipherTextLength));
		hex.Put((byte*)strEncryptedData.c_str(),strEncryptedData.length());
		ConstByteArrayParameter cbar((const byte*)pCipher,dwCipherTextLength);
		byte *pPlaintext=new byte[dwCipherTextLength];
		DecodingResult res=Aes.Decrypt(strPassword,pPlaintext,cbar);
		if(res.isValidCoding)
		{
			strText=(char *)pPlaintext;
			strText=strText.substr(0,res.messageLength); 
			return true;
		}
		else 
		{
			return false;
		}	
	}

	std::string Quote(const std::string& strText)
	{
		return std::string(_T("\"")) + strText + std::string(_T("\""));
	}

	bool LoadTextFromFile(const std::string& path, std::string& text, std::string& password)
	{
		std::string filepath = path;

		strlwr((char*)filepath.c_str());
		int nFileNameLength = _tcslen(filepath.c_str());
		if (nFileNameLength > 4)
		{
			if (!strcmp(&filepath[nFileNameLength-4], _T(".txt")))
			{
				FILE* pFile = fopen(path.c_str(), _T("rb"));
				fseek(pFile, 0, SEEK_END);
				DWORD dwFileSize = (DWORD)ftell(pFile);
				fseek(pFile, 0, SEEK_SET);
				if (dwFileSize)
				{
					text.resize(dwFileSize+1);
					fread((char*)text.c_str(), sizeof(char), dwFileSize, pFile);
					text[dwFileSize] = '\0';
				}
				fclose(pFile);
				return true;
			}
		}

		return false;
	}

	bool SaveTextToFile(const std::string& path, const std::string& text, std::string& password, HWND hWnd = 0, LPLOCKNOTEWINTRAITS wintraits = NULL)
	{
		std::string filepath = path;

		//strlwr((char*)filepath.c_str());
		int nFileNameLength = _tcslen(filepath.c_str());
		if (nFileNameLength > 4)
		{
			std::string extension = &filepath[nFileNameLength-4];
			strlwr((char*)extension.c_str());
			if (!strcmp(extension.c_str(), _T(".txt")))
			{
				FILE* pFile = _tfopen(filepath.c_str(), _T("wb"));
				bool bResult = (fwrite(text.c_str(), sizeof(char), _tcslen(text.c_str()), pFile) == _tcslen(text.c_str()));
				fclose(pFile);
				return bResult;
			}
		}			
		
		if (!_tcslen(password.c_str()))
		{
 			password = GetNewPasswordDlg();
			if (!_tcslen(password.c_str()))
			{
				return false;
			}
		}

		std::string data = _T("");
		if (_tcslen(text.c_str()))
		{
			Utils::EncryptString(text, password, data);
		}
		else
		{
			Utils::MessageBox(hWnd, STR(IDS_TEXT_IS_ENCRYPTED), MB_OK | MB_ICONINFORMATION);
		}

		TCHAR szModulePath[MAX_PATH] = {'\0'};

		::GetModuleFileName(Utils::GetModuleHandle(), szModulePath, MAX_PATH);
		::CopyFile(szModulePath, filepath.c_str(), FALSE);	
		bool bResult = Utils::UpdateResource(filepath.c_str(), _T("CONTENT"), _T("PAYLOAD"), data);

		if (wintraits)
		{
			// write window sizes to resource
			std::string sizeinfo;
			char szSizeInfo[MAX_PATH] = "";
			sprintf(szSizeInfo, "%d", wintraits->m_nWindowSizeX);
			sizeinfo = szSizeInfo;
			Utils::UpdateResource(filepath.c_str(), _T("SIZEX"), _T("INFORMATION"), sizeinfo);
			sprintf(szSizeInfo, "%d", wintraits->m_nWindowSizeY);
			sizeinfo = szSizeInfo;
			Utils::UpdateResource(filepath.c_str(), _T("SIZEY"), _T("INFORMATION"), sizeinfo);
		
			// write font size
			std::string fontsize;
			char szFontSize[MAX_PATH] = "";
			sprintf(szFontSize, "%d", wintraits->m_nFontSize);
			fontsize = szFontSize;
			Utils::UpdateResource(filepath.c_str(), _T("FONTSIZE"), _T("INFORMATION"), fontsize);

			// write font typeface
			Utils::UpdateResource(filepath.c_str(), _T("TYPEFACE"), _T("INFORMATION"), wintraits->m_strFontName);
		}

		return bResult;
	}
}

using namespace Utils;
