/*
	BmAutoCompleterVariations.cpp
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


#include <AppDefs.h>
#include <ListView.h>
#include <Looper.h>
#include <Message.h>
#include <Screen.h>
#include <TextControl.h>
#include <Window.h>

#include "BmBasics.h"
#include "Colors.h"

#include "BmAutoCompleterVariations.h"

// #pragma mark - BmDefaultPatternSelector
/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultPatternSelector
::SelectPatternBounds( const BmString& text, int32 caretPos,
							  int32* start, int32* length)
{
	if (!start || !length)
		return;
	*start = 0;
	*length = text.Length();
}

// #pragma mark - BmDefaultCompletionStyle
/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
BmDefaultCompletionStyle
::BmDefaultCompletionStyle(BmAutoCompleter::EditView* editView, 
									BmAutoCompleter::ChoiceModel* choiceModel,
									BmAutoCompleter::ChoiceView* choiceView, 
									BmAutoCompleter::PatternSelector* patternSelector)
	:	CompletionStyle(editView, choiceModel, choiceView, patternSelector)
	,	mSelectedIndex(-1)
	,	mPatternStartPos(0)
	,	mPatternLength(0)
{
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
BmDefaultCompletionStyle::~BmDefaultCompletionStyle()
{
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
bool BmDefaultCompletionStyle::Select(int32 index)
{
	if (!mChoiceView || !mChoiceModel || index == mSelectedIndex
	|| index < -1 || index >= mChoiceModel->CountChoices())
		return false;

	mSelectedIndex = index;
	mChoiceView->SelectChoiceAt(index);
	return true;
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
bool BmDefaultCompletionStyle::SelectNext(bool wrap)
{
	if (!mChoiceModel || mChoiceModel->CountChoices() == 0)
		return false;

	int32 newIndex = mSelectedIndex + 1;
	if (newIndex >= mChoiceModel->CountChoices()) {
		if (wrap)
			newIndex = 0;
		else
			newIndex = mSelectedIndex;
	}
	return Select(newIndex);
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
bool BmDefaultCompletionStyle::SelectPrevious(bool wrap)
{
	if (!mChoiceModel || mChoiceModel->CountChoices() == 0)
		return false;

	int32 newIndex = mSelectedIndex - 1;
	if (newIndex < 0) {
		if (wrap)
			newIndex = mChoiceModel->CountChoices()-1;
		else
			newIndex = 0;
	}
	return Select(newIndex);
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultCompletionStyle::ApplyChoice(bool hideChoices)
{
	if (!mChoiceModel || !mChoiceView || !mEditView || mSelectedIndex < 0)
		return;

	BmString completedText(mFullEnteredText);
	completedText.Remove(mPatternStartPos, mPatternLength);
	const BmString& choiceStr = mChoiceModel->ChoiceAt(mSelectedIndex)->Text();
	completedText.Insert(choiceStr, mPatternStartPos);

	mFullEnteredText = completedText;
	mEditView->SetEditViewState(completedText, 
										 mPatternStartPos+choiceStr.Length());
	if (hideChoices)
		mChoiceView->HideChoices();
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultCompletionStyle::CancelChoice()
{
	if (!mChoiceView || !mEditView)
		return;

	mEditView->SetEditViewState(mFullEnteredText, 
										 mPatternStartPos+mPatternLength);
	mChoiceView->HideChoices();
	Select(-1);
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultCompletionStyle::EditViewStateChanged()
{
	if (!mChoiceModel || !mChoiceView || !mEditView)
		return;

	BmString text;
	int32 caretPos;
	mEditView->GetEditViewState(text, &caretPos);
	if (mFullEnteredText == text)
		return;
	mFullEnteredText = text;
	mPatternSelector->SelectPatternBounds(text, caretPos, &mPatternStartPos, 
													  &mPatternLength);
	BmString pattern(text.String()+mPatternStartPos, mPatternLength);
	mChoiceModel->FetchChoicesFor(pattern);

	Select(-1);
	// show a single choice only if it doesn't match the pattern exactly:
	if (mChoiceModel->CountChoices() > 1
	|| mChoiceModel->CountChoices() == 1
		&& pattern.ICompare(mChoiceModel->ChoiceAt(0)->Text()) != 0) {
		mChoiceView->ShowChoices(this);
		mChoiceView->SelectChoiceAt(mSelectedIndex);
	} else
		mChoiceView->HideChoices();
}

// #pragma mark - BmDefaultChoiceView::ListView
static const int32 BM_INVOKED = 'bmin';
/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
BmDefaultChoiceView::ListView
::ListView(BmAutoCompleter::CompletionStyle* completer)
	:	BListView(BRect(0,0,100,100), "ChoiceViewList")
	,	mCompleter(completer)
{
	// we need to check if user clicks outside of window-bounds:
	SetEventMask(B_POINTER_EVENTS);
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultChoiceView::ListView::AttachedToWindow()
{
	SetTarget(this);
	SetInvocationMessage(new BMessage(BM_INVOKED));
	BListView::AttachedToWindow();
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultChoiceView::ListView::SelectionChanged()
{
	mCompleter->Select(CurrentSelection(0));
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultChoiceView::ListView::MessageReceived(BMessage* msg)
{
	switch(msg->what) {
		case BM_INVOKED:
			mCompleter->ApplyChoice();
			break;
		default:
			BListView::MessageReceived(msg);
	}
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultChoiceView::ListView::MouseDown(BPoint point)
{
	if (!Window()->Frame().Contains(ConvertToScreen(point)))
		// click outside of window, so we close it:
		Window()->Quit();
	else
		BListView::MouseDown(point);
}

// #pragma mark - BmDefaultChoiceView
/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
BmDefaultChoiceView::BmDefaultChoiceView()
	:	mWindow(NULL)
	,	mListView(NULL)
{
	
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
BmDefaultChoiceView::~BmDefaultChoiceView()
{
	HideChoices();
}
	
/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultChoiceView::SelectChoiceAt(int32 index)
{
	if (mListView && mListView->LockLooper()) {
		if (index < 0)
			mListView->DeselectAll();
		else {
			mListView->Select(index);
			mListView->ScrollToSelection();
		}
		mListView->UnlockLooper();
	}
}

class BmChoiceItem : public BStringItem
{
public:
	BmChoiceItem(const char* text)
		: BStringItem(text)
	{
	}
	virtual void DrawItem(BView* owner, BRect frame, bool complete = false)
	{
		if (IsSelected()) {
			BmSetLowUIColor(owner, B_UI_MENU_SELECTED_BACKGROUND_COLOR);
			BmSetHighUIColor(owner, B_UI_MENU_SELECTED_ITEM_TEXT_COLOR);
		}
		if (complete || IsSelected())
			owner->FillRect(frame, B_SOLID_LOW);
		BFont font;
		font_height fontHeight;
		owner->GetFont(&font);
		font.GetHeight(&fontHeight);
		owner->DrawString(Text(), 
								BPoint(frame.left+1, frame.top+2+fontHeight.ascent));
	}
private:
};

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultChoiceView::ShowChoices(BmAutoCompleter::CompletionStyle* completer)
{
	if (!completer)
		return;

	HideChoices();

	BmAutoCompleter::ChoiceModel* choiceModel = completer->GetChoiceModel();
	BmAutoCompleter::EditView* editView = completer->GetEditView();

	if (!editView || !choiceModel || choiceModel->CountChoices() == 0)
		return;

	mListView = new ListView(completer);
	int32 count = choiceModel->CountChoices();
	for(int32 i=0; i<count; ++i) {
		mListView->AddItem(
			new BmChoiceItem(choiceModel->ChoiceAt(i)->Text().String())
		);
	}

	mWindow = new BWindow(BRect(0,0,100,100), "", B_BORDERED_WINDOW_LOOK, 
								 B_NORMAL_WINDOW_FEEL,
								 B_NOT_MOVABLE | B_WILL_ACCEPT_FIRST_CLICK 
								 | B_AVOID_FOCUS | B_ASYNCHRONOUS_CONTROLS);
	mWindow->AddChild(mListView);

	int32 visibleCount = MIN(count, 5);
	float listHeight = mListView->ItemFrame(visibleCount-1).bottom+1;

	BRect pvRect = editView->GetAdjustmentFrame();
	BRect listRect = pvRect;
	listRect.bottom = listRect.top + listHeight - 1;
	BRect screenRect = BScreen().Frame();
	if (listRect.bottom+1+listHeight <= screenRect.bottom)
		listRect.OffsetTo(pvRect.left, pvRect.bottom+1);
	else
		listRect.OffsetTo(pvRect.left, pvRect.top-listHeight);

	mListView->MoveTo(0, 0);
	mListView->ResizeTo(listRect.Width(), listRect.Height());
	mWindow->MoveTo(listRect.left, listRect.top);
	mWindow->ResizeTo(listRect.Width(), listRect.Height());
	mWindow->Show();
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
void BmDefaultChoiceView::HideChoices()
{
	if (mWindow && mWindow->Lock()) {
		mWindow->Quit();
		mWindow = NULL;
		mListView = NULL;
	}
}

/*------------------------------------------------------------------------------*\
	( )
		-	
\*------------------------------------------------------------------------------*/
bool BmDefaultChoiceView::ChoicesAreShown()
{
	return (mWindow != NULL);
}
