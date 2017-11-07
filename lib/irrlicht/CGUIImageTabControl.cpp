#include "CGUIImageTabControl.h"

namespace irr
{
namespace gui
{
//! constructor
CGUIImageTab::CGUIImageTab(s32 number, IGUIEnvironment* environment,
	IGUIElement* parent, const core::rect<s32>& rectangle,
	s32 id, video::ITexture *texture, f32 scaling, s32 side)
	: IGUITab(environment, parent, id, rectangle), Number(number),
		BackColor(0,0,0,0), OverrideTextColorEnabled(false), TextColor(255,0,0,0),
		DrawBackground(false), 
		Texture(texture), Scaling(scaling), Side(side), Active(false), Drawn(false),
		DrawnRect(rectangle)
{
	#ifdef _DEBUG
	setDebugName("CGUIImageTab");
	#endif

	const IGUISkin* const skin = environment->getSkin();
	if (skin)
		TextColor = skin->getColor(EGDC_BUTTON_TEXT);
}


//! Returns number of tab in tabcontrol. Can be accessed
//! later IGUITabControl::getTab() by this number.
s32 CGUIImageTab::getNumber() const
{
	return Number;
}


//! Sets the number
void CGUIImageTab::setNumber(s32 n)
{
	Number = n;
}

void CGUIImageTab::refreshSkinColors()
{
	if ( !OverrideTextColorEnabled )
	{
		TextColor = Environment->getSkin()->getColor(EGDC_BUTTON_TEXT);
	}
}

//! draws the element and its children
void CGUIImageTab::draw()
{
	if (!IsVisible)
		return;

	IGUISkin *skin = Environment->getSkin();

	if (skin && DrawBackground)
		skin->draw2DRectangle(this, BackColor, AbsoluteRect, 0/*&AbsoluteClippingRect*/);
	
	IGUIElement::draw();
}


//! sets if the tab should draw its background
void CGUIImageTab::setDrawBackground(bool draw)
{
	DrawBackground = draw;
}


//! sets the color of the background, if it should be drawn.
void CGUIImageTab::setBackgroundColor(video::SColor c)
{
	BackColor = c;
}


//! sets the color of the text
void CGUIImageTab::setTextColor(video::SColor c)
{
	OverrideTextColorEnabled = true;
	TextColor = c;
}


video::SColor CGUIImageTab::getTextColor() const
{
	return TextColor;
}


//! returns true if the tab is drawing its background, false if not
bool CGUIImageTab::isDrawingBackground() const
{
	_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
	return DrawBackground;
}


//! returns the color of the background
video::SColor CGUIImageTab::getBackgroundColor() const
{
	return BackColor;
}


//! Writes attributes of the element.
void CGUIImageTab::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUITab::serializeAttributes(out,options);

	out->addInt		("TabNumber",		Number);
	out->addBool	("DrawBackground",	DrawBackground);
	out->addColor	("BackColor",		BackColor);
	out->addBool	("OverrideTextColorEnabled", OverrideTextColorEnabled);
	out->addColor	("TextColor",		TextColor);

}


//! Reads attributes of the element
void CGUIImageTab::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUITab::deserializeAttributes(in,options);

	setNumber(in->getAttributeAsInt("TabNumber"));
	setDrawBackground(in->getAttributeAsBool("DrawBackground"));
	setBackgroundColor(in->getAttributeAsColor("BackColor"));
	bool override = in->getAttributeAsBool("OverrideTextColorEnabled");
	setTextColor(in->getAttributeAsColor("TextColor"));
	if ( !override )
	{
		OverrideTextColorEnabled = false;
	}

	if (Parent && Parent->getType() == EGUIET_TAB_CONTROL)
	{
		((CGUIImageTabControl*)Parent)->addTab(this);
		if (isVisible())
			((CGUIImageTabControl*)Parent)->setActiveTab(this);
	}
}


//! Draws the tab image
void CGUIImageTab::drawImage(
	const irr::core::rect<s32>& frameRect
	)
{
	if (Texture)
	{
		f32 margin = 4;
		
		f32 max_width = ( frameRect.LowerRightCorner.X - frameRect.UpperLeftCorner.X - 2 * margin ) * Scaling;
		f32 max_height = ( frameRect.LowerRightCorner.Y - frameRect.UpperLeftCorner.Y - 2 * margin ) * Scaling;
		
		f32 tab_height = max_height;
		f32 tab_width = tab_height * Texture->getSize().Width / Texture->getSize().Height;
		
		if ( tab_width > max_width )
		{
			tab_height *= max_width / tab_width;
			tab_width = max_width;
		}
		
		f32 middle_x = ( frameRect.LowerRightCorner.X + frameRect.UpperLeftCorner.X ) * 0.5f;
		f32 middle_y = ( frameRect.LowerRightCorner.Y + frameRect.UpperLeftCorner.Y ) * 0.5f;
		
		video::IVideoDriver* driver = Environment->getVideoDriver();

		driver->draw2DImage(Texture,
			irr::core::rect<s32>(middle_x - tab_width * 0.5f, middle_y - tab_height * 0.5f, 
				middle_x + tab_width * 0.5f, middle_y + tab_height * 0.5f ), 
			irr::core::rect<s32>(0, 0, Texture->getSize().Width, Texture->getSize().Height), 
			0, 0, true);
	}
}

// ------------------------------------------------------------------
// Tabcontrol
// ------------------------------------------------------------------

//! constructor
CGUIImageTabControl::CGUIImageTabControl(IGUIEnvironment* environment,
	IGUIElement* parent, const core::rect<s32>& rectangle,
	bool fillbackground, bool border, s32 side, s32 id, 
	s32 tab_height, s32 tab_width, s32 tab_padding, 
	s32 tab_spacing, const core::rect<s32>& tab_rect, 
	s32 view_width, s32 view_height, const core::rect<s32>& view_rect)
	: IGUITabControl(environment, parent, id, rectangle),  
	Rect(rectangle), Tabs(), FillBackground(fillbackground), Border(border), Side(side),
	TabHeight(tab_height), TabWidth(tab_width), 
	TabPadding(tab_padding), TabSpacing(tab_spacing), TabRect(tab_rect),
	ViewWidth(view_width), ViewHeight(view_height), ViewRect(view_rect),
	VerticalAlignment(EGUIA_UPPERLEFT), 
	ScrollControl(false), UpButton(0), DownButton(0), ActiveTabIndex(-1), 
	FirstScrollTabIndex(0), LastScrollTabIndex(-1)
{
	#ifdef _DEBUG
	setDebugName("CGUIImageTabControl");
	#endif

	IGUISkin* skin = Environment->getSkin();
	IGUISpriteBank* sprites = 0;

	if ( TabHeight == 0 )
	{
		TabHeight = 32;

		if (skin)
		{
			sprites = skin->getSpriteBank();
			TabHeight = skin->getSize(gui::EGDS_BUTTON_HEIGHT) + 2;
		}
	}

	calcRects();
	
	UpButton = Environment->addButton(core::rect<s32>(0,0,10,10), this);

	if ( UpButton )
	{
		UpButton->setSpriteBank(sprites);
		UpButton->setVisible(false);
		UpButton->setSubElement(true);
		UpButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
		UpButton->setOverrideFont(Environment->getBuiltInFont());
		UpButton->grab();
	}

	DownButton = Environment->addButton(core::rect<s32>(0,0,10,10), this);

	if ( DownButton )
	{
		DownButton->setSpriteBank(sprites);
		DownButton->setVisible(false);
		DownButton->setSubElement(true);
		DownButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
		DownButton->setOverrideFont(Environment->getBuiltInFont());
		DownButton->grab();
	}

	setTabVerticalAlignment(EGUIA_UPPERLEFT);
	refreshSprites();
}

//! destructor
CGUIImageTabControl::~CGUIImageTabControl()
{
	for (u32 i=0; i<Tabs.size(); ++i)
	{
		if (Tabs[i])
			Tabs[i]->drop();
	}

	if (UpButton)
		UpButton->drop();

	if (DownButton)
		DownButton->drop();
}

void CGUIImageTabControl::refreshSprites()
{
	video::SColor color(255,255,255,255);
	IGUISkin* skin = Environment->getSkin();
	
	if (skin)
	{
		color = skin->getColor(isEnabled() ? EGDC_WINDOW_SYMBOL : EGDC_GRAY_WINDOW_SYMBOL);
	}

	if (UpButton)
	{
		UpButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_LEFT), color);
		UpButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_LEFT), color);
	}

	if (DownButton)
	{
		DownButton->setSprite(EGBS_BUTTON_UP, skin->getIcon(EGDI_CURSOR_RIGHT), color);
		DownButton->setSprite(EGBS_BUTTON_DOWN, skin->getIcon(EGDI_CURSOR_RIGHT), color);
	}
}

//! Adds a tab
IGUITab* CGUIImageTabControl::addTab(const wchar_t* caption, s32 id)
{
	return addImageTab(caption, id, 0);
}

//! Adds an image tab
CGUIImageTab* CGUIImageTabControl::addImageTab(const wchar_t* caption, s32 id, 
	video::ITexture *texture, f32 scaling)
{
	CGUIImageTab* tab = new CGUIImageTab(Tabs.size(), Environment, this, calcTabPos(), id, 
		texture, scaling);

	if (!texture)
	{
		tab->setText(caption);
	}
	
	tab->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	tab->setVisible(false);
	Tabs.push_back(tab);

	if (ActiveTabIndex == -1)
	{
		ActiveTabIndex = 0;
		tab->setVisible(true);
	}

	return tab;
}


//! adds a tab which has been created elsewhere
void CGUIImageTabControl::addTab(CGUIImageTab* tab)
{
	if (!tab)
		return;

	// check if its already added
	for (u32 i=0; i < Tabs.size(); ++i)
	{
		if (Tabs[i] == tab)
			return;
	}

	tab->grab();

	if (tab->getNumber() == -1)
		tab->setNumber((s32)Tabs.size());

	while (tab->getNumber() >= (s32)Tabs.size())
		Tabs.push_back(0);

	if (Tabs[tab->getNumber()])
	{
		Tabs.push_back(Tabs[tab->getNumber()]);
		Tabs[Tabs.size()-1]->setNumber(Tabs.size());
	}
	Tabs[tab->getNumber()] = tab;

	if (ActiveTabIndex == -1)
		ActiveTabIndex = tab->getNumber();


	if (tab->getNumber() == ActiveTabIndex)
	{
		setActiveTab(ActiveTabIndex);
	}
}

//! Insert the tab at the given index
IGUITab* CGUIImageTabControl::insertTab(s32 idx, const wchar_t* caption, s32 id)
{
	if ( idx < 0 || idx > (s32)Tabs.size() )	// idx == Tabs.size() is indeed ok here as core::array can handle that
		return NULL;

	CGUIImageTab* tab = new CGUIImageTab(idx, Environment, this, calcTabPos(), id);

	tab->setText(caption);
	tab->setAlignment(EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_LOWERRIGHT);
	tab->setVisible(false);
	Tabs.insert(tab, (u32)idx);

	if (ActiveTabIndex == -1)
	{
		ActiveTabIndex = 0;
		tab->setVisible(true);
	}

	for ( u32 i=(u32)idx+1; i < Tabs.size(); ++i )
	{
		Tabs[i]->setNumber(i);
	}

	return tab;
}

//! Removes a tab from the tabcontrol
void CGUIImageTabControl::removeTab(s32 idx)
{
	if ( idx < 0 || idx >= (s32)Tabs.size() )
		return;

	Tabs[(u32)idx]->drop();
	Tabs.erase((u32)idx);
	for ( u32 i=(u32)idx; i < Tabs.size(); ++i )
	{
		Tabs[i]->setNumber(i);
	}
}

//! Clears the tabcontrol removing all tabs
void CGUIImageTabControl::clear()
{
	for (u32 i=0; i<Tabs.size(); ++i)
	{
		if (Tabs[i])
			Tabs[i]->drop();
	}
	Tabs.clear();
}

//! Returns amount of tabs in the tabcontrol
s32 CGUIImageTabControl::getTabCount() const
{
	return Tabs.size();
}


//! Returns a tab based on zero based index
IGUITab* CGUIImageTabControl::getTab(s32 idx) const
{
	if ((u32)idx >= Tabs.size())
		return 0;

	return Tabs[idx];
}


//! called if an event happened.
bool CGUIImageTabControl::OnEvent(const SEvent& event)
{
	if (isEnabled())
	{

		switch(event.EventType)
		{
		case EET_GUI_EVENT:
			switch(event.GUIEvent.EventType)
			{
			case EGET_BUTTON_CLICKED:
				if (event.GUIEvent.Caller == UpButton)
				{
					scrollLeft();
					return true;
				}
				else if (event.GUIEvent.Caller == DownButton)
				{
					scrollRight();
					return true;
				}

			break;
			default:
			break;
			}
			break;
		case EET_MOUSE_INPUT_EVENT:
			switch(event.MouseInput.Event)
			{
			case EMIE_LMOUSE_PRESSED_DOWN:
				// todo: dragging tabs around
				return true;
			case EMIE_LMOUSE_LEFT_UP:
			{
				s32 idx = getTabAt(event.MouseInput.X, event.MouseInput.Y);
				if ( idx >= 0 )
				{
					setActiveTab(idx);
					return true;
				}
				break;
			}
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	return IGUIElement::OnEvent(event);
}


void CGUIImageTabControl::scrollLeft()
{
	if ( ScrollControl
	     && FirstScrollTabIndex > 0 )
	{
		--FirstScrollTabIndex;
	}
}


void CGUIImageTabControl::scrollRight()
{
	if ( ScrollControl
		 && FirstScrollTabIndex < (s32)(Tabs.size()) - 1 )
	{
		++FirstScrollTabIndex;
	}
}


s32 CGUIImageTabControl::calcTabWidth(s32 pos, IGUIFont* font, const wchar_t* text, bool withScrollControl,
	CGUIImageTab* tab) const
{
	if ( !font )
		return 0;

	s32 len = font->getDimension(text).Width + TabPadding;
	
	if ( tab->Texture )
	{
		len = TabHeight * tab->Scaling * tab->Texture->getSize().Width / tab->Texture->getSize().Height + TabPadding;
	}
			
	if ( TabWidth > 0 )
		len = TabWidth;

	// check if we miss the place to draw the tab-button
	if ( withScrollControl && ScrollControl && pos+len > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2 )
	{
		s32 tabMinWidth = font->getDimension(L"A").Width;
		
		if ( TabPadding > 0 && tabMinWidth < TabPadding )
			tabMinWidth = TabPadding;
	}
	
	return len;
}


void CGUIImageTabControl::calcRects()
{	
	Rect.UpperLeftCorner.X += AbsoluteRect.UpperLeftCorner.X;
	Rect.UpperLeftCorner.Y += AbsoluteRect.UpperLeftCorner.Y;
	Rect.LowerRightCorner.X += AbsoluteRect.UpperLeftCorner.X;
	Rect.LowerRightCorner.Y += AbsoluteRect.UpperLeftCorner.Y;
	
	TabRect.UpperLeftCorner.X += AbsoluteRect.UpperLeftCorner.X;
	TabRect.UpperLeftCorner.Y += AbsoluteRect.UpperLeftCorner.Y;
	TabRect.LowerRightCorner.X += AbsoluteRect.UpperLeftCorner.X;
	TabRect.LowerRightCorner.Y += AbsoluteRect.UpperLeftCorner.Y;
	
	ViewRect.UpperLeftCorner.X += AbsoluteRect.UpperLeftCorner.X;
	ViewRect.UpperLeftCorner.Y += AbsoluteRect.UpperLeftCorner.Y;
	ViewRect.LowerRightCorner.X += AbsoluteRect.UpperLeftCorner.X;
	ViewRect.LowerRightCorner.Y += AbsoluteRect.UpperLeftCorner.Y;
	
	if ( Side == 0 )
	{
		ViewRect.UpperLeftCorner.Y += TabHeight;
		ViewRect.LowerRightCorner.Y += TabHeight;
	}
	else if ( Side == 2 )
	{
		ViewRect.UpperLeftCorner.X += TabWidth;
		ViewRect.LowerRightCorner.X += TabWidth;
	}
}


void CGUIImageTabControl::calcTabs()
{	
	if ( !IsVisible )
		return;

	IGUISkin* skin = Environment->getSkin();
	
	if ( !skin )
		return;

	IGUIFont* font = skin->getFont();
	
	if ( !font )
		return;

	if ( FirstScrollTabIndex >= (s32)Tabs.size() )
		FirstScrollTabIndex = ((s32)Tabs.size()) - 1;

	if ( FirstScrollTabIndex < 0 )
		FirstScrollTabIndex = 0;
		
	s32 pos;

	if ( Side < 2 )
	{
		pos = ViewRect.UpperLeftCorner.X;
	}
	else
	{
		pos = ViewRect.UpperLeftCorner.Y;
	}
	
	CGUIImageTab* tab;
	
	for (u32 i=FirstScrollTabIndex; i<Tabs.size(); ++i)
	{
		tab = Tabs[i];
		
		if ( tab )
		{
			tab->Active = false;
			tab->Drawn = false;
		}
	}
	
	core::rect<s32> drawnRect;
	
	LastScrollTabIndex = -1;

	for (u32 i=FirstScrollTabIndex; i<Tabs.size(); ++i)
	{
		tab = Tabs[i];
		
		if ( tab )
		{
			const wchar_t* text = 0;
		
			text = Tabs[i]->getText();

			// get text length
			s32 len = calcTabWidth(pos, font, text, true, tab);
					
			if ( Side < 2 )
			{
				drawnRect.UpperLeftCorner.X = pos + 1;
				pos += len + TabSpacing;
				
				if ( ScrollControl
					 && pos > ViewRect.LowerRightCorner.X - 2 * ( TabHeight + TabSpacing ) )
				{
					break;		
				}				
				
				if ( pos > ViewRect.LowerRightCorner.X )
				{
					ScrollControl = true;	
					break;		
				}				
			}
			else
			{
				drawnRect.UpperLeftCorner.Y = pos + 1;			
				pos += TabHeight + TabSpacing;
				
				if ( ScrollControl
					 && pos > ViewRect.LowerRightCorner.Y - 2 * ( TabHeight + TabSpacing ) )
				{
					break;		
				}				
				
				if ( pos > ViewRect.LowerRightCorner.Y )
				{			
					ScrollControl = true;
					break;		
				}
			}

			if ( Side == 0 )
			{
				drawnRect.UpperLeftCorner.Y = ViewRect.UpperLeftCorner.Y + 2;
			}
			else if ( Side == 1 )
			{
				drawnRect.UpperLeftCorner.Y = ViewRect.LowerRightCorner.Y + 2;
			}
			else if ( Side == 2 )
			{
				drawnRect.UpperLeftCorner.X = ViewRect.UpperLeftCorner.X + 2;
			}
			else
			{
				drawnRect.UpperLeftCorner.X = ViewRect.LowerRightCorner.X + 2;
			}
			
			drawnRect.LowerRightCorner.X = drawnRect.UpperLeftCorner.X + len - 2;
			drawnRect.LowerRightCorner.Y = drawnRect.UpperLeftCorner.Y + TabHeight - 2;

			if ( i == (u32)ActiveTabIndex )
			{
				tab->Active = true;
				
				drawnRect.UpperLeftCorner.X -= 2;
				drawnRect.UpperLeftCorner.Y -= 2;
				drawnRect.LowerRightCorner.X += 2;
				drawnRect.LowerRightCorner.Y += 2;
			}
			
			tab->Drawn = true;
			tab->DrawnRect = drawnRect;
			
			if ( text )
				tab->refreshSkinColors();
				
			LastScrollTabIndex = i;
		}
	}
}


void CGUIImageTabControl::calcScrollButtons()
{
	core::rect<s32> buttonRect;
	
	if ( Side < 2 )
	{
		buttonRect.UpperLeftCorner.X = ViewRect.getWidth() - 2 * ( TabHeight + TabSpacing );
		
		if ( Side == 0 )
		{
			buttonRect.UpperLeftCorner.Y = 0;
		}
		else
		{
			buttonRect.UpperLeftCorner.Y = ViewRect.getHeight();
		}
		
		buttonRect.LowerRightCorner.X = buttonRect.UpperLeftCorner.X + TabHeight;
		buttonRect.LowerRightCorner.Y = buttonRect.UpperLeftCorner.Y + TabHeight;
		UpButton->setRelativePosition(buttonRect);

		buttonRect.UpperLeftCorner.X += TabHeight + TabSpacing;
		
		buttonRect.LowerRightCorner.X = buttonRect.UpperLeftCorner.X + TabHeight;
		buttonRect.LowerRightCorner.Y = buttonRect.UpperLeftCorner.Y + TabHeight;
		DownButton->setRelativePosition(buttonRect);
	}
	else
	{
		buttonRect.UpperLeftCorner.Y = ViewRect.getHeight() - 2 * ( TabHeight + TabSpacing );
		
		if ( Side == 2 )
		{
			buttonRect.UpperLeftCorner.X = TabWidth - TabHeight;
		}
		else
		{
			buttonRect.UpperLeftCorner.X = ViewRect.getWidth();
		}
		
		buttonRect.LowerRightCorner.X = buttonRect.UpperLeftCorner.X + TabHeight;
		buttonRect.LowerRightCorner.Y = buttonRect.UpperLeftCorner.Y + TabHeight;
		UpButton->setRelativePosition(buttonRect);

		buttonRect.UpperLeftCorner.Y += TabHeight + TabSpacing;
		
		buttonRect.LowerRightCorner.X = buttonRect.UpperLeftCorner.X + TabHeight;
		buttonRect.LowerRightCorner.Y = buttonRect.UpperLeftCorner.Y + TabHeight;
		DownButton->setRelativePosition(buttonRect);
	}
	
	if (!UpButton || !DownButton)
		return;
		
		UpButton->setVisible( ScrollControl );
		DownButton->setVisible( ScrollControl );

	bringToFront( UpButton );
	bringToFront( DownButton );
}


core::rect<s32> CGUIImageTabControl::calcTabPos()
{
	core::rect<s32> r;
	r.UpperLeftCorner.X = 0;
	r.LowerRightCorner.X = AbsoluteRect.getWidth();
	
	if ( Border )
	{
		++r.UpperLeftCorner.X;
		--r.LowerRightCorner.X;
	}

	if ( VerticalAlignment == EGUIA_UPPERLEFT )
	{
		r.UpperLeftCorner.Y = TabHeight+2;
		r.LowerRightCorner.Y = AbsoluteRect.getHeight()-1;
		
		if ( Border )
		{
			--r.LowerRightCorner.Y;
		}
	}
	else
	{
		r.UpperLeftCorner.Y = 0;
		r.LowerRightCorner.Y = AbsoluteRect.getHeight()-(TabHeight+2);
		
		if ( Border )
		{
			++r.UpperLeftCorner.Y;
		}
	}

	return r;
}


//! draws the element and its children
void CGUIImageTabControl::draw()
{
	if ( !IsVisible )
		return;

	IGUISkin* skin = Environment->getSkin();
	
	if ( !skin )
		return;

	IGUIFont* font = skin->getFont();
	video::IVideoDriver* driver = Environment->getVideoDriver();

	if ( !font )
		return;

	calcTabs();
	calcScrollButtons();
	
	if ( Tabs.empty() )
	{
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), AbsoluteRect, 0/*&AbsoluteClippingRect*/);
	}
	
	CGUIImageTab* activeTab = 0;
		
	for (s32 i=FirstScrollTabIndex; i<=LastScrollTabIndex; ++i)
	{
		CGUIImageTab* tab = Tabs[i];
		
		if ( tab )
		{	
			// get Text
			const wchar_t* text = 0;
			
			if ( i == ActiveTabIndex )
			{
				activeTab = tab;
			}
			else
			{
				core::rect<s32> frameRect(tab->DrawnRect);
				
				text = tab->getText();

				skin->draw3DTabButton(this, false, frameRect, 0/*&AbsoluteClippingRect*/, VerticalAlignment);

				if ( text )
				{
					// draw text
					core::rect<s32> textClipRect(frameRect);	// TODO: exact size depends on borders in draw3DTabButton which we don't get with current interface
					//textClipRect.clipAgainst(AbsoluteClippingRect);
					font->draw(text, frameRect, Tabs[i]->getTextColor(),
						true, true, &textClipRect);
				}
					
				Tabs[i]->drawImage(frameRect);
			}
		}
	}

	// draw active tab
	if ( activeTab != 0 )
	{
		core::rect<s32> frameRect(activeTab->DrawnRect);
		core::rect<s32> tr;
		
		// draw upper highlight frame

		skin->draw3DTabButton(this, true, frameRect, 0/*&AbsoluteClippingRect*/, VerticalAlignment);

		// draw text
		core::rect<s32> textClipRect(frameRect);	// TODO: exact size depends on borders in draw3DTabButton which we don't get with current interface
		//textClipRect.clipAgainst(AbsoluteClippingRect);
		font->draw(activeTab->getText(), frameRect, activeTab->getTextColor(),
			true, true, &textClipRect);

		tr.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
		tr.LowerRightCorner.X = frameRect.UpperLeftCorner.X - 1;
		tr.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - 1;
		tr.LowerRightCorner.Y = frameRect.LowerRightCorner.Y;
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), tr, 0); //&AbsoluteClippingRect);

		tr.UpperLeftCorner.X = frameRect.LowerRightCorner.X;
		tr.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), tr, 0); //&AbsoluteClippingRect);
		
		activeTab->drawImage(frameRect);
	}

	skin->draw3DTabBody(this, Border, FillBackground, AbsoluteRect, 0/*&AbsoluteClippingRect*/, TabHeight, VerticalAlignment);

	// enable scrollcontrols on need

	if ( UpButton )
		UpButton->setEnabled(ScrollControl);
	
	if ( DownButton )
		DownButton->setEnabled(ScrollControl);
		
	refreshSprites();

	IGUIElement::draw();
	
	driver->draw2DRectangle(video::SColor(32,255,0,0), AbsoluteRect, 0);
	//driver->draw2DRectangle(video::SColor(32,255,0,0), Rect, 0);
	//driver->draw2DRectangle(video::SColor(32,0,255,0), TabRect, 0);
	//driver->draw2DRectangle(video::SColor(32,0,0,255), ViewRect, 0);
}


//! Set the height of the tabs
void CGUIImageTabControl::setTabHeight( s32 height )
{
	if ( height < 0 )
		height = 0;

	TabHeight = height;
}


//! Get the height of the tabs
s32 CGUIImageTabControl::getTabHeight() const
{
	return TabHeight;
}

//! set the maximal width of a tab. Per default width is 0 which means "no width restriction".
void CGUIImageTabControl::setTabWidth(s32 width )
{
	TabWidth = width;
}

//! get the maximal width of a tab
s32 CGUIImageTabControl::getTabWidth() const
{
	return TabWidth;
}


//! Set the extra width added to tabs on each side of the text
void CGUIImageTabControl::setTabPadding( s32 padding )
{
	if ( padding < 0 )
		padding = 0;

	TabPadding = padding;
}


//! Get the extra width added to tabs on each side of the text
s32 CGUIImageTabControl::getTabPadding() const
{
	return TabPadding;
}


//! Set the alignment of the tabs
void CGUIImageTabControl::setTabVerticalAlignment( EGUI_ALIGNMENT alignment )
{
	VerticalAlignment = alignment;

	core::rect<s32> r(calcTabPos());
	for ( u32 i=0; i<Tabs.size(); ++i )
	{
		Tabs[i]->setRelativePosition(r);
	}
}

//! Get the alignment of the tabs
EGUI_ALIGNMENT CGUIImageTabControl::getTabVerticalAlignment() const
{
	return VerticalAlignment;
}


s32 CGUIImageTabControl::getTabAt(s32 xpos, s32 ypos) const
{
	core::position2di p(xpos, ypos);

	for (s32 i=FirstScrollTabIndex; i<=LastScrollTabIndex; ++i)
	{
		CGUIImageTab* tab = Tabs[i];
		
		if ( tab )
		{
			if ( tab->Drawn
			     && tab->DrawnRect.isPointInside(p))
			{
				return i;
			}
		}
	}
	return -1;
}

//! Returns which tab is currently active
s32 CGUIImageTabControl::getActiveTab() const
{
	return ActiveTabIndex;
}


//! Brings a tab to front.
bool CGUIImageTabControl::setActiveTab(s32 idx)
{
	if ((u32)idx >= Tabs.size())
		return false;

	bool changed = (ActiveTabIndex != idx);

	ActiveTabIndex = idx;

	for (s32 i=0; i<(s32)Tabs.size(); ++i)
		if (Tabs[i])
			Tabs[i]->setVisible( i == ActiveTabIndex );

	if (changed)
	{
		SEvent event;
		event.EventType = EET_GUI_EVENT;
		event.GUIEvent.Caller = this;
		event.GUIEvent.Element = 0;
		event.GUIEvent.EventType = EGET_TAB_CHANGED;
		Parent->OnEvent(event);
	}

	return true;
}


bool CGUIImageTabControl::setActiveTab(IGUITab *tab)
{
	for (s32 i=0; i<(s32)Tabs.size(); ++i)
		if (Tabs[i] == tab)
			return setActiveTab(i);
	return false;
}


//! Removes a child.
void CGUIImageTabControl::removeChild(IGUIElement* child)
{
	bool isTab = false;

	u32 i=0;
	// check if it is a tab
	while (i<Tabs.size())
	{
		if (Tabs[i] == child)
		{
			Tabs[i]->drop();
			Tabs.erase(i);
			isTab = true;
		}
		else
			++i;
	}

	// reassign numbers
	if (isTab)
	{
		for (i=0; i<Tabs.size(); ++i)
			if (Tabs[i])
				Tabs[i]->setNumber(i);
	}

	// remove real element
	IGUIElement::removeChild(child);
}


//! Update the position of the element, decides scroll button status
void CGUIImageTabControl::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
}


//! Writes attributes of the element.
void CGUIImageTabControl::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUITabControl::serializeAttributes(out,options);

	out->addInt ("ActiveTabIndex",	ActiveTabIndex);
	out->addBool("Border",		Border);
	out->addBool("FillBackground",	FillBackground);
	out->addInt ("TabHeight",	TabHeight);
	out->addInt ("TabWidth", TabWidth);
	out->addEnum("TabVerticalAlignment", s32(VerticalAlignment), GUIAlignmentNames);
}


//! Reads attributes of the element
void CGUIImageTabControl::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	Border          = in->getAttributeAsBool("Border");
	FillBackground  = in->getAttributeAsBool("FillBackground");

	ActiveTabIndex = -1;

	setTabHeight(in->getAttributeAsInt("TabHeight"));
	TabWidth     = in->getAttributeAsInt("TabWidth");

	IGUITabControl::deserializeAttributes(in,options);

	setActiveTab(in->getAttributeAsInt("ActiveTabIndex"));
	setTabVerticalAlignment( static_cast<EGUI_ALIGNMENT>(in->getAttributeAsEnumeration("TabVerticalAlignment" , GUIAlignmentNames)) );
}
} // end namespace irr
} // end namespace gui



















