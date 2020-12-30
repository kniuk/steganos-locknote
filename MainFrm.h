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

#include <list>
#include <iostream>
#include <algorithm>
#include <atldlgs.h>
#include "utils.h"

UINT UWM_FINDMSGSTRING = CFindReplaceDialog::GetFindReplaceMsg();

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	struct UndoBuffer
	{
		int m_nStartChar;
		int m_nEndChar;
		std::string m_strText;
	};

	std::list<UndoBuffer> m_listUndo;

	CLockNoteView m_view;
	CFont m_fontEdit;
	std::string m_text;
	UndoBuffer m_currentBuffer;
	std::string m_password;

	DWORD m_dwSearchFlags;
	std::string m_strSearchString;
	CFindReplaceDialog* m_pCurrentFindReplaceDialog;

	int m_nWindowSizeX;
	int m_nWindowSizeY;
	int m_nFontSize;
	std::string m_strFontName;
	bool m_bTraitsChanged;

	CMenu* m_pMenu;

	CMainFrame()
	{
		m_nWindowSizeX = 570;
		m_nWindowSizeY = 400;
		m_nFontSize = 10;
		m_strFontName = NAME_FONT_LUCIDA_CONSOLE;
		m_bTraitsChanged = false;
		m_pMenu = NULL;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		{
			return TRUE;
		}

		if (m_pCurrentFindReplaceDialog)
		{
			if (m_pCurrentFindReplaceDialog->IsDialogMessageA(pMsg))
			{
				return TRUE;
			}
		}

		return m_view.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
		MESSAGE_HANDLER(UWM_FINDMSGSTRING, OnFindMsgString)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_CHANGEPASSWORD, OnFileChangePassword)
		COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_EDIT_SELECT_ALL, OnEditSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnEditCut)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
		COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnEditUndo)
		COMMAND_ID_HANDLER(ID_EDIT_FIND, OnEditFind)
		COMMAND_ID_HANDLER(ID_EDIT_FINDNEXT, OnEditFindNext)
		COMMAND_ID_HANDLER(ID_VIEW_FONTSIZE_9, OnViewFontSize)
		COMMAND_ID_HANDLER(ID_VIEW_FONTSIZE_10, OnViewFontSize)
		COMMAND_ID_HANDLER(ID_VIEW_FONTSIZE_12, OnViewFontSize)
		COMMAND_ID_HANDLER(ID_VIEW_FONTSIZE_14, OnViewFontSize)
		COMMAND_ID_HANDLER(ID_FONT_ARIAL, OnViewFontTypeFace)
		COMMAND_ID_HANDLER(ID_FONT_COURIER_NEW, OnViewFontTypeFace)
		COMMAND_ID_HANDLER(ID_FONT_LUCIDA_CONSOLE, OnViewFontTypeFace)
		COMMAND_ID_HANDLER(ID_FONT_TAHOMA, OnViewFontTypeFace)
		COMMAND_ID_HANDLER(ID_FONT_VERDANA, OnViewFontTypeFace)
		
		COMMAND_CODE_HANDLER(EN_CHANGE, OnChange)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	std::string GetText()
	{
		std::string text;
		int nLength = m_view.GetWindowTextLength();
		text.resize(nLength);
		::GetWindowText(m_view, (char*)text.c_str(), nLength+1);
		return text;
	}

	bool SaveTextToFile(const std::string& path, const std::string& text, std::string& password, HWND hWnd = 0)
	{
		LOCKNOTEWINTRAITS wintraits;
		wintraits.m_nFontSize = m_nFontSize;
		wintraits.m_nWindowSizeX = m_nWindowSizeX;
		wintraits.m_nWindowSizeY = m_nWindowSizeY;
		wintraits.m_strFontName = m_strFontName;
		return Utils::SaveTextToFile(path, text, password, *this, &wintraits);
	}

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	void CheckFontSize()
	{
		// check the correct menu item
		HMENU hMenu = this->GetMenu();
		CMenuHandle menu(hMenu);
		menu.CheckMenuItem(ID_VIEW_FONTSIZE_9, MF_UNCHECKED);
		menu.CheckMenuItem(ID_VIEW_FONTSIZE_10, MF_UNCHECKED);
		menu.CheckMenuItem(ID_VIEW_FONTSIZE_12, MF_UNCHECKED);
		menu.CheckMenuItem(ID_VIEW_FONTSIZE_14, MF_UNCHECKED);
		switch (m_nFontSize)
		{
		case 9:
			menu.CheckMenuItem(ID_VIEW_FONTSIZE_9, MF_CHECKED);
			break;
		case 12:
			menu.CheckMenuItem(ID_VIEW_FONTSIZE_12, MF_CHECKED);
			break;
		case 14:
			menu.CheckMenuItem(ID_VIEW_FONTSIZE_14, MF_CHECKED);
			break;
		case 10:
		default:
			menu.CheckMenuItem(ID_VIEW_FONTSIZE_10, MF_CHECKED);
		};
	}

	LRESULT OnViewFontSize(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// get selected font size and check the correct menuitem
		int nOldFontSize = m_nFontSize;

		switch (wID)
		{
		case ID_VIEW_FONTSIZE_9:
			m_nFontSize = 9;
			break;
		case ID_VIEW_FONTSIZE_12:
			m_nFontSize = 12;
			break;
		case ID_VIEW_FONTSIZE_14:
			m_nFontSize = 14;
			break;
		case ID_VIEW_FONTSIZE_10:
		default:
			m_nFontSize = 10;
		}
		CheckFontSize();

		CClientDC dc(*this);
		m_fontEdit.DeleteObject();
		m_fontEdit.CreateFont(
			-MulDiv(m_nFontSize, GetDeviceCaps(dc, LOGPIXELSY), 72),
			0,
			0,
			0,
			FW_NORMAL,
			FALSE,
			FALSE,
			FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH,
			m_strFontName.c_str());

		ATLASSERT(m_fontEdit);
		m_view.SetFont(m_fontEdit);
		
		if (m_nFontSize != nOldFontSize)
		{
			m_bTraitsChanged = true;
		}
		return 0;
	}

	void CheckFontTypeFace()
	{
		// check the correct menu item
		HMENU hMenu = this->GetMenu();
		CMenuHandle menu(hMenu);
		menu.CheckMenuItem(ID_FONT_ARIAL, MF_UNCHECKED);
		menu.CheckMenuItem(ID_FONT_COURIER_NEW, MF_UNCHECKED);
		menu.CheckMenuItem(ID_FONT_LUCIDA_CONSOLE, MF_UNCHECKED);
		menu.CheckMenuItem(ID_FONT_TAHOMA, MF_UNCHECKED);
		menu.CheckMenuItem(ID_FONT_VERDANA, MF_UNCHECKED);
		if (m_strFontName == "Arial")
		{
			menu.CheckMenuItem(ID_FONT_ARIAL, MF_CHECKED);
		}
		else if (m_strFontName == "Courier New")
		{
			menu.CheckMenuItem(ID_FONT_COURIER_NEW, MF_CHECKED);
		}
		else if (m_strFontName == "Tahoma")
		{
			menu.CheckMenuItem(ID_FONT_TAHOMA, MF_CHECKED);
		}
		else if (m_strFontName == "Verdana")
		{
			menu.CheckMenuItem(ID_FONT_VERDANA, MF_CHECKED);
		}
		else if (m_strFontName == "Lucida Console")
		{
			menu.CheckMenuItem(ID_FONT_LUCIDA_CONSOLE, MF_CHECKED);
		}
		else
		{
			menu.CheckMenuItem(ID_FONT_LUCIDA_CONSOLE, MF_CHECKED);
		}
	}

	LRESULT OnViewFontTypeFace(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// get selected font name and check the correct menu item
		std::string strOldFontName = m_strFontName;
		switch (wID)
		{
		case ID_FONT_ARIAL:
			m_strFontName = NAME_FONT_ARIAL;
			break;
		case ID_FONT_COURIER_NEW:
			m_strFontName = NAME_FONT_COURIER_NEW;
			break;
		case ID_FONT_TAHOMA:
			m_strFontName = NAME_FONT_TAHOMA;
			break;
		case ID_FONT_VERDANA:
			m_strFontName = NAME_FONT_VERDANA;
			break;
		case ID_FONT_LUCIDA_CONSOLE:
		default:
			wID = ID_FONT_LUCIDA_CONSOLE;
			m_strFontName = NAME_FONT_LUCIDA_CONSOLE;
		}
		CheckFontTypeFace();

		CClientDC dc(*this);
		m_fontEdit.DeleteObject();
		m_fontEdit.CreateFont(
			-MulDiv(m_nFontSize, GetDeviceCaps(dc, LOGPIXELSY), 72),
			0,
			0,
			0,
			FW_NORMAL,
			FALSE,
			FALSE,
			FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH,
			m_strFontName.c_str());

		ATLASSERT(m_fontEdit);
		m_view.SetFont(m_fontEdit);
		
		if (strOldFontName != m_strFontName)
		{
			m_bTraitsChanged = true;
		}
		return 0;
	}

	LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		HDROP hDrop = (HDROP)wParam;
		UINT uFileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		int nResult = Utils::MessageBox(*this, STR(IDS_ASK_CONVERT_FILES), MB_YESNO | MB_ICONQUESTION);
		if (nResult == IDYES)
		{
			int nConverted = 0;
			std::string encryptPassword;
			std::list<std::string> decryptPasswords;
			for (UINT uIndex = 0; uIndex < uFileCount; uIndex++)
			{
				std::string filename;
				UINT uLength = DragQueryFile(hDrop, uIndex, NULL, 0);
				filename.resize(uLength);
				DragQueryFile(hDrop, uIndex, (char*)filename.c_str(), uLength+1);
				std::string extension = &filename[uLength-4];
				std::string newfilename = filename;
				strlwr((char*)extension.c_str());
				if (extension == _T(".txt"))
				{
					_tcscpy(&newfilename[uLength-4], _T(".exe"));
					std::string text;
					std::string password;
					if (LoadTextFromFile(filename, text, password))
					{
						if (SaveTextToFile(newfilename, text, encryptPassword))
						{
							nConverted += 1;
						}
					}
				}
				/*
				// convert SLIM back to text
				else if (extension == _T(".exe"))
				{
					_tcscpy(&newfilename[uLength-4], _T(".txt"));
				}
				*/
				else
				{					
					std::string text;
					text.resize(32768);
					sprintf((char*)text.c_str(), STR(IDS_CANT_CONVERT).c_str(), filename.c_str());
					Utils::MessageBox(*this, text, MB_OK | MB_ICONERROR);
				}
			}
			if (nConverted)
			{
				std::string text;
				text.resize(32768);
				sprintf((char*)text.c_str(), STR(IDS_CONVERT_DONE).c_str(), nConverted);
				Utils::MessageBox(*this, text, MB_OK | MB_ICONINFORMATION);
			}
		}
		DragFinish(hDrop);
		return 0;
	}

	LRESULT OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		std::string encryptPassword;

		CFileDialog dlg(FALSE, _T("exe"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, *this);
		if (dlg.DoModal() == IDOK)
		{
			std::string text;
			text = GetText();
			if (SaveTextToFile(dlg.m_szFileName, text, encryptPassword))
			{
				//m_text = text;
			}
		}

		return 0;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		std::string text = GetText();

		if (_tcscmp(m_text.c_str(),text.c_str()) || m_bTraitsChanged)
		{
			int nResult = Utils::MessageBox(*this, STR(IDS_SAVE_CHANGES), MB_YESNOCANCEL | MB_ICONQUESTION);
			if (nResult == IDCANCEL)
			{
				return FALSE;
			}

			bHandled = FALSE;

			if (nResult == IDNO)
			{
				m_bTraitsChanged = false;
				return FALSE;
			}

			if (!_tcslen(text.c_str()))
			{
				m_text = text;
				return FALSE;
			}

			if (!_tcslen(m_password.c_str()))
			{
				m_password = GetNewPasswordDlg();
				if (!_tcslen(m_password.c_str()))
				{
					bHandled = TRUE;
					return FALSE;
				}
			}
		}

		bHandled = FALSE;
		m_text = text;

		return FALSE;
	}
	
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int m_nOldWindowSizeX = m_nWindowSizeX;
		int m_nOldWindowSizeY = m_nWindowSizeY;
		RECT rc;
		GetWindowRect(&rc);
		m_nWindowSizeX = rc.right - rc.left;
		m_nWindowSizeY = rc.bottom - rc.top;
		bHandled = FALSE;
		if (m_nWindowSizeX != m_nOldWindowSizeX || m_nWindowSizeY != m_nOldWindowSizeY)
		{
			m_bTraitsChanged = true;
		}
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_dwSearchFlags = FR_DOWN;
		m_pCurrentFindReplaceDialog = NULL;

		DragAcceptFiles(TRUE);

		CreateSimpleStatusBar();

		m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, WS_EX_CLIENTEDGE);
		m_view.SetLimitText((1<<31)); // allow a lot of text to be entered

		CClientDC dc(*this);
		m_fontEdit.CreateFont(
			-MulDiv(m_nFontSize, GetDeviceCaps(dc, LOGPIXELSY), 72),
			0,
			0,
			0,
			FW_NORMAL,
			FALSE,
			FALSE,
			FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH,
			m_strFontName.c_str());

		ATLASSERT(m_fontEdit);
		m_view.SetFont(m_fontEdit);

		if (m_text.length())
		{
			m_view.SetWindowText(m_text.c_str());
			//m_view.SetSelAll();
		}

		std::string modulepath;
		modulepath.resize(MAX_PATH+1);
		std::string title;
		::GetModuleFileName(Utils::GetModuleHandle(), (char*)modulepath.c_str(), MAX_PATH);
		int nSlashPos = modulepath.rfind("\\");
		if (nSlashPos != -1)
		{
			title = &modulepath[nSlashPos+1];
			int nDotPos = title.rfind(".");
			if (nDotPos != -1)
			{
				title[nDotPos] = '\0';
			}
		}

		std::string windowtitle;
		windowtitle.resize(MAX_PATH + 100);
		sprintf((char*)windowtitle.c_str(), "%s - %s", title.c_str(), STR(IDR_MAINFRAME).c_str());

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		m_currentBuffer.m_strText = GetText();
		//m_view.GetSel(m_currentBuffer.m_nStartChar, m_currentBuffer.m_nEndChar);
		m_view.GetSel(m_currentBuffer.m_nStartChar, m_currentBuffer.m_nEndChar);

		SetWindowPos(NULL, 0, 0, m_nWindowSizeX, m_nWindowSizeY, SWP_NOZORDER|SWP_NOMOVE);
		CenterWindow();

		m_view.SetFocus();

		SetWindowText(windowtitle.c_str());

		return 0;
	}

	LRESULT OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_view.Copy();
		return 0;
	}

	LRESULT OnEditCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_view.Cut();
		return 0;
	}

	LRESULT OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (m_pCurrentFindReplaceDialog)
		{
			m_pCurrentFindReplaceDialog->SendMessageToDescendants(WM_PASTE, 0, 0);
		}
		else
		{
			m_view.Paste();
		}
		return 0;
	}

	LRESULT OnEditUndo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (!m_listUndo.empty())
		{
			UndoBuffer undo = m_listUndo.back();
			m_listUndo.pop_back();
			m_currentBuffer = undo;
			m_view.SetWindowText(m_currentBuffer.m_strText.c_str());
			m_view.SetSel(m_currentBuffer.m_nStartChar, m_currentBuffer.m_nEndChar);
		}
		return 0;
	}

	LRESULT OnEditFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (m_pCurrentFindReplaceDialog)
		{
			m_pCurrentFindReplaceDialog->ShowWindow(SW_SHOW);
			m_pCurrentFindReplaceDialog->SetFocus();
			return 0;
		}
		CFindReplaceDialog* pDlgFindReplace = new CFindReplaceDialog();
		pDlgFindReplace->Create(TRUE, m_strSearchString.c_str(), _T(""), m_dwSearchFlags, *this);
		pDlgFindReplace->ShowWindow(SW_SHOW);
		m_pCurrentFindReplaceDialog = pDlgFindReplace;
		return 0;
	}	

	void FindNext()
	{		
		int nBegin, nEnd;		
		std::string text;
		std::string searchtext;
		text = GetText();
		searchtext = m_strSearchString;
		if (!(m_dwSearchFlags & FR_MATCHCASE))
		{
			_strupr((char*)text.c_str());
			_strupr((char*)searchtext.c_str());
		}
		m_view.GetSel(nBegin, nEnd);
		

		int nIndex;
		if (!(m_dwSearchFlags & FR_DOWN))
		{
			nIndex = text.rfind(searchtext, nBegin-1);
			if (nIndex == -1)
			{
				nIndex = text.rfind(searchtext, -1);
			}
		}
		else
		{
			nIndex = text.find(searchtext, nEnd);
			if (nIndex == -1)
			{
				nIndex = text.find(searchtext, 0);
			}
		}
		if (nIndex != -1)
		{
			m_view.SetSel(nIndex,nIndex+strlen(searchtext.c_str()));
		}
		else
		{
			char szErrorMessage[MAX_PATH] = "";
			sprintf(szErrorMessage, STR(IDS_FIND_NOT_FOUND).c_str(), m_strSearchString.c_str());
			Utils::MessageBox(*this, szErrorMessage, MB_OK | MB_ICONWARNING);
			if (m_pCurrentFindReplaceDialog)
			{
				m_pCurrentFindReplaceDialog->SetFocus();
			}
		}
	}

	LRESULT OnEditFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (!m_pCurrentFindReplaceDialog)
		{
			BOOL bHandled = FALSE;
			OnEditFind(0, 0, NULL, bHandled);
		}
		else
		{
			FindNext();
		}
		return 0;
	}

	LRESULT OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		m_view.SetSelAll();
		return 0;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnFindMsgString(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CFindReplaceDialog* pDlgFindReplace = CFindReplaceDialog::GetNotifier(lParam);
		ATLASSERT(m_pCurrentFindReplaceDialog == pDlgFindReplace);

		m_dwSearchFlags = (pDlgFindReplace->m_fr.Flags) & (FR_WHOLEWORD|FR_MATCHCASE|FR_DOWN);
		m_strSearchString = pDlgFindReplace->GetFindString();

		if (pDlgFindReplace->IsTerminating())
		{
			m_pCurrentFindReplaceDialog = NULL;
		}
		else if (pDlgFindReplace->FindNext())
		{
			FindNext();
		}

		return 0;
	}

	LRESULT OnChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (m_listUndo.size() >= 1000)
		{
			m_listUndo.pop_front();
		}
		m_listUndo.push_back(m_currentBuffer);
		m_currentBuffer.m_strText = GetText();
		m_view.GetSel(m_currentBuffer.m_nStartChar, m_currentBuffer.m_nEndChar);
		return 0;
	}

	LRESULT OnFileChangePassword(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (_tcslen(m_password.c_str()))
		{
			std::string strOldPassword = GetPasswordDlg(*this);
			if (!_tcslen(strOldPassword.c_str()))
				return 0;			
			if (strOldPassword != m_password)
			{
				Utils::MessageBox(*this, STR(IDS_INVALID_PASSWORD), MB_OK | MB_ICONERROR);
				return 0;
			}
		}

		std::string strNewPassword = GetNewPasswordDlg(*this);
		if (!_tcslen(strNewPassword.c_str()))
			return 0;

		m_password = strNewPassword;
		Utils::MessageBox(*this, STR(IDS_PASSWORD_CHANGED), MB_OK | MB_ICONINFORMATION);
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}
};
