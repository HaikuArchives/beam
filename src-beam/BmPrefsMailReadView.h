/*
	BmPrefsMailReadView.h
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


#ifndef _BmPrefsMailReadView_h
#define _BmPrefsMailReadView_h

#include "BmPrefsView.h"

#define BM_CHECK_IF_PPP_UP_CHANGED 	'bmCP'

class BmCheckControl;
class BmTextControl;
/*------------------------------------------------------------------------------*\
	BmPrefsMailReadView
		-	
\*------------------------------------------------------------------------------*/
class BmPrefsMailReadView : public BmPrefsView {
	typedef BmPrefsView inherited;

public:
	// c'tors and d'tor:
	BmPrefsMailReadView();
	virtual ~BmPrefsMailReadView();
	
	// overrides of BmPrefsView base:
	void Initialize();
	void SaveData();
	void UndoChanges();

	// overrides of BView base:
	void MessageReceived( BMessage* msg);

private:

	BmTextControl* mHeaderListSmallControl;
	BmTextControl* mHeaderListLargeControl;
	BmTextControl* mMimeTypeTrustInfoControl;
	BmTextControl* mMarkAsReadDelayControl;
	BmCheckControl* mAutoCheckIfPppUpControl;

	// Hide copy-constructor and assignment:
	BmPrefsMailReadView( const BmPrefsMailReadView&);
	BmPrefsMailReadView operator=( const BmPrefsMailReadView&);
};

#endif