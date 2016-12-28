/* 
* Copyright 2004, Hajo Kirchhoff - Lit Window Productions, http://www.litwindow.com
* This file is part of the Lit Window Library. All use of this material - copying
* in full or part, including in other works, using in non-profit or for-profit work
* and other uses - is governed by the licence contained in the Lit Window Library 
* distribution, file LICENCE.TXT
* $Id: new_widgets.cpp,v 1.6 2007/11/08 16:21:28 Hajo Kirchhoff Exp $
*/
#include "stdwx.h"
using namespace std;
#include <wx/event.h>
#include "litwindow/lwbase.hpp"
#include "litwindow/dataadapter.h"
#include "litwindow/wx/rapidUI.h"
#include "litwindow/wx/new_widgets.h"
#include <boost/bind.hpp>
#include <wx/renderer.h>
#include <wx/treectrl.h>
#include <wx/confbase.h>

#define new DEBUG_NEW

namespace litwindow {

	namespace wx {

#pragma region dropdown_dialog implementation
		DEFINE_EVENT_TYPE(lwEVT_COMMAND_DROP_DOWN_DIALOG);
		DEFINE_EVENT_TYPE(lwEVT_COMMAND_AUTOCOMPLETE_FIND_TEXT_FOR_DATA);
		DEFINE_EVENT_TYPE(lwEVT_COMMAND_AUTOCOMPLETE_COMPLETE_TEXT);
		DEFINE_EVENT_TYPE(lwEVT_COMMAND_AUTOCOMPLETE_FILL_DROP_DOWN);

		dropdown_dialog::dropdown_dialog()
		{
			Init();
		}

		dropdown_dialog::dropdown_dialog(wxWindow *parent, wxWindowID winid,
			const wxPoint& pos,
			const wxSize& size,
			long style,
			const wxString& name)
		{
			Init();
			Create(parent, winid, pos, size, style, name);
		}

		dropdown_dialog::~dropdown_dialog()
		{
			save_size();
			//wxApp::GetInstance()->Disconnect(wxEVT_SET_FOCUS, wxFocusEventHandler(dropdown_dialog::OnSetFocus), NULL, this);
			wxApp::GetInstance()->Disconnect(wxEVT_KILL_FOCUS, wxFocusEventHandler(dropdown_dialog::OnSetFocus), NULL, this);
		}

		void dropdown_dialog::Init()
		{
			m_dropdown_widget=0;
			m_child=0;
			m_is_sizing=m_show_sizing_cursor=false;
		}

		bool dropdown_dialog::Create(wxWindow *parent, wxWindowID winid,
			const wxPoint& pos,
			const wxSize& size,
			long style,
			const wxString& name)
		{
			bool rc=Inherited::Create(GetOwningTopLevelWindow(parent), winid, wxEmptyString, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name);
			Connect(wxEVT_MOTION, wxMouseEventHandler(dropdown_dialog::OnMouseMove));
			Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(dropdown_dialog::OnLeftDown));
			Connect(wxEVT_LEFT_UP, wxMouseEventHandler(dropdown_dialog::OnLeftUp));
			Connect(wxEVT_PAINT, wxPaintEventHandler(dropdown_dialog::OnPaint));
			Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(dropdown_dialog::OnOKCancel));
			Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(dropdown_dialog::OnOKCancel));
			wxApp::GetInstance()->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(dropdown_dialog::OnSetFocus), NULL, this);
			wxAcceleratorEntry entries[]={
				wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F4, wxID_OK)
			};
			wxAcceleratorTable table(WXSIZEOF(entries), entries);
			SetAcceleratorTable(table);
			return rc;
		}
		void dropdown_dialog::DropDownFrom(wxWindow *from)
		{
			m_focused_before_drop_down=FindFocus();
			m_dropdown_widget=from;
			wxString parent_name(m_dropdown_widget->GetName());
			int width=wxConfigBase::Get()->Read(wxString::Format(_T("/dropdown_dialog::DropDownFrom/%s/width"), parent_name.c_str()), 150);
			int height=wxConfigBase::Get()->Read(wxString::Format(_T("/dropdown_dialog::DropDownFrom/%s/height"), parent_name.c_str()), 150);
			wxSize sz(from->GetSize());
			wxPoint ps=from->ClientToScreen(wxPoint(0, 0));
			wxRect startposition(ps.x, ps.y+sz.y-1, max(sz.x, width), height);
			wxSize displaySize=wxGetDisplaySize();
			if (startposition.GetBottom()>displaySize.GetHeight())
				startposition.Offset(0, ps.y-startposition.GetBottom());
			if (startposition.GetRight()>displaySize.GetWidth())
				startposition.Offset(displaySize.GetWidth()-startposition.GetRight(), 0);
			SetSize(startposition);
			Show();
			if (m_focused_before_drop_down)
				m_focused_before_drop_down->SetFocus();
		}

		void dropdown_dialog::OnPaint(wxPaintEvent &evt)
		{
			wxPaintDC dc(this);
			wxSize sz(GetClientSize());

			/* clear area */
			dc.SetBackground(GetBackgroundColour());
			dc.SetBackgroundMode(wxSOLID);
			dc.Clear();

			dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW));
			/* paint border */
			dc.DrawLine(0, 0, 0, sz.y-1);
			dc.DrawLine(0, sz.y-1, sz.x-1, sz.y-1);
			/* paint grip */
			//dc.SetBrush(*wxBLACK_BRUSH)
			for (int i=sizing_handle_height+1; i>0; i-=3) {
				dc.DrawLine(sz.x-i, sz.y, sz.x, sz.y-i);
			}
		}
		void dropdown_dialog::OnSetFocus(wxFocusEvent &evt)
		{
			evt.Skip();
			if (m_dropdown_widget && evt.GetEventType()==wxEVT_KILL_FOCUS) {
				wxWindow *newly_focused_window=evt.GetWindow();
				if (newly_focused_window==0) {
					newly_focused_window=FindFocus();
				}
				if (newly_focused_window
					&& newly_focused_window!=this 
					&& IsChildOf(newly_focused_window, this)==false 
					&& newly_focused_window!=m_dropdown_widget 
					&& IsChildOf(newly_focused_window, m_dropdown_widget)==false)
				{
					// the window that is going to receive the focus is not our child, close the dialog

					// first reset 'focused_before_drop_down'. the focus is going to a different window.
					m_focused_before_drop_down=0;
					EndDropDownDialog(wxID_CANCEL);
				}
			}
		}
		void dropdown_dialog::OnMouseMove(wxMouseEvent &evt)
		{
			if (m_is_sizing) {
				if (evt.LeftIsDown()) {
					// might be a bug: evt.GetPosition() returns very strange coordinates after a SetSize. Thus, use wxGetMousePosition instead
					wxPoint mouse_at(wxGetMousePosition());
					wxPoint new_size=mouse_at-ClientToScreen(wxPoint(0,0));
					if (new_size.x<50) new_size.x=50;
					if (new_size.y<25) new_size.y=25;
					SetClientSize(wxRect(ClientToScreen(wxPoint(0,0)), wxSize(new_size.x, new_size.y)));
				} else {
					ReleaseCapture();
					m_is_sizing=false;
				}
			} else {
				wxPoint lower_right(wxPoint(-(int)sizing_handle_height,-(int)sizing_handle_height)+GetSize());
				//lower_right=ClientToScreen(lower_right);
				wxRect grip_rect(lower_right, wxSize(sizing_handle_height, sizing_handle_height));
				bool is_sizing=grip_rect.Contains(evt.GetPosition());
				if (is_sizing!=m_show_sizing_cursor) {
					m_show_sizing_cursor=is_sizing;
					SetCursor(is_sizing ? wxCursor(wxCURSOR_SIZENWSE) : wxNullCursor);
				}
			}
		}
		void dropdown_dialog::OnLeftDown(wxMouseEvent &evt)
		{
			if (m_show_sizing_cursor) {
				if (m_is_sizing==false) {
					m_is_sizing=true;
					CaptureMouse();
				}
			}
		}
		void dropdown_dialog::OnLeftUp(wxMouseEvent &evt)
		{
			if (m_is_sizing) {
				m_is_sizing=false;
				ReleaseMouse();
			} else {
				//wxCommandEvent cmd(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());
				//cmd.SetInt(GetSelection());
				//GetParent()->AddPendingEvent(cmd);
			}
		}
		void dropdown_dialog::SetChild(wxWindow *child)
		{
			if (child) {
				wxSizer *sz=GetSizer();
				if (sz==0) {
					SetSizer(sz=new wxBoxSizer(wxVERTICAL));
					wxSizer *buttonsizer=new wxBoxSizer(wxHORIZONTAL);
					wxButton *btn;
					buttonsizer->Add(btn=new wxButton(this, wxID_OK, _("&OK"), wxDefaultPosition, wxSize(-1, sizing_handle_height), wxBU_EXACTFIT|wxNO_BORDER), 0, wxRIGHT, 5);
					btn->SetDefault();
					btn->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
					buttonsizer->Add(btn=new wxButton(this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxSize(-1, sizing_handle_height), wxBU_EXACTFIT|wxNO_BORDER));
					btn->SetFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
					sz->Add(buttonsizer, 0, wxALIGN_CENTER_HORIZONTAL);
				}
				m_child=child;
				if (child->GetParent()!=this)
					child->Reparent(this);
				sz->Prepend(child, 1, wxEXPAND);
				//SetMinSize(child->GetMinSize()+wxSize(0, sizing_handle_height));
				//child->SetSize(GetSize()-wxSize(0, sizing_handle_height));
			}
		}
		void dropdown_dialog::OnOKCancel(wxCommandEvent &evt)
		{
			EndDropDownDialog(evt.GetId());
		}
		void dropdown_dialog::EndDropDownDialog(int result)
		{
			save_size();
			if (m_dropdown_widget) {
				m_dropdown_widget=0;
			}
			wxCommandEvent evt(lwEVT_COMMAND_DROP_DOWN_DIALOG, GetId());
			evt.SetInt(result);
			evt.SetEventObject(this);
			AddPendingEvent(evt);
			Hide();
			if (m_focused_before_drop_down)
				m_focused_before_drop_down->SetFocus();
		}

		void dropdown_dialog::save_size()
		{
			if (m_dropdown_widget) {
				wxString parent_name(m_dropdown_widget->GetName());
				wxSize sz(GetSize());
				wxConfigBase::Get()->Write(wxString::Format(_T("/dropdown_dialog::DropDownFrom/%s/width"), parent_name.c_str()), sz.GetWidth());
				wxConfigBase::Get()->Write(wxString::Format(_T("/dropdown_dialog::DropDownFrom/%s/height"), parent_name.c_str()), sz.GetHeight());
			}
		}
#pragma endregion

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
		dropdown_control::dropdown_control(wxWindow *parent, wxWindowID id, const wxPoint &position, const wxSize &size, long style, const wxString &name)
		{
			Init();
			Inherited::Create(parent, id, position, size, style, name);
		}
		void dropdown_control::Create(wxWindow *parent, wxWindowID id, const wxPoint &position, const wxSize &size, long style, const wxString &name)
		{
			Inherited::Create(parent, id, position, size, style|wxTAB_TRAVERSAL, name);

			wxBoxSizer *sz=new wxBoxSizer(wxHORIZONTAL);
			m_text=new wxTextCtrl(this, 1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, name+wxT("_ac_text"));
			sz->Add(m_text, 1, wxLEFT|wxTOP, 0);
			static wxBitmap dropdown;
			static wxBitmap dropdown_pressed;
			static wxBitmap dropdown_focused;

			if (dropdown.Ok()==false) {
				wxMemoryDC mdc;
				wxSize sz=m_text->GetSize();
				sz.x=sz.y*2/3;
				dropdown=wxBitmap(sz.x, sz.y);
				mdc.SelectObject(dropdown);
				mdc.SetBackground(m_text->GetBackgroundColour());
				mdc.Clear();
				mdc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
				mdc.DrawRectangle(-1, 0, sz.x+1, sz.y);
				wxRendererNative::Get().DrawComboBoxDropButton(this, mdc, wxRect(wxPoint(0,1), sz-wxSize(1,2)));
				dropdown_pressed=wxBitmap(sz.x, sz.y);
				mdc.SelectObject(dropdown_pressed);
				mdc.SetBackground(m_text->GetBackgroundColour());
				mdc.Clear();
				mdc.DrawRectangle(-1, 0, sz.x+1, sz.y);
				wxRendererNative::Get().DrawComboBoxDropButton(this, mdc, wxRect(wxPoint(0,1), sz-wxSize(1,2)), wxCONTROL_PRESSED);
				dropdown_focused=wxBitmap(sz.x, sz.y);
				mdc.SelectObject(dropdown_focused);
				mdc.SetBackground(m_text->GetBackgroundColour());
				mdc.Clear();
				mdc.DrawRectangle(-1, 0, sz.x+1, sz.y);
				wxRendererNative::Get().DrawComboBoxDropButton(this, mdc, wxRect(wxPoint(0, 1), sz-wxSize(1,2)), wxCONTROL_FOCUSED);
				mdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW), 1, wxFDIAGONAL_HATCH));
				mdc.SetBrush(*wxTRANSPARENT_BRUSH);
				mdc.DrawRectangle(0, 0, sz.x-1, sz.y-1);
			}
			m_button=new wxBitmapButton(this, 2, dropdown, wxDefaultPosition, wxDefaultSize, 0);
			m_button->SetBitmapSelected(dropdown_pressed);
			m_button->SetBitmapFocus(dropdown_focused);
			sz->Add(m_button, 0, wxTOP|wxBOTTOM, 0);

			SetMinSize(size);
			SetSizer(sz);

			m_text->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(dropdown_control::OnTextUpdated), 0, this);
			m_text->Connect(wxEVT_CHAR, wxCharEventHandler(dropdown_control::OnTextChar), 0, this);
			m_button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(dropdown_control::OnButtonClicked), 0, this);
		}

		void dropdown_control::Init()
		{
			m_text=0;
			m_button=0;
			m_dropdown=0;
			m_setting_data=0;
			m_freeze_text_update=0;
			m_user_data=-1;
			m_disable_autocomplete=false;
		}
		void dropdown_control::show_drop_down_widget(bool do_show)
		{
			if (do_show) {
				if (m_dropdown==0) {
					m_dropdown=new dropdown_dialog(this, wxID_ANY);
					m_dropdown->SetMinSize(wxSize(150, -1));
					m_dropdown->SetChild(create_drop_down_window(m_dropdown));
					m_dropdown->DropDownFrom(this);
					m_dropdown->Connect(lwEVT_COMMAND_DROP_DOWN_DIALOG, wxCommandEventHandler(dropdown_control::OnCommandDropDownDialog), NULL, this);
				}
				refresh_drop_down_widget();
			} else {
				if (m_dropdown) {
					m_dropdown->Destroy();
					m_dropdown=0;
				}
			}
		}
		void dropdown_control::refresh_drop_down_widget()
		{
			wxCommandEvent evt(litwindow::wx::lwEVT_COMMAND_AUTOCOMPLETE_FILL_DROP_DOWN, GetId());
			evt.SetEventObject(this);
			ProcessEvent(evt);
		}
		void dropdown_control::OnCommandDropDownDialog(wxCommandEvent &evt)
		{
			evt.Skip();
			if (evt.GetInt()==wxID_OK) {
				long current_value=get_drop_down_value();
				wxString current_text=get_drop_down_text();
				do_set_text(current_text, current_value);
			}
			hide_drop_down_widget();
		}
		void dropdown_control::OnTextUpdated(wxCommandEvent &evt)
		{
			evt.Skip();
			/* only expand the text if the user types characters. Do not expand the text if the user deletes characters or on internal update */
			if (m_freeze_text_update==0) {
				int length_delta=m_text->GetValue().length()-m_last_text_value.length();
				m_last_text_value=m_text->GetValue();
				do_auto_complete_text(m_last_text_value, false);
			}
		}
		void dropdown_control::OnTextChar(wxKeyEvent &evt)
		{
			evt.Skip();
			switch (evt.GetKeyCode()) 
			{
			case WXK_ESCAPE:
				if (m_dropdown) {
					hide_drop_down_widget();
					evt.Skip(false);	// consume event
				}
				break;
			case WXK_F4:
				if (m_dropdown) {
					hide_drop_down_widget();
				} else {
					do_auto_complete_text(m_text->GetValue(), true);
					if (m_dropdown)
						m_dropdown->SetFocus();
				}
				break;
			case WXK_TAB:
			case WXK_RETURN:
				{
					if (evt.ControlDown()==false && evt.AltDown()==false && evt.ShiftDown()==false && evt.MetaDown()==false) {
						// simulate a 'TAB' event with the button as the starting point
						// this will skip the button
						wxNavigationKeyEvent nav_evt;
						nav_evt.SetDirection(true);
						nav_evt.SetCurrentFocus(m_button);
						nav_evt.SetFromTab(true);
						AddPendingEvent(nav_evt);
						evt.Skip(false);
					}
				}
				break;
			case WXK_DOWN:
			case WXK_UP:
				if (m_dropdown==0) {
					do_auto_complete_text(m_text->GetValue(), true);
				}
				if (m_dropdown)
					m_dropdown->SetFocus();
				break;
			case WXK_BACK:
			case WXK_DELETE:
			case WXK_NUMPAD_DELETE:
				m_disable_autocomplete=true;
				hide_drop_down_widget();
				break;
			case 1:	// Ctrl+A
				{
					m_text->SetSelection(-1,-1);
					evt.Skip(false);
				} break;
			default:
				break;
			}
		}

		void dropdown_control::OnButtonClicked(wxCommandEvent &evt)
		{
			//do_auto_complete_text(wxEmptyString, true, true);
			show_drop_down_widget();
			//m_text->SetFocus();
			evt.Skip();
		}
		void dropdown_control::do_auto_complete_text(const wxString &text, bool show_widget)
		{
			++m_freeze_text_update;
			if (show_widget)
				show_drop_down_widget();
			m_last_text_value=text;
			long current_user_data(GetValue());
			++m_setting_data;
			// find_user_data_for may change 'Text' and 'Value'
			// but since m_setting_data>0 it will not trigger the update mechanism, only set 'Text' and 'Value'
			find_user_data_for(text, m_disable_autocomplete==false);
			--m_setting_data;
			if (GetText().length()>m_last_text_value.length()) {
				m_text->SetSelection(m_last_text_value.length(), -1);
			}
			// now restore m_user_data to its former value so that the update mechanism will be triggered if it is different from its former value
			swap(m_user_data, current_user_data);
			do_set_text(GetText(), current_user_data);
			--m_freeze_text_update;
			m_disable_autocomplete=false;
		}
		void dropdown_control::do_set_text(const wxString &text, long new_user_data)
		{
			++m_freeze_text_update;
			m_text->SetValue(text);
			--m_freeze_text_update;
			NotifyChanged(*this, "Text", false, new_user_data!=m_user_data);
			if (new_user_data!=m_user_data) {
				m_user_data=new_user_data;
				NotifyChanged(*this, "Value");
			}
			wxLogDebug(_T("**__%s %d"), text.c_str(), new_user_data);
		}
		void dropdown_control::find_user_data_for(const wxString &text, bool partial_match)
		{
			wxCommandEvent evt(lwEVT_COMMAND_AUTOCOMPLETE_COMPLETE_TEXT, GetId());
			evt.SetString(text);
			evt.SetEventObject(this);
			evt.SetInt(partial_match ? 1 : 0);
			if (ProcessEvent(evt)==false)
				SetValue(-1);
		}
		void dropdown_control::find_text_for(long user_data)
		{
			wxCommandEvent evt(lwEVT_COMMAND_AUTOCOMPLETE_FIND_TEXT_FOR_DATA, GetId());
			evt.SetInt(user_data);
			evt.SetEventObject(this);
			if (ProcessEvent(evt)==false) {
				// do nothing. Could erase the text, but leave it alone as the user may have made a typo and may want to correct it
			}
		}
		/// Set the user data for this control.
		/// Setting values for this control has added complexity, because user data and text are coupled
		/// While setting the user data, text may change as well. To avoid feedback loops, m_setting_data guards
		/// set loops. During the 'Set' data gathering period, m_setting_data > 0.
		void dropdown_control::SetValue(long new_user_data)
		{
			if (new_user_data!=m_user_data) {
				if (m_setting_data>0) {
					// a SetValue/Text is already in progress. Simply set m_user_data and return.
					m_user_data=new_user_data;
				} else {
					++m_setting_data;
					find_text_for(new_user_data);
					do_set_text(GetText(), new_user_data);
					m_last_text_value=GetText();
					--m_setting_data;
				}
			}
		}
		void dropdown_control::SetText(const wxString &new_text)
		{
			if (new_text!=GetText()) {
				if (m_setting_data) {
					// a SetValue/Text is already in progress. Simply set the text and return.
					++m_freeze_text_update;
					m_text->SetValue(new_text);
					--m_freeze_text_update;
				} else {
					++m_setting_data;
					m_last_text_value=new_text;
					find_user_data_for(new_text);
					do_set_text(new_text, GetValue());
					--m_setting_data;
				}
			}
		}
		IMPLEMENT_ABSTRACT_CLASS(litwindow::wx::dropdown_control, dropdown_control::Inherited);
	};

};

LWL_BEGIN_AGGREGATE_ABSTRACT(litwindow::wx::dropdown_control)
PROP_GetSet(long, Value)
PROP_GetSet(wxString, Text)
LWL_END_AGGREGATE()