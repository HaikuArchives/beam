/*
	BmLogWindow.cpp
		$Id$
*/
/*************************************************************************/
/*                                                                       */
/*  Beam - BEware Another Mailer                                         */
/*                                                                       */
/*  http://www.hirschkaefer.de/beam                                      */
/*                                                                       */
/*  Copyright (C) 2002 Oliver Tappe <beam@hirschkaefer.de>               */
/*                                                                       */
/*  This program is free software; you can redistribute it and/or        */
/*  modify it under the terms of the GNU General Public License          */
/*  as published by the Free Software Foundation; either version 2       */
/*  of the License, or (at your option) any later version.               */
/*                                                                       */
/*  This program is distributed in the hope that it will be useful,      */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    */
/*  General Public License for more details.                             */
/*                                                                       */
/*  You should have received a copy of the GNU General Public            */
/*  License along with this program; if not, write to the                */
/*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,         */
/*  Boston, MA  02111-1307, USA.                                         */
/*                                                                       */
/*************************************************************************/


#include <Message.h>

#include <MScrollView.h>
#include <MTextView.h>

#include "BmLogWindow.h"
#include "BmPrefs.h"

int32 BmLogWindow::nWinCount = 0;

/*------------------------------------------------------------------------------*\
	CreateAndStartInstanceInWindow( )
		-	
\*------------------------------------------------------------------------------*/
BmLogWindow* BmLogWindow::CreateAndStartInstanceFor( const char* logfileName) {
	float x = 50+(nWinCount*20)%300;
	float y = 50+(nWinCount*20)%300;
	BRect frame( x, y, x+599, y+399);
	BmString title("Beam-Log: ");
	title << logfileName;
	BmLogWindow* win = new BmLogWindow( frame, title, logfileName);
	win->Show();
	TheLogHandler->StartWatchingLogfile( win, logfileName);
	nWinCount++;
	return win;
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
BmLogWindow::BmLogWindow( const BRect& frame, const BmString& title, 
								  const char* logfileName)
	:	inherited( frame, title.String(), 
					  B_TITLED_WINDOW_LOOK, 
					  B_NORMAL_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
	,	mLogfileName( logfileName)
{
	mLogView = new MTextView();
	mLogView->MakeEditable( false);
	AddChild( dynamic_cast< BView*>( 
		new MScrollView( mLogView, false, true)
	));
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
BmLogWindow::~BmLogWindow() {
	TheLogHandler->StopWatchingLogfile( this, mLogfileName.String());
}

/*------------------------------------------------------------------------------*\
	MessageReceived()
		-	double-dispatch standard editing messages to focus-view:
\*------------------------------------------------------------------------------*/
void BmLogWindow::MessageReceived( BMessage* msg) {
	switch( msg->what) {
		case B_COPY:
		case B_CUT: 
		case B_PASTE: 
		case B_SELECT_ALL: {
			bool seenThis;
			if (msg->FindBool( "seenThis", &seenThis) != B_OK) {
				msg->AddBool( "seenThis", true);
				BView* focusView = CurrentFocus();
				if (focusView)
					PostMessage( msg, focusView);
			}
			break;
		}
		case BM_LOG_MSG: {
			BmString logMsg = msg->FindString( BmLogHandler::MSG_MESSAGE);
			int32 len = mLogView->TextLength();
			mLogView->Select( len, len);
			mLogView->Insert( logMsg.String());
			mLogView->ScrollToOffset( len + logMsg.Length());
			break;
		}
		default: {
			inherited::MessageReceived( msg);
		}
	}
}