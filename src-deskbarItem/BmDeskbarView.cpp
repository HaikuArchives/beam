/*
	BmDeskbarView.cpp
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

/*************************************************************************/
/*                                                                       */
/* Parts of this source-file have been derived (ripped?) from Scooby!    */
/*                                                                       */
/*************************************************************************/

#include <stdio.h>

#include <Autolock.h>
#include <Bitmap.h>
#include <Deskbar.h>
#include <FindDirectory.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <NodeInfo.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <Roster.h>
#include <String.h>
#include <Volume.h>

#include "BmDeskbarView.h"
#include "BmMsgTypes.h"

/***********************************************************
 * This is the exported function that will be used by Deskbar
 * to create and add the replicant
 ***********************************************************/
extern "C" _EXPORT BView* instantiate_deskbar_item();

const char* BM_DESKBAR_APP_SIG = "application/x-vnd.zooey-beam_deskbar";
const char* BM_APP_SIG = "application/x-vnd.zooey-beam";

const char* const BM_DeskbarItemName = "Beam_DeskbarItem";
const char* const BM_DeskbarNormal = "DeskbarIconNormal";
const char* const BM_DeskbarNew = "DeskbarIconNew";

/***********************************************************
 * Deskbar item installing function
 ***********************************************************/
BView* instantiate_deskbar_item(void) {
	return new BmDeskbarView(BRect(0, 0, 15, 15));
}

/*------------------------------------------------------------------------------*\
	LivesInTrash( nref)
		-	return whether or not the given node_ref lives inside the trash
\*------------------------------------------------------------------------------*/
bool LivesInTrash( const node_ref* nref) {
	BDirectory dir( nref);
	if (dir.InitCheck() != B_OK)
		return false;
	BEntry entry;
	if (dir.GetEntry( &entry) != B_OK)
		return false;
	BPath path;
	if (entry.GetPath( &path) != B_OK)
		return false;
	BString pathStr = path.Path();
	pathStr << "/";
	return pathStr.ICompare( "/boot/home/Desktop/Trash/", 25) == 0;
} 

/***********************************************************
 * Constructor.
 ***********************************************************/
BmDeskbarView::BmDeskbarView(BRect frame)
	:	BView( frame, BM_DeskbarItemName, B_FOLLOW_NONE, 
				 B_WILL_DRAW|B_PULSE_NEEDED)
	,	mCurrIcon( NULL)
	,	mNewMailCount( 0)
{
}

/***********************************************************
 * Constructor for achiving.
 ***********************************************************/
BmDeskbarView::BmDeskbarView(BMessage *message)
	:	BView( message)
	,	mCurrIcon( NULL)
	,	mNewMailCount( 0)
{
}

/***********************************************************
 * Destructor
 ***********************************************************/
BmDeskbarView::~BmDeskbarView() {
	delete mCurrIcon;
}

/***********************************************************
 * Instantiate
 ***********************************************************/
BmDeskbarView* BmDeskbarView::Instantiate(BMessage *data) {
	if (!validate_instantiation(data, "BmDeskbarView"))
		return NULL;
	return new BmDeskbarView(data);
}

/***********************************************************
 * Archive
 ***********************************************************/
status_t BmDeskbarView::Archive( BMessage *data,
										   bool deep) const {
	BView::Archive(data, deep);
	return data->AddString( "add_on", BM_DESKBAR_APP_SIG);
}

void BmDeskbarView::AttachedToWindow() {
	InstallDeskbarMonitor();
}


/***********************************************************
 * Draw
 ***********************************************************/
void BmDeskbarView::Draw(BRect /*updateRect*/) {	
	rgb_color oldColor = HighColor();
	SetHighColor(Parent()->ViewColor());
	FillRect(BRect(0.0,0.0,15.0,15.0));
	SetHighColor(oldColor);
	SetDrawingMode(B_OP_OVER);
	if(mCurrIcon)
		DrawBitmap(mCurrIcon,BRect(0.0,0.0,15.0,15.0));
	SetDrawingMode(B_OP_COPY);
	Sync();
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void BmDeskbarView::MessageReceived(BMessage *msg) {
	switch( msg->what) {
		case BMM_RESET_ICON: {
			ChangeIcon( BM_DeskbarNormal); 
			break;
		}
		case B_QUERY_UPDATE: {
			HandleQueryUpdateMsg( msg);
			break;
		}
		default: {
			BView::MessageReceived( msg);
		}
	}
}

/***********************************************************
 * IncNewMailCount
 ***********************************************************/
void BmDeskbarView::IncNewMailCount()	{ 
	if (mNewMailCount++ == 0 || mCurrIconName != BM_DeskbarNew)
		ChangeIcon( BM_DeskbarNew); 
}

/***********************************************************
 * DecNewMailCount()
 ***********************************************************/
void BmDeskbarView::DecNewMailCount() {
	if (--mNewMailCount < 0)
		mNewMailCount = 0;
	if (!mNewMailCount) 
		ChangeIcon( BM_DeskbarNormal); 
}

/***********************************************************
 * ChangeIcon
 ***********************************************************/
void BmDeskbarView::ChangeIcon( const char* iconName) {
	if (!iconName || mCurrIconName == iconName)
		return;
		
	BBitmap* newIcon = NULL;
	team_id beamTeam = be_roster->TeamFor( BM_APP_SIG);
	app_info appInfo;
	if (be_roster->GetRunningAppInfo( beamTeam, &appInfo) == B_OK) {
		appInfo.ref.set_name( BM_DeskbarItemName);
		// Load icon from the deskbaritem's resources
		BFile file( &appInfo.ref, B_READ_ONLY);
		if (file.InitCheck() == B_OK) {
			BResources rsrc( &file);
			size_t len;
			const void *data = rsrc.LoadResource( 'BBMP', iconName, &len);
			if (len) {
				BMemoryIO stream( data, len);
				stream.Seek( 0, SEEK_SET);
				BMessage archive;
				if (archive.Unflatten( &stream) == B_OK)
					newIcon = new BBitmap( &archive);
			}
		}
	}
	delete mCurrIcon;
	mCurrIcon = newIcon;
	mCurrIconName = newIcon ? iconName : "";
	LockLooper();
	Invalidate();
	UnlockLooper();
}

/*------------------------------------------------------------------------------*\
	QueryForNewMails()
		-	
\*------------------------------------------------------------------------------*/
void BmDeskbarView::InstallDeskbarMonitor() {
	int32 count;
	dirent* dent;
	node_ref nref;
	char buf[4096];

	BPath path;
	entry_ref eref;
	
	BMessenger thisAsTarget( this);

	// determine the volume of our home-directory:
	if (find_directory( B_USER_DIRECTORY, &path) == B_OK
	&& get_ref_for_path( path.Path(), &eref) == B_OK) {
		BVolume mboxVolume = eref.device;
		if (mNewMailQuery.SetVolume( &mboxVolume) == B_OK
		&& mNewMailQuery.SetPredicate( "MAIL:status == 'New'") == B_OK
		&& mNewMailQuery.SetTarget( thisAsTarget) == B_OK
		&& mNewMailQuery.Fetch() == B_OK) {
			while ((count = mNewMailQuery.GetNextDirents((dirent* )buf, 4096)) > 0) {
				dent = (dirent* )buf;
				while (count-- > 0) {
					nref.device = dent->d_pdev;
					nref.node = dent->d_pino;
					if (!LivesInTrash( &nref))
						mNewMailCount++;
					// Bump the dirent-pointer by length of the dirent just handled:
					dent = (dirent* )((char* )dent + dent->d_reclen);
				}
			}
		}
	}
	ChangeIcon( mNewMailCount ? BM_DeskbarNew : BM_DeskbarNormal);
}

/***********************************************************
 * SendToBeam
 ***********************************************************/
void BmDeskbarView::SendToBeam( BMessage* msg) {
	BMessenger beam( BM_APP_SIG);
	if (beam.IsValid()) {
		if (beam.SendMessage( msg, (BHandler*)NULL, 1000000) == B_OK)
			return;
	}
	// Beam has probably crashed, we quit, too:
	BDeskbar().RemoveItem( BM_DeskbarItemName);
}

/***********************************************************
 * Pulse
 ***********************************************************/
void BmDeskbarView::Pulse() {
	BMessenger beam( BM_APP_SIG);
	if (!beam.IsValid()) {
		// Beam has probably crashed, we quit, too:
		BDeskbar().RemoveItem( BM_DeskbarItemName);
	}
}

/***********************************************************
 * MouseDown
 ***********************************************************/
void BmDeskbarView::MouseDown(BPoint pos) {
	inherited::MouseDown(pos);
	int32 buttons;
	BMessage* msg = Looper()->CurrentMessage();
	if (msg && msg->FindInt32( "buttons", &buttons)==B_OK) {
		if (buttons == B_PRIMARY_MOUSE_BUTTON) {
			team_id beamTeam = be_roster->TeamFor( BM_APP_SIG);
			if (beamTeam)
				be_roster->ActivateApp( beamTeam);
		} else if (buttons == B_SECONDARY_MOUSE_BUTTON)
			ShowMenu( pos);
	}
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDeskbarView::ShowMenu( BPoint point) {
	BMenuItem* item;
	BPopUpMenu* theMenu = new BPopUpMenu( BM_DeskbarItemName, false, false);

	item = new BMenuItem( "Check Mail", new BMessage( BMM_CHECK_MAIL));
	item->SetTarget( BMessenger( BM_APP_SIG));
	theMenu->AddItem( item);

	theMenu->AddSeparatorItem();
	item = new BMenuItem( "New Message...", new BMessage( BMM_NEW_MAIL));
	item->SetTarget( BMessenger( BM_APP_SIG));
	theMenu->AddItem( item);

	theMenu->AddSeparatorItem();
	item = new BMenuItem( "Reset Icon", new BMessage( BMM_RESET_ICON));
	item->SetTarget( this);
	theMenu->AddItem( item);

	theMenu->AddSeparatorItem();
	item = new BMenuItem( "Quit Beam", new BMessage( B_QUIT_REQUESTED));
	item->SetTarget( BMessenger( BM_APP_SIG));
	theMenu->AddItem( item);

   ConvertToScreen(&point);
	BRect openRect;
	openRect.top = point.y - 5;
	openRect.bottom = point.y + 5;
	openRect.left = point.x - 5;
	openRect.right = point.x + 5;
  	theMenu->Go( point, true, false, openRect);
  	delete theMenu;
}

/*------------------------------------------------------------------------------*\
	HandleQueryUpdateMsg()
		-	
\*------------------------------------------------------------------------------*/
void BmDeskbarView::HandleQueryUpdateMsg( BMessage* msg) {
	int32 opcode = msg->FindInt32( "opcode");
	node_ref nref;
	switch( opcode) {
		case B_ENTRY_CREATED: {
			if (msg->FindInt64( "directory", &nref.node) == B_OK
			&& msg->FindInt32( "device", &nref.device) == B_OK
			&& !LivesInTrash( &nref)) {
				IncNewMailCount();
			}
			break;
		}
		case B_ENTRY_REMOVED: {
			if (msg->FindInt64( "directory", &nref.node) == B_OK
			&& msg->FindInt32( "device", &nref.device) == B_OK
			&& !LivesInTrash( &nref)) {
				DecNewMailCount();
			}
			break;
		}
	}
}