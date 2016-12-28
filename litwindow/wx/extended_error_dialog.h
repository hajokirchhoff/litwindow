/////////////////////////////////////////////////////////////////////////////
// Name:        extended_error_dialog.h
// Purpose:     
// Author:      Hajo Kirchhoff
// Modified by: 
// Created:     26/11/2008 10:24:51
// RCS-ID:      
// Copyright:   Copyright Hajo Kirchhoff IT-Consulting, Lit Window Productions
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _EXTENDED_ERROR_DIALOG_H_
#define _EXTENDED_ERROR_DIALOG_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/html/htmlwin.h"
#include "wx/collpane.h"
////@end includes
#include <wx/dialog.h>
#include "litwindow/wx/extended_error.h"
#include "litwindow/wx/lwwx.h"

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxHtmlWindow;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_EXTENDED_ERROR_DIALOG 10000
#define ID_MESSAGE 10001
#define ID_COLLAPSIBLEPANE 10002
#define ID_DETAIL 10006
#define SYMBOL_EXTENDED_ERROR_DIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_EXTENDED_ERROR_DIALOG_TITLE _("Error...")
#define SYMBOL_EXTENDED_ERROR_DIALOG_IDNAME ID_EXTENDED_ERROR_DIALOG
#define SYMBOL_EXTENDED_ERROR_DIALOG_SIZE wxSize(400, 300)
#define SYMBOL_EXTENDED_ERROR_DIALOG_POSITION wxDefaultPosition
////@end control identifiers

namespace litwindow { namespace wx {

/*!
 * extended_error_dialog class declaration
 */

class LWWX_API extended_error_dialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( extended_error_dialog )
    DECLARE_EVENT_TABLE()

	bool m_allow_retry;
public:
    /// Constructors
    extended_error_dialog();
    extended_error_dialog( wxWindow* parent, wxWindowID id = SYMBOL_EXTENDED_ERROR_DIALOG_IDNAME, const wxString& caption = SYMBOL_EXTENDED_ERROR_DIALOG_TITLE, const wxPoint& pos = SYMBOL_EXTENDED_ERROR_DIALOG_POSITION, const wxSize& size = SYMBOL_EXTENDED_ERROR_DIALOG_SIZE, long style = SYMBOL_EXTENDED_ERROR_DIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_EXTENDED_ERROR_DIALOG_IDNAME, const wxString& caption = SYMBOL_EXTENDED_ERROR_DIALOG_TITLE, const wxPoint& pos = SYMBOL_EXTENDED_ERROR_DIALOG_POSITION, const wxSize& size = SYMBOL_EXTENDED_ERROR_DIALOG_SIZE, long style = SYMBOL_EXTENDED_ERROR_DIALOG_STYLE );

    /// Destructor
    ~extended_error_dialog();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin extended_error_dialog event handler declarations

////@end extended_error_dialog event handler declarations

////@begin extended_error_dialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end extended_error_dialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin extended_error_dialog member variables
    wxBoxSizer* m_main_sizer;
    wxHtmlWindow* m_message;
    wxHtmlWindow* m_detail;
////@end extended_error_dialog member variables
	wxStdDialogButtonSizer* m_standard_buttons;
    void SetMessage(const wxString &message);
    void SetDetails(const wxString &details);

    static int show_error_dialog(const extended_error<char> &e, const wxString &context=wxEmptyString, bool can_retry=false);
    static int show_error_dialog(const wxString& msg, const wxString &detail, const wxString &context=wxEmptyString, bool can_retry=false);
    inline static int show(const wxString &msg, const wxString &detail=wxString(), const wxString &context=wxEmptyString, bool can_retry=false) { return show_error_dialog(msg, detail, context); }

    template <typename Exception>
    static int show(const Exception &e)
    {
        return show_error_dialog(make_extended_error<char>(e));
    }
	template <typename Exception>
	static int show_exception(const Exception &e, const wxString &context=wxEmptyString, bool can_retry=false)
	{
		return show_error_dialog(make_extended_error<char>(e), context, can_retry);
	}
    static wxString extended_error_dialog::MakeHtml(const wxString &input);
	template <typename Fnc>
	static int try_op(Fnc &fnc, const wxString &context=wxString(), bool can_retry=false)
	{
		int rc=can_retry ? wxYES : wxOK;
		try {
			fnc();
			rc=wxOK;
		}
		catch (std::exception &e) {
			rc=show_exception(e, context, can_retry);
		}
		catch (boost::exception &e) {
			rc=show_exception(e, context, can_retry);
		}
		catch (...) {
			rc=show(_("Unknown exception"), _("No further details are available."), context, can_retry);
		}
		return rc;
	}
	bool allow_retry() const { return m_allow_retry; }
	void allow_retry(bool yes) { m_allow_retry=yes; }
};

    // _EXTENDED_ERROR_DIALOG_H_

} /*namespace wx*/ } /*namespace litwindow*/

namespace wx1 = litwindow::wx;

#endif
