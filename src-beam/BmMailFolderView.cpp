/*
	BmMailFolderView.cpp
		$Id$
*/

#include <String.h>

#include "BmApp.h"
#include "BmLogHandler.h"
#include "BmMailFolder.h"
#include "BmMailFolderList.h"
#include "BmMailFolderView.h"
#include "BmMailRefView.h"
#include "BmUtil.h"

const int16 BmMailFolderItem::nFirstTextCol = 2;

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
BmMailFolderItem::BmMailFolderItem( BString key, BmListModelItem* _item, uint32 level, 
												bool superitem, bool expanded)
	:	inherited( key, _item, true, level, superitem, expanded)
{
	BBitmap* icon;
	BmMailFolder* folder = dynamic_cast<BmMailFolder*>( _item);
	BString displayName = folder->Name();
	if (folder->HasNewMail()) {
		int32 count = folder->NewMailCount();
		if (folder->NewMailCountForSubfolders() && !expanded)
 			count += folder->NewMailCountForSubfolders();
 		if (count)
			displayName << " - (" << count << ")";
	}
	if (folder->NewMailCount())
		icon = bmApp->IconMap["Folder_WithNew"];
	else
		icon = bmApp->IconMap["Folder"];
	SetColumnContent( 1, icon, 2.0, false);
	BmListColumn cols[] = {
		{ displayName.String(), false },
		{ NULL, false }
	};
	SetTextCols( nFirstTextCol, cols);
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
BmMailFolderItem::~BmMailFolderItem() { 
}



/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
bool BmMailFolderItem::NeedsHighlight() { 
	BmMailFolder* folder = dynamic_cast<BmMailFolder*>( ModelItem());
	return folder->HasNewMail();
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
BmMailFolderView* BmMailFolderView::CreateInstance(  minimax minmax, int32 width, int32 height) {
	BmMailFolderView* mailFolderView = new BmMailFolderView( minmax, width, height);
	return mailFolderView;
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
BmMailFolderView::BmMailFolderView( minimax minmax, int32 width, int32 height)
	:	inherited( minmax, BRect(0,0,width,height), "Beam_FolderView", B_SINGLE_SELECTION_LIST, 
					  true, true)
{
	Initialize( BRect( 0,0,width,height), B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE,
					B_FOLLOW_TOP_BOTTOM, false, true, false, B_FANCY_BORDER);
	AddColumn( new CLVColumn( NULL, 10.0, 
									  CLV_EXPANDER | CLV_LOCK_AT_BEGINNING | CLV_NOT_MOVABLE, 10.0));
	AddColumn( new CLVColumn( NULL, 18.0, CLV_LOCK_AT_BEGINNING | CLV_NOT_MOVABLE 
									  | CLV_NOT_RESIZABLE | CLV_PUSH_PASS | CLV_MERGE_WITH_RIGHT, 18.0));
	AddColumn( new CLVColumn( "Folders", 300.0, CLV_SORT_KEYABLE | CLV_NOT_RESIZABLE | CLV_NOT_MOVABLE, 300.0));
	SetSortFunction( CLVEasyItem::CompareItems);
	SetSortKey( 2);
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
BmMailFolderView::~BmMailFolderView() {
}

/*------------------------------------------------------------------------------*\
	()
		-	
\*------------------------------------------------------------------------------*/
BmListViewItem* BmMailFolderView::CreateListViewItem( BmListModelItem* item,
																		uint32 level) {
	return new BmMailFolderItem( item->Key(), item, level, false, false);
}

/*------------------------------------------------------------------------------*\
	MessageReceived( msg)
		-	
\*------------------------------------------------------------------------------*/
void BmMailFolderView::MessageReceived( BMessage* msg) {
	try {
		switch( msg->what) {
			default:
				inherited::MessageReceived( msg);
		}
	}
	catch( exception &err) {
		// a problem occurred, we tell the user:
		BM_SHOWERR( BString("MailFolderView: ") << err.what());
	}
}

/*------------------------------------------------------------------------------*\
	SelectionChanged()
		-	
\*------------------------------------------------------------------------------*/
void BmMailFolderView::SelectionChanged( void) {
	uint32 selection = CurrentSelection();
	if (selection >= 0) {
		BmMailFolderItem* folderItem;
		folderItem = dynamic_cast<BmMailFolderItem*>(ItemAt( selection));
		if (folderItem) {
			BmMailFolder* folder = dynamic_cast<BmMailFolder*>(folderItem->ModelItem());
			if (folder)
				bmApp->MailRefView->ShowFolder( folder);
		}
	}
}
