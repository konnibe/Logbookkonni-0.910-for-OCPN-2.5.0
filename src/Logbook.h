#ifndef _LOGBOOK_H_
#define _LOGBOOK_H_

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "../../../include/ocpn_plugin.h"
#include "LogbookHTML.h"
#include "nmea0183/nmea0183.h"

//#define PBVE_DEBUG 1

class Options;
class LogbookDialog;
class PBVEDialog;

class Logbook : public LogbookHTML
{
private:
	enum fields{ ROUTE,RDATE,RTIME,SIGN,WAKE,DISTANCE,DTOTAL,POSITION,COG,COW,SOG,SOW,DEPTH,REMARKS,
				 BARO,WIND,WSPD,CURRENT,CSPD,WAVE,SWELL,WEATHER,CLOUDS,VISIBILITY,
				 MOTOR,MOTORT,FUEL,FUELT,SAILS,REEF,WATER,WATERT,MREMARKS};	

	struct Pos{
		double posLat;
		double latitude;
		double latmin;
		char   NSflag;
		double posLon;
		double longitude;
		double lonmin;
		char   WEflag;
				}oldPosition,newPosition;

	NMEA0183       		m_NMEA0183;
	RMB					tempRMB;
	wxString			sLat;
	wxString			sLon;
	wxDateTime			mUTCDateTime;
	wxString			sDate;
	wxString			sTime;
	wxString		    sSOG;
	wxString			sSOW;
	wxString		    sCOG;
	wxString		    sCOW;
	wxString			sDistance;
	wxString		    sWind;
	wxString			sWindSpeed;
	wxString			sDepth;
	wxString			sLogText;
	wxString			lastWayPoint;
	Options				*opt;
	bool				noSentence;
	bool				gpsStatus;
	bool				bCOW;
	double				dCOW;
	double				dCOG;
	bool				mode;
	bool				courseChange;
	bool				everySM;
	bool				guardChange;
	bool				waypointArrived;
	bool				oldLogbook;

	wxString			toSDMM ( int NEflag, double a, bool mode );
	wxString			toSDMMOpenCPN ( int NEflag, double a, bool hi_precision );
	void				setPositionString(double lat,int north, double lon, int east);
	void				setDateTimeString(wxDateTime s);
	void				getModifiedCellValue(int grid, int row, int selcol, int col);
	wxString			computeCell(int grid,int row, int col, wxString s, bool mode);
	void				clearAllGrids();
	wxString			calculateDistance(wxString fromstr, wxString tostr);
	wxDouble			positionStringToDezimal(wxString pos);
	wxDouble			positionStringToDezimalModern(wxString pos);
	wxString			getWake();
	void				checkCourseChanged();
	void				checkGuardChanged();
	void				checkWayPoint(RMB rmb);
	void				checkDistance();
	wxString			positionTraditional(int NEflag, double a, bool mode );
	wxString			positionGPSLike(int NEflag, double a, bool mode );
	void				setOldPosition();

#ifdef PBVE_DEBUG
	int pbvecount;
#endif

public:
	LogbookDialog*	dialog;
	LogbookHTML*	logbookHTML;
	wxString		layout_locn;
	wxString		layoutODT;
	wxString		data_locn;
	bool			modified;
	wxDateTime		mCorrectedDateTime;
	long			dLastMinute;
	PBVEDialog*		pvbe;

public:
	Logbook(LogbookDialog* parent, wxString data, wxString layout, wxString layoutODT);
	~Logbook(void);

	void SetSentence(wxString &sentence);
	void SetPosition(PlugIn_Position_Fix &pfix);
	void loadData();
	void loadSelectedData(wxString path);
	void loadDatanew();
	void deleteRow(int row);
	void appendRow(bool mode);
	void update();
	void clearNMEAData();
	void newLogbook();
	void switchToActuellLogbook();
	void selectLogbook();
	void changeCellValue(int row, int col, int offset);
	void setLayoutLocation(wxString loc);
	void SetGPSStatus(bool status);
	bool checkGPS(bool appendClick);


	wxTextFile* logbookFile;
	wxString	title;

private:
	wxString	logbookData_actuell;
	bool		noAppend; // Old Logbook; append Rows not allowed
};


#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class NoAppendDialog
///////////////////////////////////////////////////////////////////////////////
class NoAppendDialog : public wxDialog 
{
	private:
	
	protected:
		wxStaticText* m_staticText73;
		wxStdDialogButtonSizer* m_sdbSizer5;
		wxButton* m_sdbSizer5OK;
	
	public:
		
		NoAppendDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Information"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 255,130 ), long style = wxDEFAULT_DIALOG_STYLE );
		~NoAppendDialog();
	
};
////////////////////////////
// PVBE Dialog
///////////////////////////
class PBVEDialog : public wxFrame 
{
	private:
		LogbookDialog* dialog;
	protected:

	
	public:
		virtual void PBVEDialogOnClose( wxCloseEvent& event );
		void OnCloseWindow(wxCloseEvent& ev);
		wxTextCtrl* m_textCtrlPVBE;		
		PBVEDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~PBVEDialog();
	
};
#endif