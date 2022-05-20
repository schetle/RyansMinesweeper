#include "SRightClickableButton.h"

FReply SRightClickableButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Let super handle anything but the right mouse button
	if (MouseEvent.GetEffectingButton() != EKeys::RightMouseButton)
	{
		return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
	}

	/*
	 * Code adapted with changes only to "RightMouseButton" handling from SButton.cpp
	 */
	FReply Reply = FReply::Unhandled();
	if (IsEnabled() && (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton || MouseEvent.IsTouchEvent()))
	{
		Press();
		PressedScreenSpacePosition = MouseEvent.GetScreenSpacePosition();

		EButtonClickMethod::Type InputClickMethod = GetClickMethodFromInputType(MouseEvent);
		
		if(InputClickMethod == EButtonClickMethod::MouseDown)
		{
			//get the reply from the execute function
			Reply = ExecuteOnRightMouseButtonClick();

			//You should ALWAYS handle the OnClicked event.
			ensure(Reply.IsEventHandled() == true);
		}
		else if (InputClickMethod == EButtonClickMethod::PreciseClick)
		{
			// do not capture the pointer for precise taps or clicks
			// 
			Reply = FReply::Handled();
		}
		else
		{
			//we need to capture the mouse for MouseUp events
			Reply = FReply::Handled().CaptureMouse( AsShared() );
		}
	}

	Invalidate(EInvalidateWidget::Layout);

	//return the constructed reply
	return Reply;
}

FReply SRightClickableButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Let super handle anything but the right mouse button
	if (MouseEvent.GetEffectingButton() != EKeys::RightMouseButton)
	{
		return SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
	}

	/*
	 * Code adapted with changes only to "RightMouseButton" handling from SButton.cpp
	 */
	FReply Reply = FReply::Unhandled();
	const EButtonClickMethod::Type InputClickMethod = GetClickMethodFromInputType(MouseEvent);
	const bool bMustBePressed = InputClickMethod == EButtonClickMethod::DownAndUp || InputClickMethod == EButtonClickMethod::PreciseClick;
	const bool bMeetsPressedRequirements = (!bMustBePressed || (bIsPressed && bMustBePressed));

	if (bMeetsPressedRequirements && ( ( MouseEvent.GetEffectingButton() == EKeys::RightMouseButton || MouseEvent.IsTouchEvent())))
	{
		Release();

		if ( IsEnabled() )
		{
			if (InputClickMethod == EButtonClickMethod::MouseDown )
			{
				// NOTE: If we're configured to click on mouse-down/precise-tap, then we never capture the mouse thus
				//       may never receive an OnMouseButtonUp() call.  We make sure that our bIsPressed
				//       state is reset by overriding OnMouseLeave().
			}
			else
			{
				bool bEventOverButton = IsHovered();

				if ( !bEventOverButton && MouseEvent.IsTouchEvent() )
				{
					bEventOverButton = MyGeometry.IsUnderLocation(MouseEvent.GetScreenSpacePosition());
				}

				if ( bEventOverButton )
				{
					// If we asked for a precise tap, all we need is for the user to have not moved their pointer very far.
					const bool bTriggerForTouchEvent = InputClickMethod == EButtonClickMethod::PreciseClick;

					// If we were asked to allow the button to be clicked on mouse up, regardless of whether the user
					// pressed the button down first, then we'll allow the click to proceed without an active capture
					const bool bTriggerForMouseEvent = (InputClickMethod == EButtonClickMethod::MouseUp || HasMouseCapture() );

					if ( ( bTriggerForTouchEvent || bTriggerForMouseEvent ) )
					{
						Reply = ExecuteOnRightMouseButtonClick();
					}
				}
			}
		}
		
		//If the user of the button didn't handle this click, then the button's
		//default behavior handles it.
		if ( Reply.IsEventHandled() == false )
		{
			Reply = FReply::Handled();
		}
	}

	//If the user hasn't requested a new mouse captor and the button still has mouse capture,
	//then the default behavior of the button is to release mouse capture.
	if ( Reply.GetMouseCaptor().IsValid() == false && HasMouseCapture() )
	{
		Reply.ReleaseMouseCapture();
	}

	Invalidate(EInvalidateWidget::Layout);

	return Reply;
	
}

void SRightClickableButton::SetOnRightMouseButtonClicked(FOnClicked InOnClicked)
{
	OnRightMouseButtonClicked = InOnClicked;
}

FReply SRightClickableButton::ExecuteOnRightMouseButtonClick()
{
	/*
	 * Code adapted with changes only to call our RightMouseButton handler handling from SButton.cpp
	 */
	
	if (OnRightMouseButtonClicked.IsBound())
	{
		FReply Reply = OnRightMouseButtonClicked.Execute();
#if WITH_ACCESSIBILITY
		FSlateApplicationBase::Get().GetAccessibleMessageHandler()->OnWidgetEventRaised(AsShared(), EAccessibleEvent::Activate);
#endif
		return Reply;
	}
	else
	{
		return FReply::Handled();
	}
}
