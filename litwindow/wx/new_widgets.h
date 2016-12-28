/* 
 * Copyright 2004-2006, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
 * This file is part of the Lit Window Library. All use of this material - copying
 * in full or part, including in other works, using in non-profit or for-profit work
 * and other uses - is governed by the licence contained in the Lit Window Library 
 * distribution, file LICENCE.TXT
 * $Id: new_widgets.h,v 1.3 2007/11/08 16:21:27 Hajo Kirchhoff Exp $
 */
#ifndef _LWWX_NEW_WIDGETS_H
#define _LWWX_NEW_WIDGETS_H

#pragma once

#include <wx/panel.h>
#include "litwindow/wx/lwwx.h"
#include "wx/xrc/xmlres.h"

class wxWindow;
class wxTreeCtrl;

namespace litwindow {

	namespace wx {
		/// lwEVT_COMMAND_FIND_TEXT is sent when the user data changes and an observer needs to lookup the proper text for the user data
		DECLARE_EXPORTED_EVENT_TYPE(LWWX_API, lwEVT_COMMAND_AUTOCOMPLETE_FIND_TEXT_FOR_DATA, 0);
		/// lwEVT_COMMAND_AUTOCOMPLETE_TEXT is sent when the text changes and an observer needs to lookup the full text and user data for a partial text
		DECLARE_EXPORTED_EVENT_TYPE(LWWX_API, lwEVT_COMMAND_AUTOCOMPLETE_COMPLETE_TEXT, 0);
		/// lwEVT_COMMAND_AUTOCOMPLETE_FILL_DROP_DOWN is sent whenever the control wants to show the drop down with the available choices. Gives an observer the chance to fill the drop down widget
		DECLARE_EXPORTED_EVENT_TYPE(LWWX_API, lwEVT_COMMAND_AUTOCOMPLETE_FILL_DROP_DOWN, 0);

		/// lwEVT_COMMAND_DROP_DOWN_DIALOG is sent when the drop down dialog ends
		DECLARE_EXPORTED_EVENT_TYPE(LWWX_API, lwEVT_COMMAND_DROP_DOWN_DIALOG, 0);

#pragma region Utility functions
		inline wxTopLevelWindow *GetOwningTopLevelWindow(wxWindow *p)
		{
			wxTopLevelWindow *w;
			do {
				w=wxDynamicCast(p, wxTopLevelWindow);
				if (w)
					return w;
				p=p->GetParent();
			} while (p);
			return 0;
		}
		inline bool IsParentOf(wxWindow *parent, wxWindow *child)
		{
			while (child) {
				child=child->GetParent();
				if (parent==child)
					return true;
			}
			return false;
		}
		inline bool IsChildOf(wxWindow *child, wxWindow *parent)
		{
			bool rc=IsParentOf(parent, child);
			return rc;
		}
		//void for_each_children(wxWindow *child, boost::function<void(wxWindow*)> fnc)
		//{
		//	fnc(child);
		//	wxWindowList &children(child->GetChildren());
		//	for (wxWindowList::iterator i=children.begin(); i!=children.end(); ++i)
		//		for_each_children(*i, fnc);
		//}

#pragma endregion
		/// helper class to allow resizing of a widget used as a 'popup' or 'drowdown' widget by autocomplete controls
		class LWWX_API dropdown_dialog:public wxDialog
		{
			typedef wxDialog Inherited;
		public:
			dropdown_dialog();
			dropdown_dialog(wxWindow *parent, wxWindowID winid,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL | wxNO_BORDER,
				const wxString& name = wxPanelNameStr);

			bool Create(wxWindow *parent, wxWindowID winid,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = wxTAB_TRAVERSAL | wxNO_BORDER,
				const wxString& name = wxPanelNameStr);

			~dropdown_dialog();

			void save_size();
			void SetChild(wxWindow *child);
			wxWindow *GetChild() const { return m_child; }
			/// size and show panel as a drop down
			void DropDownFrom(wxWindow *from);
			/// close the dropdown_dialog drop down - will only hide the panel !! and disconnect any handlers to external windows
			void EndDropDownDialog(int result);

		protected:
			void EndDialog(int rc) { return EndDropDownDialog(rc); }
			void OnKeyDown(wxKeyEvent &evt);
			void OnPaint(wxPaintEvent &evt);
			void OnSetFocus(wxFocusEvent &evt);
			void OnSize(wxSizeEvent &evt);
			void OnMouseMove(wxMouseEvent &evt);
			void OnLeftDown(wxMouseEvent &evt);
			void OnLeftUp(wxMouseEvent &evt);
			void OnOKCancel(wxCommandEvent &evt);

			void fit_child_inside(wxSize newSize=wxDefaultSize);

			void Init();
			wxWindow *m_child;
			wxWindow *m_focused_before_drop_down;
			static const size_t sizing_handle_height = 19;
			bool	m_is_sizing,							///< true while user resizes the panel
						m_show_sizing_cursor;	///< true while sizing cursor is shown (user hovers above grip or is sizing the panel)
			wxWindow *m_dropdown_widget;	///< widget this control has been dropped down from
		};

		class LWWX_API dropdown_control:public wxPanel
		{
		public:
			dropdown_control() {Init();}
			dropdown_control(wxWindow *parent, wxWindowID id=wxID_ANY, const wxPoint &position=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0, const wxString &name=wxEmptyString);
			void Create(wxWindow *parent, wxWindowID id=wxID_ANY, const wxPoint &position=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0, const wxString &name=wxEmptyString);
			long GetValue() const { return m_user_data; }
			void SetValue(long new_user_data);
			wxString GetText() const { return m_text->GetValue(); }
			void SetText(const wxString &new_text);
			typedef wxPanel Inherited;
		protected:
			void Init();
			wxTextCtrl *m_text;
			wxBitmapButton *m_button;
			dropdown_dialog *m_dropdown;
			bool m_disable_autocomplete;	///< if true, do not autocomplete text on OnTextUpdate events. This happens when the user presses DEL, BACKSPACE etc...
			int m_freeze_text_update;			///< if >0 then ignore OnTextUpdate events -> these events originate from our own SetText.
			int m_setting_data;						///< if >0 then a SetValue or SetText is in progress.
			wxString m_last_text_value;		///< contains the uncompleted, users original text. This is m_text->GetValue() minus any autocompletion string
			long m_user_data;							///< user data associated with the current text

			void OnTextUpdated(wxCommandEvent &evt);
			void OnTextChar(wxKeyEvent &evt);
			void OnButtonClicked(wxCommandEvent &evt);
			void OnCommandDropDownDialog(wxCommandEvent &evt);

			/// try to complete 'text', show autocomplete widget if desired
			void do_auto_complete_text(const wxString &text, bool show_widget);
			/// show the drop down widget
			void show_drop_down_widget(bool do_show=true);
			/// refresh the contents of the drop down widget. This may involve sending an event to an observer and letting the observer fill the widget
			void refresh_drop_down_widget();
			/// hide the auto complete widget
			void hide_drop_down_widget() { show_drop_down_widget(false); }

			/// set the text in the text control and the user data
			void do_set_text(const wxString &new_text, long user_data);

			/// lookup user data in store for 'text'. Ask observer to call SetValue if they have user data for 'text'.
			/// if 'partial_match'==true, then a partial match is acceptable
			virtual void find_user_data_for(const wxString &text, bool partial_match=false);
			/// lookup text for user data. Ask observer to call SetText if text exists for this user data.
			virtual void find_text_for(long user_data);
			/// create the window to be shown in the drop down dialog
			virtual wxWindow *create_drop_down_window(wxWindow *parent) = 0;
			/// get the currently selected value
			virtual long get_drop_down_value() const = 0;
			/// get the currently selected text
			virtual wxString get_drop_down_text() const = 0;

			DECLARE_ABSTRACT_CLASS(dropdown_control);
		};

	};
};

#endif
