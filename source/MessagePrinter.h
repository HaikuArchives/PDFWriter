/*

MessagePrinter.h

Copyright (c) 2001 OpenBeOS. 

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

#ifndef MSG_PRINTER_H
#define MSG_PRINTER_H 

#include <StorageKit.h>
#include <SupportKit.h>
#include <string.h>

const char FILE_NAME[] = "MessageFile";

/**
 * Class
 */
class MessagePrinter
{
private:
	BNode node;
	status_t _err;
	char msgFileName[B_FILE_NAME_LENGTH];
	
public:
	MessagePrinter() { strcpy(msgFileName, FILE_NAME);	}
	MessagePrinter(const char *fileName) { strcpy(msgFileName, fileName); }
	~MessagePrinter() { }

	status_t Print(BMessage *msg);
};

#endif
