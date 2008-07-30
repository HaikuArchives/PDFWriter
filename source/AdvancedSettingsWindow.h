/*

PDF Writer printer driver.

Copyright (c) 2002 OpenBeOS.

Authors:
	Philippe Houdoin
	Simon Gauvin
	Michael Pfeiffer

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef ADVANCED_SETTINGS_WINDOW_H
#define ADVANCED_SETTINGS_WINDOW_H


#include "BlockingWindow.h"
#include "PrinterDriver.h"


#include <Directory.h>
#include <String.h>


class BMessage;
class BPopUpMenu;
class BTextControl;


class AdvancedSettingsWindow : public HWindow
{
	typedef HWindow	inherited;
public:
	// Constructors, destructors, operators...

					AdvancedSettingsWindow(BMessage *doc_info);

	virtual void	MessageReceived(BMessage *msg);

	enum {
					OK_MSG						= 'ok__',
					CANCEL_MSG					= 'cncl',
					CREATE_LINKS_MSG			= 'clnk',
					LINK_BORDER_MSG				= 'lnkb',
					CREATE_BOOKMARKS_MSG		= 'cbmk',
					DEFINITION_MSG				= 'defi',
					CREATE_XREFS_MSG			= 'cxrf',
					XREFS_MSG					= 'xref',
					AUTO_CLOSE_MSG				= 'acls',
					OPEN_SETTINGS_FOLDER_MSG	= 'opsf',
	};

private:
	void			_UpdateSettings();
	BDirectory		_SetupDirectory(const char* dirName);
	void			_AddMenuItem(BPopUpMenu* menu, const char* label, CloseOption option);

private:
	BMessage*		fSettings;
	BTextControl*	fLicenseKey;
	bool			fCreateLinks;
	float			fLinkBorderWidth;
	bool			fCreateBookmarks;
	BString			fBookmarkDefinition;
	bool			fCreateXRefs;
	BString			fXRefs;
	CloseOption		fCloseOption;
};

#endif
