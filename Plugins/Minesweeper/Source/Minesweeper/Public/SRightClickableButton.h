#pragma once
#include <Widgets/Input/SButton.h>

/*
* Code adapted from SButton.cpp to add a new delegate to handle Right Mouse Button clicks via an
* added delegate.
*/
class SRightClickableButton : public SButton
{
public:
	
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	
	/** See OnClicked event */
	void SetOnRightMouseButtonClicked(FOnClicked InOnClicked);

protected:
	/** The delegate to execute when the button is clicked */
	FOnClicked OnRightMouseButtonClicked;
	
	FReply ExecuteOnRightMouseButtonClick();
};
