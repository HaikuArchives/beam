/*
	BmStorageUtil.h
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


#ifndef _BmStorageUtil_h
#define _BmStorageUtil_h

#include <set>

#include <String.h>

class entry_ref;

bool MoveToTrash( const entry_ref* refs, int32 count);

bool LivesInTrash( const entry_ref* eref);

bool CheckMimeType( const entry_ref* eref, const char* type);

bool FetchFile( BString fileName, BString& contents);

/*------------------------------------------------------------------------------*\
	BmTempFileList
		-	
\*------------------------------------------------------------------------------*/
class BmTempFileList {
	typedef set<BString> BmFileSet;
public:
	BmTempFileList() : mCount(0) 			{}
	~BmTempFileList();
	void AddFile( BString fileWithPath);
	void RemoveFile( BString fileWithPath);
	BString NextTempFilename()				{ return BString("bm_") << ++mCount; }
	BString NextTempFilenameWithPath();
private:
	BmFileSet mFiles;
	int32 mCount;
	// Hide copy-constructor and assignment:
	BmTempFileList( const BmTempFileList&);
	BmTempFileList operator=( const BmTempFileList&);
};

extern BmTempFileList TheTempFileList;

#endif