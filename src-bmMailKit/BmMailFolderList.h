/*
	BmMailFolderList.h
		$Id$
*/

#ifndef _BmMailFolderList_h
#define _BmMailFolderList_h

#include <sys/stat.h>

#include <Locker.h>

#include "BmDataModel.h"
#include "BmMailFolder.h"

/*------------------------------------------------------------------------------*\*\
	BmMailFolderList
		-	class 
\*------------------------------------------------------------------------------*/
class BmMailFolderList : public BArchivable, public BmListModel {
	typedef BArchivable inherited;
	typedef BmListModel inheritedModel;

	// archival-fieldnames:
	static const char* const MSG_MAILBOXMTIME = 	"bm:mboxmtime";
	static const char* const MSG_CURRFOLDER = 	"bm:currfolder";
	static const char* const MSG_TOPFOLDER = 		"bm:topfolder";

	static const char* const ARCHIVE_FILENAME = 	"Folder Cache";

public:
	// c'tors and d'tor
	BmMailFolderList();
	virtual ~BmMailFolderList();
	
	// archival stuff:
	virtual status_t Archive( BMessage* archive, bool deep = true) const;

	//	
	void StartJob();

	// getters:
	status_t InitCheck()						{ return mInitCheck; }

	//
	bool Store();

private:
	// the following members will be archived as part of BmFolderList:
	BmMailFolder* mTopFolder;
	BmMailFolder* mCurrFolder;

	// the following members will NOT be archived at all:
	status_t mInitCheck;

	//
	void InitializeMailFolders();
	int doInitializeMailFolders( BmMailFolder* folder, int level);
	//
	void InstantiateMailFolders( BMessage* archive);
	void doInstantiateMailFolders( BmMailFolder* folder, BMessage* archive, int level);
	//
	void QueryForNewMails();
};


#endif
