/*
	BmPrefsWin.cpp
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


#include <Autolock.h>
#include <Font.h>
#include <Message.h>
#include <String.h>

#include <BeBuild.h>
#ifdef B_BEOS_VERSION_DANO
	class BFont;
	class BMessage;
	class BPopUpMenu;
	class BRect;
#endif

#include <layout-all.h>

#include "ColumnListView.h"
#include "CLVEasyItem.h"
#include "UserResizeSplitView.h"

#include "BmApp.h"
#include "BmBasics.h"
#include "BmLogHandler.h"
#include "BmMsgTypes.h"
#include "BmPrefsView.h"
#include "BmPrefsGeneralView.h"
#include "BmPrefsMailConstrView.h"
#include "BmPrefsMailReadView.h"
#include "BmPrefsRecvMailView.h"
#include "BmPrefsSendMailView.h"
#include "BmPrefsShortcutsView.h"
#include "BmPrefsSignatureView.h"
#include "BmPrefsWin.h"
#include "BmUtil.h"



/********************************************************************************\
	BmPrefsWin
\********************************************************************************/

#define BM_SAVE_CHANGES 'bmSV'
#define BM_UNDO_CHANGES 'bmUD'

BmPrefsWin* BmPrefsWin::theInstance = NULL;

/*------------------------------------------------------------------------------*\
	CreateInstance()
		-	creates the app's main window
		-	initialiazes the window's dimensions by reading its archive-file (if any)
\*------------------------------------------------------------------------------*/
BmPrefsWin* BmPrefsWin::CreateInstance() 
{
	if (!theInstance) {
		theInstance = new BmPrefsWin;
		theInstance->ReadStateInfo();
	}
	return theInstance;
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
BmPrefsWin::BmPrefsWin()
	:	inherited( "PrefsWin", BRect(50,50,549,449),
					  "Beam Preferences",
					  B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 
					  B_ASYNCHRONOUS_CONTROLS | B_NOT_CLOSABLE)
	,	mPrefsListView( NULL)
	,	mPrefsViewContainer( NULL)
	,	mVertSplitter( NULL)
{
	MView* mOuterGroup = 
		new VGroup( 
			new MBorder( M_RAISED_BORDER, 5, NULL,
				new HGroup(
					minimax(500,200),
					mVertSplitter = new UserResizeSplitView(
						new VGroup(
							minimax(100,200,200,1E5),
							CreatePrefsListView( minimax(100,200,200,1E5), 120, 200),
							new Space( minimax(0,2,0,2)),
							0
						),
						mPrefsViewContainer = new BmPrefsViewContainer(
							new LayeredGroup( 
								new BmPrefsView( NULL),
								new BmPrefsGeneralView(),
								new BmPrefsShortcutsView(),
								new BmPrefsMailConstrView(),
								new BmPrefsSendMailView(),
								new BmPrefsSignatureView(),
								new BmPrefsMailReadView(),
								new BmPrefsRecvMailView(),
								0
							)
						),
						"vsplitter", 120, B_VERTICAL, true, true, false, B_FOLLOW_NONE
					),
					0
				)
			),
			new Space( minimax(0,10,0,10)),
			new HGroup(
				new Space(),
				new MButton( "Cancel", new BMessage(BM_UNDO_CHANGES), this, minimax(-1,-1,-1,-1)),
				new Space( minimax(20,0,20,0)),
				new MButton( "Save", new BMessage(BM_SAVE_CHANGES), this, minimax(-1,-1,-1,-1)),
				new Space( minimax(20,0,20,0)),
				0
			),
			new Space( minimax(0,10,0,10)),
			0
		);
	AddChild( dynamic_cast<BView*>(mOuterGroup));
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
BmPrefsWin::~BmPrefsWin() {
	theInstance = NULL;
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
status_t BmPrefsWin::ArchiveState( BMessage* archive) const {
	status_t ret = inherited::ArchiveState( archive)
						|| archive->AddFloat( MSG_VSPLITTER, mVertSplitter->DividerLeftOrTop());
	return ret;
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
status_t BmPrefsWin::UnarchiveState( BMessage* archive) {
	float vDividerPos;
	status_t ret = inherited::UnarchiveState( archive)
						|| archive->FindFloat( MSG_VSPLITTER, &vDividerPos);
	if (ret == B_OK) {
		mVertSplitter->SetPreferredDividerLeftOrTop( vDividerPos);
	}
	return ret;
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
CLVContainerView* BmPrefsWin::CreatePrefsListView( minimax minmax, int32 width, int32 height) {
	mPrefsListView = new ColumnListView( minmax, BRect( 0, 0, width-1, height-1), NULL, 
													 B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
													 B_SINGLE_SELECTION_LIST, true, true);

	BFont font(*be_bold_font);
	mPrefsListView->SetFont( &font);
	mPrefsListView->SetSelectionMessage( new BMessage( BM_SELECTION_CHANGED));
	mPrefsListView->SetTarget( this);
	mPrefsListView->ClickSetsFocus( true);
	CLVContainerView* container 
		= mPrefsListView->Initialize( BRect( 0,0,width-1,height-1), 
												B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
												B_FOLLOW_TOP_BOTTOM, false, true, true, B_FANCY_BORDER,
												be_bold_font);
	mPrefsListView->AddColumn( 
		new CLVColumn( NULL, 10.0, 
							CLV_EXPANDER | CLV_LOCK_AT_BEGINNING | CLV_NOT_MOVABLE, 10.0));
	mPrefsListView->AddColumn( 
		new CLVColumn( "   Category", 300.0, 
							CLV_NOT_MOVABLE|CLV_NOT_RESIZABLE|CLV_TELL_ITEMS_WIDTH, 300.0));

	CLVEasyItem* item;	
	item = new CLVEasyItem( 0, false, false, 18.0);
	item->SetColumnContent( 1, "General");
	mPrefsListView->AddItem( item);
	mPrefsListView->Expand( item);

	item = new CLVEasyItem( 1, false, false, 18.0);
	item->SetColumnContent( 1, "Shortcuts");
	mPrefsListView->AddItem( item);

	item = new CLVEasyItem( 0, true, false, 18.0);
	item->SetColumnContent( 1, "Sending Mail");
	mPrefsListView->AddItem( item);
	mPrefsListView->Expand( item);

	item = new CLVEasyItem( 1, false, false, 18.0);
	item->SetColumnContent( 1, "Accounts");
	mPrefsListView->AddItem( item);

	item = new CLVEasyItem( 1, false, false, 18.0);
	item->SetColumnContent( 1, "Signatures");
	mPrefsListView->AddItem( item);

	item = new CLVEasyItem( 0, true, false, 18.0);
	item->SetColumnContent( 1, "Receiving Mail");
	mPrefsListView->AddItem( item);
	mPrefsListView->Expand( item);

	item = new CLVEasyItem( 1, false, false, 18.0);
	item->SetColumnContent( 1, "Accounts");
	mPrefsListView->AddItem( item);

	return container;
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
void BmPrefsWin::MessageReceived( BMessage* msg) {
	try {
		switch( msg->what) {
			case BM_SELECTION_CHANGED: {
				int32 index = mPrefsListView->CurrentSelection( 0);
				int32 fullIndex = mPrefsListView->FullListIndexOf( 
					(CLVListItem*)mPrefsListView->ItemAt( index)
				);
				mPrefsViewContainer->ShowPrefs( 1+fullIndex);
				break;
			}
			case BM_SAVE_CHANGES: {
				mPrefsViewContainer->SaveData();
				PostMessage( B_QUIT_REQUESTED);
				break;
			}
			case BM_UNDO_CHANGES: {
				mPrefsViewContainer->Finish();
				PostMessage( B_QUIT_REQUESTED);
				break;
			}
			default:
				inherited::MessageReceived( msg);
		}
	}
	catch( exception &err) {
		// a problem occurred, we tell the user:
		BM_SHOWERR( BString("PrefsWin: ") << err.what());
	}
}

/*------------------------------------------------------------------------------*\
	QuitRequested()
		-	standard BeOS-behaviour, we allow a quit
\*------------------------------------------------------------------------------*/
bool BmPrefsWin::QuitRequested() {
	BM_LOG2( BM_LogPrefsWin, BString("PrefsWin has been asked to quit"));
	return true;
}

/*------------------------------------------------------------------------------*\
	Quit()
		-	standard BeOS-behaviour, we quit
\*------------------------------------------------------------------------------*/
void BmPrefsWin::Quit() {
	mPrefsViewContainer->Finish();
	mPrefsViewContainer->WriteStateInfo();
	BM_LOG2( BM_LogPrefsWin, BString("PrefsWin has quit"));
	inherited::Quit();
}

/*------------------------------------------------------------------------------*\
	PrefsListSelectionChanged()
		-	
\*------------------------------------------------------------------------------*/
void BmPrefsWin::PrefsListSelectionChanged( int32 numSelected) {
}
