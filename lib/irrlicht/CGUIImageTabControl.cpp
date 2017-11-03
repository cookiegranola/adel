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
		Texture(texture), Scaling(scaling), Side(side)
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
		skin->draw2DRectangle(this, BackColor, AbsoluteRect, &AbsoluteClippingRect);
	
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
	bool fillbackground, bool border, s32 id, s32 tab_height, 
	s32 side, s32 view_width, s32 view_height)
	: IGUITabControl(environment, parent, id, rectangle), ActiveTab(-1),
	Border(border), FillBackground(fillbackground), ScrollControl(false), TabHeight(tab_height), VerticalAlignment(EGUIA_UPPERLEFT),
	UpButton(0), DownButton(0), TabMaxWidth(0), CurrentScrollTabIndex(0), TabExtraWidth(20), 
	Side(side), ViewWidth(view_width), ViewHeight(view_height)
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

	ViewRect.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
	ViewRect.UpperLeftCorner.Y = AbsoluteRect.UpperLeftCorner.Y + TabHeight;
	ViewRect.LowerRightCorner.X = ViewRect.UpperLeftCorner.X + ViewWidth;
	ViewRect.LowerRightCorner.Y = ViewRect.UpperLeftCorner.Y + TabHeight + ViewHeight;

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

	if (ActiveTab == -1)
	{
		ActiveTab = 0;
		tab->setVisible(true);
	}

	recalculateScrollBar();

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

	if (ActiveTab == -1)
		ActiveTab = tab->getNumber();


	if (tab->getNumber() == ActiveTab)
	{
		setActiveTab(ActiveTab);
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

	if (ActiveTab == -1)
	{
		ActiveTab = 0;
		tab->setVisible(true);
	}

	for ( u32 i=(u32)idx+1; i < Tabs.size(); ++i )
	{
		Tabs[i]->setNumber(i);
	}

	recalculateScrollBar();

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
	if ( CurrentScrollTabIndex > 0 )
		--CurrentScrollTabIndex;
	recalculateScrollBar();
}


void CGUIImageTabControl::scrollRight()
{
	if ( CurrentScrollTabIndex < (s32)(Tabs.size()) - 1 )
	{
		if ( needScrollControl(CurrentScrollTabIndex, true) )
			++CurrentScrollTabIndex;
	}
	recalculateScrollBar();
}

s32 CGUIImageTabControl::calcTabWidth(s32 pos, IGUIFont* font, const wchar_t* text, bool withScrollControl,
	CGUIImageTab* tab) const
{
	if ( !font )
		return 0;

	s32 len = font->getDimension(text).Width + TabExtraWidth;
	
	if ( tab->Texture )
	{
		len = TabHeight * tab->Scaling * tab->Texture->getSize().Width / tab->Texture->getSize().Height + TabExtraWidth;
	}
	
	if ( TabMaxWidth > 0 && len > TabMaxWidth )
		len = TabMaxWidth;

	// check if we miss the place to draw the tab-button
	if ( withScrollControl && ScrollControl && pos+len > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2 )
	{
		s32 tabMinWidth = font->getDimension(L"A").Width;
		if ( TabExtraWidth > 0 && TabExtraWidth > tabMinWidth )
			tabMinWidth = TabExtraWidth;

		if ( ScrollControl && pos+tabMinWidth <= UpButton->getAbsolutePosition().UpperLeftCorner.X - 2 )
		{
			len = UpButton->getAbsolutePosition().UpperLeftCorner.X - 2 - pos;
		}
	}
	return len;
}

bool CGUIImageTabControl::needScrollControl(s32 startIndex, bool withScrollControl)
{
	if ( startIndex >= (s32)Tabs.size() )
		startIndex -= 1;

	if ( startIndex < 0 )
		startIndex = 0;

	IGUISkin* skin = Environment->getSkin();
	if (!skin)
		return false;

	IGUIFont* font = skin->getFont();

	core::rect<s32> frameRect(AbsoluteRect);

	if (Tabs.empty())
		return false;

	if (!font)
		return false;

	s32 pos = frameRect.UpperLeftCorner.X + 2;

	for (s32 i=startIndex; i<(s32)Tabs.size(); ++i)
	{
		// get Text
		const wchar_t* text = 0;
		if (Tabs[i])
			text = Tabs[i]->getText();

		// get text length
		s32 len = calcTabWidth(pos, font, text, false, Tabs[i]);	// always without withScrollControl here or len would be shortened

		frameRect.LowerRightCorner.X += len;

		frameRect.UpperLeftCorner.X = pos;
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + len;
		pos += len;

		if ( withScrollControl && pos > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2)
			return true;

		if ( !withScrollControl && pos > AbsoluteRect.LowerRightCorner.X )
			return true;
	}

	return false;
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

	core::rect<s32> frameRect(AbsoluteRect);

	if ( Tabs.empty() )
	{
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), frameRect, &AbsoluteClippingRect);
	}
	
	if ( !font )
		return;

	core::rect<s32> tr;
	s32 pos;

	if ( Side < 2 )
	{
		pos = ViewRect.UpperLeftCorner.X;
	}
	else
	{
		pos = ViewRect.UpperLeftCorner.Y;
	}

		
printf("View %d %d\n", ViewWidth, ViewHeight);
printf("ViewRect %d %d %d %d\n", ViewRect.UpperLeftCorner.X, ViewRect.UpperLeftCorner.Y, ViewRect.LowerRightCorner.X, ViewRect.LowerRightCorner.Y);
printf("AbsoluteRect %d %d %d %d\n", AbsoluteRect.UpperLeftCorner.X, AbsoluteRect.UpperLeftCorner.Y, AbsoluteRect.LowerRightCorner.X, AbsoluteRect.LowerRightCorner.Y);

	bool needLeftScroll = CurrentScrollTabIndex > 0;
	bool needRightScroll = false;


	CGUIImageTab *activeTab = 0;
	core::rect<s32> activeRect;

	for (u32 i=CurrentScrollTabIndex; i<Tabs.size(); ++i)
	{
		// get Text
		const wchar_t* text = 0;
		
		if ( Tabs[i] )
		{
			text = Tabs[i]->getText();
		}

		// get text length
		s32 len = calcTabWidth(pos, font, text, true, Tabs[i]);
				
		if ( ScrollControl && pos+len > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2 )
		{
			needRightScroll = true;
			break;
		}

		if ( Side < 2 )
		{
			frameRect.UpperLeftCorner.X = pos + 1;
			pos += len;
		}
		else
		{
			frameRect.UpperLeftCorner.Y = pos + 1;			
			pos += TabHeight;
		}

		if ( Side == 0 )
		{
			frameRect.UpperLeftCorner.Y = ViewRect.UpperLeftCorner.Y - TabHeight + 1;
		}
		else if ( Side == 1 )
		{
			frameRect.UpperLeftCorner.Y = ViewRect.LowerRightCorner.Y + 1;
		}
		else if ( Side == 2 )
		{
			frameRect.UpperLeftCorner.X = ViewRect.UpperLeftCorner.X - len + 1;
		}
		else
		{
			frameRect.UpperLeftCorner.X = ViewRect.LowerRightCorner.X + 1;
		}
		
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + len - 1;
		frameRect.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y + TabHeight - 1;
		
		if ( text )
			Tabs[i]->refreshSkinColors();
printf("frameRect %d %d %d %d\n", frameRect.UpperLeftCorner.X, frameRect.UpperLeftCorner.Y, frameRect.LowerRightCorner.X, frameRect.LowerRightCorner.Y);

		if ( (s32)i == ActiveTab )
		{
			activeTab = Tabs[i];
			activeRect = frameRect;
			//activetext = text;
		}
		else
		{
			skin->draw3DTabButton(this, false, frameRect, &AbsoluteClippingRect, VerticalAlignment);

			if ( text )
			{
				// draw text
				core::rect<s32> textClipRect(frameRect);	// TODO: exact size depends on borders in draw3DTabButton which we don't get with current interface
				textClipRect.clipAgainst(AbsoluteClippingRect);
				font->draw(text, frameRect, Tabs[i]->getTextColor(),
					true, true, &textClipRect);
			}
				
			Tabs[i]->drawImage(frameRect);
		}
	}

	// draw active tab
	if ( activeTab != 0 )
	{
		activeRect.UpperLeftCorner.X -= 2;
		activeRect.UpperLeftCorner.Y -= 2;
		activeRect.LowerRightCorner.X += 2;
		activeRect.LowerRightCorner.Y += 2;
	
		// draw upper highlight frame

		skin->draw3DTabButton(this, true, activeRect, &AbsoluteClippingRect, VerticalAlignment);

		// draw text
		core::rect<s32> textClipRect(activeRect);	// TODO: exact size depends on borders in draw3DTabButton which we don't get with current interface
		textClipRect.clipAgainst(AbsoluteClippingRect);
		font->draw(activeTab->getText(), activeRect, activeTab->getTextColor(),
			true, true, &textClipRect);

		tr.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
		tr.LowerRightCorner.X = activeRect.UpperLeftCorner.X - 1;
		tr.UpperLeftCorner.Y = activeRect.LowerRightCorner.Y - 1;
		tr.LowerRightCorner.Y = activeRect.LowerRightCorner.Y;
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), tr, &AbsoluteClippingRect);

		tr.UpperLeftCorner.X = activeRect.LowerRightCorner.X;
		tr.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
		driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), tr, &AbsoluteClippingRect);
		
		activeTab->drawImage(activeRect);
	}

	/*
				tr.UpperLeftCorner.X = AbsoluteRect.UpperLeftCorner.X;
				tr.LowerRightCorner.X = AbsoluteRect.LowerRightCorner.X;
				tr.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - 1;
				tr.LowerRightCorner.Y = frameRect.LowerRightCorner.Y;
				driver->draw2DRectangle(skin->getColor(EGDC_3D_HIGH_LIGHT), tr, &AbsoluteClippingRect);
	*/

	skin->draw3DTabBody(this, Border, FillBackground, AbsoluteRect, &AbsoluteClippingRect, TabHeight, VerticalAlignment);

	// enable scrollcontrols on need
	if ( UpButton )
		UpButton->setEnabled(needLeftScroll);
	if ( DownButton )
		DownButton->setEnabled(needRightScroll);
	refreshSprites();

	IGUIElement::draw();
	
	//driver->draw2DRectangle(video::SColor(32,255,255,32), ViewRect, 0);
}


//! Set the height of the tabs
void CGUIImageTabControl::setTabHeight( s32 height )
{
	if ( height < 0 )
		height = 0;

	TabHeight = height;

	recalculateScrollButtonPlacement();
	recalculateScrollBar();
}


//! Get the height of the tabs
s32 CGUIImageTabControl::getTabHeight() const
{
	return TabHeight;
}

//! set the maximal width of a tab. Per default width is 0 which means "no width restriction".
void CGUIImageTabControl::setTabMaxWidth(s32 width )
{
	TabMaxWidth = width;
}

//! get the maximal width of a tab
s32 CGUIImageTabControl::getTabMaxWidth() const
{
	return TabMaxWidth;
}


//! Set the extra width added to tabs on each side of the text
void CGUIImageTabControl::setTabExtraWidth( s32 extraWidth )
{
	if ( extraWidth < 0 )
		extraWidth = 0;

	TabExtraWidth = extraWidth;

	recalculateScrollBar();
}


//! Get the extra width added to tabs on each side of the text
s32 CGUIImageTabControl::getTabExtraWidth() const
{
	return TabExtraWidth;
}


void CGUIImageTabControl::recalculateScrollBar()
{
	if (!UpButton || !DownButton)
		return;

	ScrollControl = needScrollControl() || CurrentScrollTabIndex > 0;

	if (ScrollControl)
	{
		UpButton->setVisible( true );
		DownButton->setVisible( true );
	}
	else
	{
		UpButton->setVisible( false );
		DownButton->setVisible( false );
	}

	bringToFront( UpButton );
	bringToFront( DownButton );
}

//! Set the alignment of the tabs
void CGUIImageTabControl::setTabVerticalAlignment( EGUI_ALIGNMENT alignment )
{
	VerticalAlignment = alignment;

	recalculateScrollButtonPlacement();
	recalculateScrollBar();

	core::rect<s32> r(calcTabPos());
	for ( u32 i=0; i<Tabs.size(); ++i )
	{
		Tabs[i]->setRelativePosition(r);
	}
}

void CGUIImageTabControl::recalculateScrollButtonPlacement()
{
	IGUISkin* skin = Environment->getSkin();
	s32 ButtonSize = 16;
	s32 ButtonHeight = TabHeight - 2;
	if ( ButtonHeight < 0 )
		ButtonHeight = TabHeight;
	if (skin)
	{
		ButtonSize = skin->getSize(EGDS_WINDOW_BUTTON_WIDTH);
		if (ButtonSize > TabHeight)
			ButtonSize = TabHeight;
	}

	s32 ButtonX = RelativeRect.getWidth() - (s32)(2.5f*(f32)ButtonSize) - 1;
	s32 ButtonY = 0;

	if (VerticalAlignment == EGUIA_UPPERLEFT)
	{
		ButtonY = 2 + (TabHeight / 2) - (ButtonHeight / 2);
		UpButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
		DownButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_UPPERLEFT, EGUIA_UPPERLEFT);
	}
	else
	{
		ButtonY = RelativeRect.getHeight() - (TabHeight / 2) - (ButtonHeight / 2) - 2;
		UpButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT);
		DownButton->setAlignment(EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT);
	}

	UpButton->setRelativePosition(core::rect<s32>(ButtonX, ButtonY, ButtonX+ButtonSize, ButtonY+ButtonHeight));
	ButtonX += ButtonSize + 1;
	DownButton->setRelativePosition(core::rect<s32>(ButtonX, ButtonY, ButtonX+ButtonSize, ButtonY+ButtonHeight));
}

//! Get the alignment of the tabs
EGUI_ALIGNMENT CGUIImageTabControl::getTabVerticalAlignment() const
{
	return VerticalAlignment;
}


s32 CGUIImageTabControl::getTabAt(s32 xpos, s32 ypos) const
{
	core::position2di p(xpos, ypos);
	IGUISkin* skin = Environment->getSkin();
	IGUIFont* font = skin->getFont();

	core::rect<s32> frameRect(AbsoluteRect);

	if ( VerticalAlignment == EGUIA_UPPERLEFT )
	{
		frameRect.UpperLeftCorner.Y += 2;
		frameRect.LowerRightCorner.Y = frameRect.UpperLeftCorner.Y + TabHeight;
	}
	else
	{
		frameRect.UpperLeftCorner.Y = frameRect.LowerRightCorner.Y - TabHeight;
	}

	s32 pos = frameRect.UpperLeftCorner.X + 2;

	if (!frameRect.isPointInside(p))
		return -1;

	for (s32 i=CurrentScrollTabIndex; i<(s32)Tabs.size(); ++i)
	{
		// get Text
		const wchar_t* text = 0;
		if (Tabs[i])
			text = Tabs[i]->getText();

		// get text length
		s32 len = calcTabWidth(pos, font, text, true, Tabs[i]);
		if ( ScrollControl && pos+len > UpButton->getAbsolutePosition().UpperLeftCorner.X - 2 )
			return -1;

		frameRect.UpperLeftCorner.X = pos;
		frameRect.LowerRightCorner.X = frameRect.UpperLeftCorner.X + len;

		pos += len;

		if (frameRect.isPointInside(p))
		{
			return i;
		}
	}
	return -1;
}

//! Returns which tab is currently active
s32 CGUIImageTabControl::getActiveTab() const
{
	return ActiveTab;
}


//! Brings a tab to front.
bool CGUIImageTabControl::setActiveTab(s32 idx)
{
	if ((u32)idx >= Tabs.size())
		return false;

	bool changed = (ActiveTab != idx);

	ActiveTab = idx;

	for (s32 i=0; i<(s32)Tabs.size(); ++i)
		if (Tabs[i])
			Tabs[i]->setVisible( i == ActiveTab );

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

	recalculateScrollBar();
}


//! Update the position of the element, decides scroll button status
void CGUIImageTabControl::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
	recalculateScrollBar();
}


//! Writes attributes of the element.
void CGUIImageTabControl::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUITabControl::serializeAttributes(out,options);

	out->addInt ("ActiveTab",	ActiveTab);
	out->addBool("Border",		Border);
	out->addBool("FillBackground",	FillBackground);
	out->addInt ("TabHeight",	TabHeight);
	out->addInt ("TabMaxWidth", TabMaxWidth);
	out->addEnum("TabVerticalAlignment", s32(VerticalAlignment), GUIAlignmentNames);
}


//! Reads attributes of the element
void CGUIImageTabControl::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	Border          = in->getAttributeAsBool("Border");
	FillBackground  = in->getAttributeAsBool("FillBackground");

	ActiveTab = -1;

	setTabHeight(in->getAttributeAsInt("TabHeight"));
	TabMaxWidth     = in->getAttributeAsInt("TabMaxWidth");

	IGUITabControl::deserializeAttributes(in,options);

	setActiveTab(in->getAttributeAsInt("ActiveTab"));
	setTabVerticalAlignment( static_cast<EGUI_ALIGNMENT>(in->getAttributeAsEnumeration("TabVerticalAlignment" , GUIAlignmentNames)) );
}
} // end namespace irr
} // end namespace gui



















