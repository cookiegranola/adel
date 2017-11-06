#ifndef __C_GUI_IMAGE_TAB_CONTROL_H_INCLUDED__
#define __C_GUI_IMAGE_TAB_CONTROL_H_INCLUDED__

#include "IrrCompileConfig.h"
#include "IGUITabControl.h"
#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IGUIFont.h"
#include "IVideoDriver.h"
#include "irrArray.h"
#include "rect.h"

namespace irr
{
namespace gui
{
	class CGUIImageTabControl;
	class IGUIButton;

	// A tab, onto which other gui elements could be added.
	class CGUIImageTab : public IGUITab
	{
	public:

		//! constructor
		CGUIImageTab(s32 number, IGUIEnvironment* environment,
			IGUIElement* parent, const core::rect<s32>& rectangle,
			s32 id, 
			video::ITexture *texture=0, f32 scaling=1.0f, s32 side=0);

		//! destructor
		//virtual ~CGUIImageTab();

		//! Returns number of this tab in tabcontrol. Can be accessed
		//! later IGUITabControl::getTab() by this number.
		virtual s32 getNumber() const;

		//! Sets the number
		virtual void setNumber(s32 n);

		//! draws the element and its children
		virtual void draw();

		//! sets if the tab should draw its background
		virtual void setDrawBackground(bool draw=true);

		//! sets the color of the background, if it should be drawn.
		virtual void setBackgroundColor(video::SColor c);

		//! sets the color of the text
		virtual void setTextColor(video::SColor c);

		//! returns true if the tab is drawing its background, false if not
		virtual bool isDrawingBackground() const;

		//! returns the color of the background
		virtual video::SColor getBackgroundColor() const;

		virtual video::SColor getTextColor() const;

		//! Writes attributes of the element.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;

		//! Reads attributes of the element
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

		//! only for internal use by CGUIImageTabControl
		void refreshSkinColors();
		
		void drawImage(const irr::core::rect<s32>& frameRect);
		
		s32 Number;
		video::SColor BackColor;
		bool OverrideTextColorEnabled;
		video::SColor TextColor;
		bool DrawBackground;
		video::ITexture *Texture;
		f32 Scaling;
		s32 Side;
		bool Active;
		bool Drawn;
		core::rect<s32> DrawnRect;
	};


	//! A standard tab control
	class CGUIImageTabControl : public IGUITabControl
	{
	public:

		//! destructor
		CGUIImageTabControl(IGUIEnvironment* environment,
			IGUIElement* parent, const core::rect<s32>& rectangle,
			bool fillbackground=true, bool border=true, s32 id=-1, 
			s32 tab_height=0, 
			s32 side=0, s32 view_width=0, s32 view_height=0);

		//! destructor
		virtual ~CGUIImageTabControl();

		//! Adds a tab
		virtual IGUITab * addTab(const wchar_t* caption, s32 id=-1);
		
		//! Adds an image tab
		virtual CGUIImageTab* addImageTab(const wchar_t* caption, s32 id=-1, 
			video::ITexture* texture=0, f32 scaling=1.0f);

		//! Adds a tab that has already been created
		virtual void addTab(CGUIImageTab* tab);

		//! Insert the tab at the given index
		virtual IGUITab* insertTab(s32 idx, const wchar_t* caption, s32 id=-1);

		//! Removes a tab from the tabcontrol
		virtual void removeTab(s32 idx);

		//! Clears the tabcontrol removing all tabs
		virtual void clear();

		//! Returns amount of tabs in the tabcontrol
		virtual s32 getTabCount() const;

		//! Returns a tab based on zero based index
		virtual IGUITab* getTab(s32 idx) const;

		//! Brings a tab to front.
		virtual bool setActiveTab(s32 idx);

		//! Brings a tab to front.
		virtual bool setActiveTab(IGUITab *tab);

		//! Returns which tab is currently active
		virtual s32 getActiveTab() const;

		//! get the the id of the tab at the given absolute coordinates
		virtual s32 getTabAt(s32 xpos, s32 ypos) const;

		//! called if an event happened.
		virtual bool OnEvent(const SEvent& event);

		//! place the visible tabs
		virtual void computeTabs();
		
		//! draws the element and its children
		virtual void draw();

		//! Removes a child.
		virtual void removeChild(IGUIElement* child);

		//! Writes attributes of the element.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const;
		//! Set the height of the tabs
		virtual void setTabHeight( s32 height );

		//! Reads attributes of the element
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

		//! Get the height of the tabs
		virtual s32 getTabHeight() const;

		//! set the maximal width of a tab. Per default width is 0 which means "no width restriction".
		virtual void setTabMaxWidth(s32 width );

		//! get the maximal width of a tab
		virtual s32 getTabMaxWidth() const;

		//! Set the alignment of the tabs
		//! note: EGUIA_CENTER is not an option
		virtual void setTabVerticalAlignment( gui::EGUI_ALIGNMENT alignment );

		//! Get the alignment of the tabs
		virtual gui::EGUI_ALIGNMENT getTabVerticalAlignment() const;

		//! Set the extra width added to tabs on each side of the text
		virtual void setTabExtraWidth( s32 extraWidth );

		//! Get the extra width added to tabs on each side of the text
		virtual s32 getTabExtraWidth() const;

		//! Update the position of the element, decides scroll button status
		virtual void updateAbsolutePosition();

	private:

		void scrollLeft();
		void scrollRight();
		bool needScrollControl( s32 startIndex=0, bool withScrollControl=false );
		s32 calcTabWidth(s32 pos, IGUIFont* font, const wchar_t* text, bool withScrollControl,
			CGUIImageTab* tab) const;
		core::rect<s32> calcTabPos();

		void recalculateScrollButtonPlacement();
		void recalculateScrollBar();
		void refreshSprites();

		core::array<CGUIImageTab*> Tabs;	// CGUIImageTab* because we need setNumber (which is certainly not nice)
		s32 ActiveTab;
		bool NeedLeftScroll;
		bool NeedRightScroll;
		bool Border;
		bool FillBackground;
		bool ScrollControl;
		s32 TabHeight;
		gui::EGUI_ALIGNMENT VerticalAlignment;
		IGUIButton* UpButton;
		IGUIButton* DownButton;
		s32 TabMaxWidth;
		s32 CurrentScrollTabIndex;
		s32 TabExtraWidth;
		s32 Side;
		s32 ViewWidth;
		s32 ViewHeight;
		core::rect<s32> ViewRect;
	};
} // end namespace gui
} // end namespace irr
#endif
