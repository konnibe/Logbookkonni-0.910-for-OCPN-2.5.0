#ifndef WX_PRECOMP
     # include <wx/wx.h>
#endif

#include "Maintenance.h"
#include "FastComboEditor.h"
#include "GridCellChoiceRenderer.h"
#include "LogbookDialog.h"
#include "logbook_pi.h"
#include "Logbook.h"
#include "Options.h"
#include "Export.h"
#include "FastComboEditor.h"

#include <wx/tokenzr.h>
#include <wx/filename.h> 
#include <wx/mimetype.h>
#include <wx/wfstream.h> 
#include <wx/txtstrm.h> 
#include <wx/zipstrm.h> 
#include <wx/generic/gridctrl.h>
#include <wx/datetime.h>


#include <memory>
using namespace std;

Maintenance::Maintenance(LogbookDialog* d, wxString data, wxString layout, wxString layoutODT)
	: Export(d)
{
	dialog = d;
	this->layout_locn = layout;
	this->ODTLayout = layoutODT;
	grid = d->m_gridMaintanence;
	repairs = d->m_gridMaintanenceRepairs;
	buyparts = d->m_gridMaintenanceBuyParts;
	selectedCol = 0;
	selectedRow = 0;
	modified = false;

	green = wxColour(0,255,0);
	red = wxColour(255,0,0);
	redlight = wxColour(255,128,128);
	yellow = wxColour(255,255,0);
	yellowlight = wxColour(255,255,155);
	white = wxColour(255,255,255);

	wxString serviceData = data;
	wxTextFile *serviceFile;
	serviceData.Append(_T("service.txt"));
	wxFileName wxHomeFiledir = serviceData ;
	if(!wxHomeFiledir.FileExists())
	{
		serviceFile = new wxTextFile(serviceData);	
		serviceFile->Create();
	}
	else
		serviceFile = new wxTextFile(serviceData);

	data_locn = serviceData;

	wxString buyPartsData = data;
	wxTextFile *buyPartsFile;
	buyPartsData.Append(_T("buyparts.txt"));
	wxHomeFiledir = buyPartsData ;
	if(!wxHomeFiledir.FileExists())
	{
		buyPartsFile = new wxTextFile(buyPartsData);	
		buyPartsFile->Create();
	}
	else
		buyPartsFile = new wxTextFile(buyPartsData);

	data_locnBuyParts = buyPartsData;

	wxString repairsData = data;
	wxTextFile *repairsFile;
	repairsData.Append(_T("repairs.txt"));
	wxHomeFiledir = repairsData ;
	if(!wxHomeFiledir.FileExists())
	{
		repairsFile = new wxTextFile(repairsData);	
		repairsFile->Create();
	}
	else
		repairsFile = new wxTextFile(repairsData);

	data_locnRepairs = repairsData;

	setLayoutLocation();

	m_choices[0] = dialog->m_gridGlobal->GetColLabelValue(6)+_T(" +"); // Distance/T
	m_choices[1] = dialog->m_gridMotorSails->GetColLabelValue(1)+_T(" +"); // Motor/h
	m_choices[2] = dialog->m_gridGlobal->GetColLabelValue(3); // Sign
	m_choices[3] = _("Fix Date");
	m_choices[4] = _("Date + Days");
	m_choices[5] = _("Date + Weeks");
	m_choices[6] = _("Date + Month");

	m_YesNo[0] = _("Yes"); 
	m_YesNo[1] = _("No"); 

	m_Priority[0] = _T("0"); 
	m_Priority[1] = _T("1"); 
	m_Priority[2] = _T("2"); 
	m_Priority[3] = _T("3"); 
	m_Priority[4] = _T("4"); 
	m_Priority[5] = _T("5"); 
}

Maintenance::~Maintenance(void)
{
	update();
	updateRepairs();
	updateBuyParts();
}

void Maintenance::setLayoutLocation()
{
	wxString layout_locn;

	if(dialog->m_radioBtnHTMLBuyParts->GetValue())
		layout_locnBuyParts = this->layout_locn;
	else
		layout_locnBuyParts = ODTLayout;

	wxString buypartsLay = layout_locnBuyParts;

	buypartsLay.Append(_T("buyparts"));
	dialog->appendOSDirSlash(&buypartsLay);
	layout_locnBuyParts = buypartsLay;
	dialog->loadLayoutChoice(buypartsLay,dialog->m_choiceSelectLayoutBuyParts);


	if(dialog->m_radioBtnHTMLService->GetValue())
		layout_locnService = this->layout_locn;
	else
		layout_locnService = ODTLayout;

	wxString serviceLay = layout_locnService;

	serviceLay.Append(_T("service"));
	dialog->appendOSDirSlash(&serviceLay);
	layout_locnService = serviceLay;
	dialog->loadLayoutChoice(serviceLay,dialog->m_choiceSelectLayoutService);

	if(dialog->m_radioBtnHTMLRepairs->GetValue())
		layout_locnRepairs = this->layout_locn;
	else
		layout_locnRepairs = ODTLayout;

	wxString repairsLay = layout_locnRepairs;

	repairsLay.Append(_T("repairs"));
	dialog->appendOSDirSlash(&repairsLay);
	layout_locnRepairs = repairsLay;
	dialog->loadLayoutChoice(repairsLay,dialog->m_choiceSelectLayoutRepairs);
}

void Maintenance::setAlignmentService()
{
	grid->SetReadOnly(lastRow,START);
	grid->SetReadOnly(lastRow,PRIORITY);
	grid->SetCellAlignment( lastRow,PRIORITY,wxALIGN_CENTER, wxALIGN_TOP );
	grid->SetCellAlignment( lastRow,TEXT,wxALIGN_LEFT, wxALIGN_TOP );
	grid->SetCellAlignment( lastRow,IF,wxALIGN_CENTER, wxALIGN_TOP );
	grid->SetCellAlignment( lastRow,ACTIVE,wxALIGN_CENTER, wxALIGN_TOP );
}

void Maintenance::addLine()
{
	modified = true;
	grid->AppendRows();

	lastRow = grid->GetNumberRows()-1;
	selectedRow = lastRow;
	setAlignmentService();

	grid->SetCellRenderer(lastRow, IF, new wxGridCellChoiceRenderer);
	grid->SetCellEditor(lastRow,IF,new wxFastComboEditor(7,m_choices,true));

	grid->SetCellRenderer(lastRow, ACTIVE, new wxGridCellChoiceRenderer);
	grid->SetCellEditor(lastRow,ACTIVE,new wxFastComboEditor(2,m_YesNo,true));

	grid->SetCellValue(lastRow,PRIORITY,_T("5"));
	grid->SetCellValue(lastRow,IF,m_choices[0]);
	grid->SetCellValue(lastRow,WARN,_T("1"));
	grid->SetCellValue(lastRow,URGENT,_T("2"));
	cellCollChanged(IF, lastRow);
	cellCollChanged(WARN, lastRow);
	checkService(dialog->m_gridGlobal->GetNumberRows()-1);
	grid->SetCellBackgroundColour(lastRow,START,wxColour( 240, 240, 240 ));

	grid->SetCellValue(lastRow,ACTIVE,_("Yes"));
}

void Maintenance::addLineRepairs()
{
	modified = true;
	repairs->AppendRows();

	lastRowRepairs = repairs->GetNumberRows()-1;
	selectedRowRepairs = lastRowRepairs;
	setAlignmentRepairs();

	repairs->SetCellValue(lastRowRepairs,RPRIORITY,_T("0"));
	checkRepairs();
}

void Maintenance::addLineBuyParts()
{
	modified = true;
	buyparts->AppendRows();

	lastRowBuyParts = buyparts->GetNumberRows()-1;
	selectedRowBuyParts = lastRowBuyParts;	
	setAlignmentBuyParts();

	buyparts->SetCellValue(lastRowBuyParts,PPRIORITY,_T("0"));
	checkBuyParts();
}

void Maintenance::setAlignmentRepairs()
{
	repairs->SetCellAlignment(lastRowRepairs,RPRIORITY,wxALIGN_CENTER, wxALIGN_TOP);
	repairs->SetCellAlignment(lastRowRepairs,RTEXT,wxALIGN_LEFT, wxALIGN_TOP);

	repairs->SetCellRenderer(lastRowRepairs, RPRIORITY, new wxGridCellChoiceRenderer);
	repairs->SetCellEditor(lastRowRepairs,RPRIORITY,new wxFastComboEditor(6,m_Priority));
	repairs->SetCellEditor(lastRowRepairs,RTEXT,new wxGridCellAutoWrapStringEditor);
}

void Maintenance::setAlignmentBuyParts()
{
	repairs->SetCellRenderer(lastRowBuyParts, RPRIORITY, new wxGridCellChoiceRenderer);
	buyparts->SetCellEditor(lastRowBuyParts,PPRIORITY,new wxFastComboEditor(6,m_Priority));
	buyparts->SetCellEditor(lastRowBuyParts,PARTS,new wxGridCellAutoWrapStringEditor);
	buyparts->SetCellAlignment(lastRowBuyParts,PPRIORITY,wxALIGN_CENTER, wxALIGN_TOP);
	buyparts->SetCellAlignment(lastRowBuyParts,PCATEGORY,wxALIGN_CENTER, wxALIGN_TOP);
	buyparts->SetCellAlignment(lastRowBuyParts,TITLE,wxALIGN_LEFT, wxALIGN_TOP);
	buyparts->SetCellAlignment(lastRowBuyParts,PARTS,wxALIGN_LEFT, wxALIGN_TOP);
	buyparts->SetCellAlignment(lastRowBuyParts,DATE,wxALIGN_CENTER, wxALIGN_TOP);
	buyparts->SetCellAlignment(lastRowBuyParts,AT,wxALIGN_LEFT, wxALIGN_TOP);
}

void Maintenance::loadData()
{
	wxString t, s;

	wxFileInputStream input( data_locn );
	wxTextInputStream* stream = new wxTextInputStream (input);

	int row = 0;
	while( (t = stream->ReadLine()))
	{
		if(input.Eof()) break;
		addLine();

		wxStringTokenizer tkz(t, _T("\t"),wxTOKEN_RET_EMPTY );
		int c = 0;
		while ( tkz.HasMoreTokens() )
		{
			s = dialog->restoreDangerChar(tkz.GetNextToken());
			s.RemoveLast();

			switch(c)
			{
			case TEXT:		grid->SetCellValue(row,TEXT,s); break;
			case IF:		grid->SetCellValue(row,IF,s); break;
			case WARN:		grid->SetCellValue(row,WARN,s); break;
			case URGENT:	grid->SetCellValue(row,URGENT,s); break;
			case START:		grid->SetCellValue(row,START,s); break;
			case ACTIVE:	grid->SetCellValue(row,ACTIVE,s); break;
			}
			c++;
		}
		row++;
	}

	wxFileInputStream input1( data_locnBuyParts );
	wxTextInputStream* stream1 = new wxTextInputStream (input1);

	row = 0;
	while( (t = stream1->ReadLine()))
	{
		if(input1.Eof()) break;
		addLineBuyParts();

		wxStringTokenizer tkz(t, _T("\t"),wxTOKEN_RET_EMPTY );
		int c = 0;
		while ( tkz.HasMoreTokens() )
		{
			s = dialog->restoreDangerChar(tkz.GetNextToken());
			s.RemoveLast();

			switch(c)
			{
			case PRIORITY:	buyparts->SetCellValue(row,PRIORITY,s);
							break;
			case PCATEGORY:	buyparts->SetCellValue(row,PCATEGORY,s);
							break;
			case TITLE:		buyparts->SetCellValue(row,TITLE,s);
							break;
			case PARTS   :	buyparts->SetCellValue(row,PARTS,s);
							break;
			case DATE    :	buyparts->SetCellValue(row,DATE,s);
							break;
			case AT      :	buyparts->SetCellValue(row,AT,s);
							break;
			}
			c++;
		}
		buyparts->AutoSizeRow(row,false);
		row++;
	}

	wxFileInputStream input2( data_locnRepairs );
	wxTextInputStream* stream2 = new wxTextInputStream (input2);

	row = 0;
	while( (t = stream2->ReadLine()))
	{
		if(input2.Eof()) break;
		addLineRepairs();

		wxStringTokenizer tkz(t, _T("\t"),wxTOKEN_RET_EMPTY );
		int c = 0;
		while ( tkz.HasMoreTokens() )
		{
			s = dialog->restoreDangerChar(tkz.GetNextToken());
			s.RemoveLast();

			switch(c)
			{
			case RPRIORITY:	repairs->SetCellValue(row,RPRIORITY,s);
							break;
			case RTEXT:		repairs->SetCellValue(row,RTEXT,s);
							break;
			}
			c++;
		}
		repairs->AutoSizeRow(row,false);
		row++;
	}

	checkService(dialog->m_gridGlobal->GetNumberRows()-1);
	checkRepairs();
	checkBuyParts();
	modified = false;
}

void Maintenance::buyParts(int i)
{
	wxString s;
	wxGrid *grid;
	int text, selectedRow;

	if(i == 0)
	{
		s = _("Service");
		grid = this->grid;
		text = TEXT;
		selectedRow = this->selectedRow;
	}
	else
	{
		s = _("Repairs");
		grid = repairs;
		text = RTEXT;
		selectedRow = selectedRowRepairs;
	}

	addLineBuyParts();

	dialog->m_gridMaintenanceBuyParts->SetCellValue(lastRowBuyParts,PPRIORITY,
		grid->GetCellValue(selectedRow,PRIORITY));
	dialog->m_gridMaintenanceBuyParts->SetCellValue(lastRowBuyParts,PCATEGORY,s);
	dialog->m_gridMaintenanceBuyParts->SetCellValue(lastRowBuyParts,TITLE,
		grid->GetCellValue(selectedRow,text).Trim());
	checkBuyParts();

	dialog->m_notebook6->SetSelection(2);
}

void Maintenance::setRowBackground(int row, wxColour &c)
{
	for(int i= 0; i < grid->GetNumberCols(); i++)
		grid->SetCellBackgroundColour(row,i,c);

	if(c == wxColour(255,0,0))
		grid->SetCellValue(row,PRIORITY,_T("1"));
	else if(c == wxColour(255,255,0))
		grid->SetCellValue(row,PRIORITY,_T("3"));
	else if(c == wxColour(0,255,0))
		grid->SetCellValue(row,PRIORITY,_T("5"));
	else if(c == wxColour(255,255,255))
		grid->SetCellValue(row,PRIORITY,_T("0"));
}

void Maintenance::checkService(int row)
{

	if(dialog->m_gridGlobal->GetNumberRows() == 0) return;

	wxString date;
	wxDateTime dtstart, dturgent, dtwarn;
	wxDateSpan spanu,spanw;
	wxString g, yesno;
	int choice = -1;
	double startValue, warnValue, urgentValue;
	double distanceTotal, motorTotal;
	wxString sign, cell;
	int border = 0;
	wxColour rowBack;

	for(int r = 0; r < grid->GetNumberRows(); r++)
	{
		g = grid->GetCellValue(r,IF);

		yesno = grid->GetCellValue(r,ACTIVE);
		if(g.IsEmpty()) continue;

		cell = grid->GetCellValue(r,START);
		cell.ToDouble(&startValue);
		cell = grid->GetCellValue(r,WARN);
		cell.ToDouble(&warnValue);
		cell = grid->GetCellValue(r,URGENT);
		cell.ToDouble(&urgentValue);

		cell = dialog->m_gridGlobal->GetCellValue(row,6);
		cell.ToDouble(&distanceTotal);
		cell = dialog->m_gridMotorSails->GetCellValue(row,1);
		cell.ToDouble(&motorTotal);

		if(g == m_choices[0])
			choice = 0;
		else if (g == m_choices[1])
			choice = 1;
		else if (g == m_choices[2])
			choice = 2;
		else if (g == m_choices[3])
			choice = 3;
		else if (g == m_choices[4])
			choice = 4;
		else if (g == m_choices[5])
			choice = 5;
		else if (g == m_choices[6])
			choice = 6;

		if(yesno != _("No"))
		{
			switch(choice)
			{
			case 0:
				if(distanceTotal >= startValue+urgentValue)
				{
					border = 2;
					rowBack = red;
					break;
				}
				else if(distanceTotal >= startValue+warnValue)
				{
					if(border != 2)
						border = 1;
					rowBack = yellow;
					break;
				}
				else
					rowBack = green;
				break;
			case 1:
				if(motorTotal >= startValue+urgentValue)
				{
					border = 2;
					rowBack = red;
					break;
				}
				else if(motorTotal >= startValue+warnValue)
				{
					if(border != 2)
						border = 1;
					rowBack = yellow;
					break;
				}
				else
					rowBack = green;
				break;
			case 2:
				if(grid->GetCellValue(r,URGENT) == dialog->m_gridGlobal->GetCellValue(row,3))
				{
					border = 2;
					rowBack = red;
					break;
				}
				else
					rowBack = green;
				break;
			case 3:	
				date = grid->GetCellValue(r,URGENT);
				dturgent.ParseDate(date);
				date = grid->GetCellValue(r,WARN);
				dtwarn.ParseDate(date);
				dtstart.ParseDate(dtstart.Now().FormatDate());

				if(dtstart >= dturgent)
				{
					border = 2;
					rowBack = red;
					break;
				}
				else if(dtstart >= dtwarn)
				{
					if(border != 2)
						border = 1;
					rowBack = yellow;
					break;
				}
				else
					rowBack = green;
				break;
			case 4:
				date = dialog->m_gridMaintanence->GetCellValue(r,START);
				dtstart.ParseDate(date.data());
				long days;
				grid->GetCellValue(r,WARN).ToLong(&days);
				spanw.SetDays((int)days);
				grid->GetCellValue(r,URGENT).ToLong(&days);
				spanu.SetDays((int)days);
				dturgent = dtstart;
				dturgent += spanu;
				dtwarn = dtstart;
				dtwarn += spanw;
				dtstart.ParseDate(dtstart.Now().FormatDate());

				if(dtstart >= dturgent )
				{
					border = 2;
					rowBack = red;
					break;
				}
				else if(dtstart >= dtwarn )
				{
					if(border != 2)
						border = 1;
					rowBack = yellow;
					break;
				}
				else
					rowBack = green;
				break;
			case 5:
				date = dialog->m_gridMaintanence->GetCellValue(r,START);
				dtstart.ParseDate(date.data());
				long weeks;
				grid->GetCellValue(r,WARN).ToLong(&weeks);
				spanw.SetWeeks((int) weeks);
				grid->GetCellValue(r,URGENT).ToLong(&weeks);
				spanu.SetWeeks((int) weeks);
				dturgent = dtstart;
				dturgent += spanu;
				dtwarn = dtstart;
				dtwarn += spanw;
				dtstart.ParseDate(dtstart.Now().FormatDate());

				if(dtstart >= dturgent )
				{
					border = 2;
					rowBack = red;
					break;
				}
				else if(dtstart >= dtwarn)
				{
					if(border != 2)
						border = 1;
					rowBack = yellow;
					break;
				}
				else
					rowBack = green;
				break;
			case 6:
				date = dialog->m_gridMaintanence->GetCellValue(r,START);
				dtstart.ParseDate(date.data());
				long month;
				grid->GetCellValue(r,WARN).ToLong(&month);
				spanw.SetMonths((int)month);
				grid->GetCellValue(r,URGENT).ToLong(&month);
				spanu.SetMonths((int) month);
				dturgent = dtstart;
				dturgent += spanu;
				dtwarn = dtstart;
				dtwarn += spanw;
				dtstart.ParseDate(dtstart.Now().FormatDate());

				if(dtstart >= dturgent )
				{
					border = 2;
					rowBack = red;
					break;
				}
				else if(dtstart >= dtwarn)
				{
					if(border != 2)
						border = 1;
					rowBack = yellow;
					break;
				}
				else
					rowBack = green;
			}
		}
		else
			setRowBackground(r,white);

		setRowBackground(r,rowBack);
		setBuyPartsPriority(grid,r,PRIORITY,TEXT);
	}

	checkBuyParts();

	switch(border)
	{
		case 0:
			dialog->SetBackgroundColour(dialog->defaultBackground);
			break;
		case 1:
			dialog->SetBackgroundColour(yellow);
			break;
		case 2:
			dialog->SetBackgroundColour(red);
			break;
	}
	dialog->Refresh();
}

void Maintenance::checkRepairs()
{
	for(int row = 0; row < repairs->GetNumberRows(); row++)
	{
		long i;
		repairs->GetCellValue(row,RPRIORITY).ToLong(&i);
		switch(i)
		{
		case 0:
			setRowBackgroundRepairs(row,white);
			break;
		case 1:
			setRowBackgroundRepairs(row,red);
			break;
		case 2:
			setRowBackgroundRepairs(row,redlight);
			break;
		case 3:
			setRowBackgroundRepairs(row,yellow);
			break;
		case 4:
			setRowBackgroundRepairs(row,yellowlight);
			break;
		case 5:
			setRowBackgroundRepairs(row,green);
			break;
		}
		setBuyPartsPriority(repairs,row,RPRIORITY,RTEXT);
	}
	repairs->Refresh();
	checkBuyParts();
}

void Maintenance::checkBuyParts()
{
	for(int row = 0; row < buyparts->GetNumberRows(); row++)
	{
		long i;
		buyparts->GetCellValue(row,RPRIORITY).ToLong(&i);
		switch(i)
		{
		case 0:
			setRowBackgroundBuyParts(row,white);
			break;
		case 1:
			setRowBackgroundBuyParts(row,red);
			break;
		case 2:
			setRowBackgroundBuyParts(row,redlight);
			break;
		case 3:
			setRowBackgroundBuyParts(row,yellow);
			break;
		case 4:
			setRowBackgroundBuyParts(row,yellowlight);
			break;
		case 5:
			setRowBackgroundBuyParts(row,green);
			break;
		}
	}
	buyparts->Refresh();
}

void Maintenance::setBuyPartsPriority(wxGrid *grid ,int row, int p, int t)
{
	wxString priority = grid->GetCellValue(row,p);
	for(int i = 0; i < buyparts->GetNumberRows(); i++)
	{
		if(buyparts->GetCellValue(i,TITLE).Trim() == grid->GetCellValue(row,t).Trim())
			buyparts->SetCellValue(i,PRIORITY,grid->GetCellValue(row,p));
	}
}

void Maintenance::setRowDone(int row)
{
	wxString g = grid->GetCellValue(selectedRow,IF);
	int choice = -1;

	if(g == m_choices[0])
			choice = 0;
	else if (g == m_choices[1])
			choice = 1;
	else if (g == m_choices[2])
			choice = 2;
	else if (g == m_choices[3])
			choice = 3;
	else if (g == m_choices[4])
			choice = 4;
	else if (g == m_choices[5])
			choice = 5;
	else if (g == m_choices[6])
			choice = 6;

	switch(choice)
	{
	case 0:
		grid->SetCellValue(selectedRow,START,
		  			   dialog->m_gridGlobal->GetCellValue(
					   dialog->m_gridGlobal->GetNumberRows()-1,6));
		break;
	case 1:
		grid->SetCellValue(selectedRow,START,
			dialog->m_gridMotorSails->GetCellValue(
					   dialog->m_gridGlobal->GetNumberRows()-1,1));
		break;
	case 2:
		grid->SetCellValue(selectedRow,ACTIVE,_("No"));
		checkService(dialog->m_gridGlobal->GetNumberRows()-1);
		break;
	case 3:
		grid->SetCellValue(selectedRow,ACTIVE,_("No"));
		grid->SetCellValue(selectedRow,WARN,wxDateTime::Now().FormatDate());
		grid->SetCellValue(selectedRow,URGENT,wxDateTime::Now().FormatDate());
	case 4:
	case 5:
	case 6:
		grid->SetCellValue(selectedRow,START,wxDateTime::Now().FormatDate());
		break;
	}

	if(grid->GetCellValue(row,ACTIVE) == _("Yes"))
		setRowBackground(row,green);
	else
		setRowBackground(selectedRow,white);
	grid->Refresh();
}

void Maintenance::setRepairDone(int row)
{
	repairs->SetCellValue(row,RPRIORITY,_("0"));
	setRowBackgroundRepairs(row, white);
	checkBuyParts();
}

void Maintenance::setRowBackgroundRepairs(int row, wxColour &c)
{
	for(int i = 0; i < repairs->GetNumberCols(); i++)
		repairs->SetCellBackgroundColour(row,i,c);
}

void Maintenance::setRowBackgroundBuyParts(int row, wxColour &c)
{
	for(int i = 0; i < buyparts->GetNumberCols(); i++)
		buyparts->SetCellBackgroundColour(row,i,c);
}

void Maintenance::cellCollChanged(int col, int row)
{
	if(dialog->m_gridGlobal->GetNumberRows() == 0) return;

	if(col == IF)
	{
		wxString g = grid->GetCellValue(selectedRow,IF);

		if(g == m_choices[0])
		{
					grid->SetCellValue(selectedRow,START,
							dialog->m_gridGlobal->GetCellValue(
							dialog->m_gridGlobal->GetNumberRows()-1,6));
						grid->SetCellValue(selectedRow,WARN,_T("1"));
						grid->SetCellValue(selectedRow,URGENT,_T("2"));
		}
		else if (g == m_choices[1])
		{
						grid->SetCellValue(selectedRow,START,
							dialog->m_gridMotorSails->GetCellValue(
							dialog->m_gridMotorSails->GetNumberRows()-1,1));
						grid->SetCellValue(selectedRow,WARN,_T("1"));
						grid->SetCellValue(selectedRow,URGENT,_T("2"));
		}
		else if (g == m_choices[2])
						grid->SetCellValue(selectedRow,WARN,
							dialog->m_gridGlobal->GetCellValue(
							dialog->m_gridGlobal->GetNumberRows()-1,3));
		else if (g == m_choices[3])
		{
						grid->SetCellValue(selectedRow,START,_T(""));
						grid->SetCellValue(selectedRow,WARN,  (wxDateTime::Now().Add(wxDateSpan(0,0,0,1))).FormatDate());
						grid->SetCellValue(selectedRow,URGENT,(wxDateTime::Now().Add(wxDateSpan(0,0,0,1))).FormatDate());
		}
		else if (g == m_choices[4] || g == m_choices[5] || g == m_choices[6])
		{
						grid->SetCellValue(selectedRow,START,wxDateTime::Now().FormatDate());
						grid->SetCellValue(selectedRow,WARN,_T("1"));
						grid->SetCellValue(selectedRow,URGENT,_T("2"));
		}
		col = WARN;
	}

	if(col == WARN || col == URGENT)
	{
		wxString ss;

		wxString g = grid->GetCellValue(row,IF);

		if(g == m_choices[0] || g == m_choices[1])
		{
			wxString s = grid->GetCellValue(row,START);
			s = s.substr(s.find_last_of(' '));
			wxDouble d; 
			grid->GetCellValue(row,col).ToDouble(&d);
			ss = wxString::Format(_T("%5.0f %s"),d,s.c_str());
		}
		else if(g == m_choices[4] || g == m_choices[5] || g == m_choices[6])
		{
			wxString s;
			if(g == m_choices[4])
				s = dialog->logbookPlugIn->opt->days;
			else if(g == m_choices[5])
				s = dialog->logbookPlugIn->opt->weeks;
			else if(g == m_choices[6])
				s = dialog->logbookPlugIn->opt->month;
			wxDouble d; 
			grid->GetCellValue(row,col).ToDouble(&d);
			ss = wxString::Format(_T("%5.0f %s"),d,s.c_str());
		}
		else 
		{
			ss = grid->GetCellValue(row,col);
		}

		if(col == WARN)
		{
			grid->SetCellValue(row,WARN,ss);
			grid->SetCellValue(row,URGENT,ss);
		}
		else
			grid->SetCellValue(row,URGENT,ss);
	}

	if(col == ACTIVE)
	{
		if(grid->GetCellValue(row,ACTIVE) == m_YesNo[0] && 
			grid->GetCellValue(row,PRIORITY) == _T("0"))
			grid->SetCellValue(row,PRIORITY,_T("5"));

		setBuyPartsPriority(grid ,row, PRIORITY, TEXT);
	}
}

void Maintenance::cellSelected(int col, int row)
{
	selectedCol = col;
	selectedRow = row;
}

void Maintenance::update()
{
//	if(!modified) return;

	wxString s = _T(""), temp;

	wxString newLocn = data_locn;
	newLocn.Replace(_T("txt"),_T("Bak"));
	wxRename(data_locn,newLocn);

	wxFileOutputStream output( data_locn );
	wxTextOutputStream* stream = new wxTextOutputStream (output);

	int count = grid->GetNumberRows();
	for(int r = 0; r < count; r++)
	{
		for(int c = 0; c < grid->GetNumberCols(); c++)
			{
				temp = grid->GetCellValue(r,c);
				s += dialog->replaceDangerChar(temp);
				s += _T(" \t");
			}
		s.RemoveLast();
		stream->WriteString(s+_T("\n"));
		s = _T("");
	}
	output.Close();
}

void Maintenance::updateRepairs()
{
//	if(!modified) return;
	wxString s = _T(""), temp;

	wxString newLocn = data_locnRepairs;
	newLocn.Replace(_T("txt"),_T("Bak"));
	wxRename(data_locnRepairs,newLocn);

	wxFileOutputStream output( data_locnRepairs );
	wxTextOutputStream* stream = new wxTextOutputStream (output);

	int count = dialog->m_gridMaintanenceRepairs->GetNumberRows();
	for(int r = 0; r < count; r++)
	{
		for(int c = 0; c < dialog->m_gridMaintanenceRepairs->GetNumberCols(); c++)
			{
				temp = dialog->m_gridMaintanenceRepairs->GetCellValue(r,c);
				s += dialog->replaceDangerChar(temp);
				s += _T(" \t");
			}
		s.RemoveLast();
		stream->WriteString(s+_T("\n"));
		s = _T("");
	}
	output.Close();
}

void Maintenance::updateBuyParts()
{
//	if(!modified) return;
	wxString s = _T(""), temp;

	wxString newLocn = data_locnBuyParts;
	newLocn.Replace(_T("txt"),_T("Bak"));
	wxRename(data_locnBuyParts,newLocn);

	wxFileOutputStream output( data_locnBuyParts );
	wxTextOutputStream* stream = new wxTextOutputStream (output);

	int count = dialog->m_gridMaintenanceBuyParts->GetNumberRows();
	for(int r = 0; r < count; r++)
	{
		for(int c = 0; c < dialog->m_gridMaintenanceBuyParts->GetNumberCols(); c++)
			{
				temp = dialog->m_gridMaintenanceBuyParts->GetCellValue(r,c);
				s += dialog->replaceDangerChar(temp);
				s += _T(" \t");
			}
		s.RemoveLast();
		stream->WriteString(s+_T("\n"));
		s = _T("");
	}
	output.Close();
}

void Maintenance::viewODT(int tab,wxString path,wxString layout,int mode)
{
	wxString locn, fn;

	if(tab == dialog->SERVICE)
	  {
	    locn = layout_locnService;
	    fn = data_locn;
	  }
	else if(tab == dialog->REPAIRS)
	{
	    locn = layout_locnRepairs;
	    fn = data_locnRepairs;
	}
	else if(tab == dialog->BUYPARTS)
	{
	    locn = this->layout_locnBuyParts;
	    fn = data_locnBuyParts;
	}

	toODT(tab,locn, layout, mode);

	if(layout != _T(""))
	{
	    fn.Replace(_T("txt"),_T("odt"));
		dialog->startApplication(fn,_T(".odt"));
	}
}

void Maintenance::viewHTML(int tab,wxString path,wxString layout,int mode)
{
	wxString locn, fn;

	if(tab == dialog->SERVICE)
	  {
	    locn = layout_locnService;
	    fn = data_locn;
	  }
	else if(tab == dialog->REPAIRS)
	{
	    locn = layout_locnRepairs;
	    fn = data_locnRepairs;
	}
	else if(tab == dialog->BUYPARTS)
	{
	    locn = this->layout_locnBuyParts;
	    fn = data_locnBuyParts;
	}

	toHTML(tab,locn, layout, mode);

	if(layout != _T(""))
	{
	    fn.Replace(_T("txt"),_T("html"));
		dialog->startBrowser(fn);
	}
}

wxString Maintenance::toHTML(int tab,wxString path,wxString layout,int mode)
{
	wxString top;
	wxString header;
	wxString middle;
	wxString bottom;

	wxString layout_loc;
	wxGrid * grid = NULL;

	wxString savePath = path;

	if(tab == dialog->SERVICE)
	{
		path = data_locn;
		layout_loc = layout_locnService;
		grid = this->grid;
	}
	else if(tab == dialog->REPAIRS)
	{

		path = data_locnRepairs;
		layout_loc = layout_locnRepairs;
		grid = repairs;
	}
	else if(tab == dialog->BUYPARTS)
	{
		path = this->data_locnBuyParts;
		layout_loc = layout_locnBuyParts;
		grid = buyparts;
	}

	wxString tempPath = path;

	wxString html = readLayoutHTML(layout_loc,layout);
	html = replaceLabels(html,grid);

	if(!cutInPartsHTML( html, &top, &header, &middle, &bottom))
		return _T("");

	wxTextFile* text = setFiles(savePath, &tempPath, mode);

	writeToHTML(text,grid,tempPath,layout_loc+layout+_T(".html"), top,header,middle,bottom,mode);

	return tempPath;
}

wxString Maintenance::replaceLabels(wxString s, wxGrid *grid)
{
	if(grid == this->grid)
	{
		s.Replace(_T("#LSERVICE#"),		dialog->m_notebook6->GetPageText(0));
		s.Replace(_T("#LPRIORITY#"),	grid->GetColLabelValue(0));
		s.Replace(_T("#LTEXT#"),		grid->GetColLabelValue(1));
		s.Replace(_T("#LIF#"),			grid->GetColLabelValue(2));
		s.Replace(_T("#LWARN#"),		grid->GetColLabelValue(3));
		s.Replace(_T("#LURGENT#"),		grid->GetColLabelValue(4));
		s.Replace(_T("#LSTART#"),		grid->GetColLabelValue(5));
		s.Replace(_T("#LACTIVE#"),		grid->GetColLabelValue(6));
	}
	else if(grid == repairs)
	{
		s.Replace(_T("#LREPAIRS#"),		dialog->m_notebook6->GetPageText(1));
		s.Replace(_T("#LPRIORITY#"),	grid->GetColLabelValue(0));
		s.Replace(_T("#LTEXT#"),		grid->GetColLabelValue(1));
	}
	else if(grid == buyparts)
	{
		s.Replace(_T("#LBUYPARTS#"),	dialog->m_notebook6->GetPageText(2));
		s.Replace(_T("#LPRIORITY#"),	grid->GetColLabelValue(0));
		s.Replace(_T("#LCATEGORY#"),	grid->GetColLabelValue(1));
		s.Replace(_T("#LTITLE#"),		grid->GetColLabelValue(2));
		s.Replace(_T("#LBUYPARTS#"),	grid->GetColLabelValue(3));
		s.Replace(_T("#LDATE#"),		grid->GetColLabelValue(4));
		s.Replace(_T("#LAT#"),			grid->GetColLabelValue(5));
	}

	return s;
}

wxString Maintenance::toODT(int tab,wxString path,wxString layout,int mode)
{
	wxString top;
	wxString header;
	wxString middle;
	wxString bottom;

	wxString layout_loc;
	wxGrid * grid = NULL;

	wxString savePath = path;

	if(tab == dialog->SERVICE)
	{
		path = data_locn;
		layout_loc = layout_locnService;
		grid = this->grid;
	}
	else if(tab == dialog->REPAIRS)
	{

		path = data_locnRepairs;
		layout_loc = layout_locnRepairs;
		grid = repairs;
	}
	else if(tab == dialog->BUYPARTS)
	{
		path = this->data_locnBuyParts;
		layout_loc = layout_locnBuyParts;
		grid = buyparts;
	}

	wxString tempPath = path;

	wxString odt = readLayoutODT(layout_loc,layout);
	odt = replaceLabels(odt,grid);

	if(!cutInPartsODT( odt, &top, &header,	&middle, &bottom))
		return _T("");

	wxTextFile* text = setFiles(savePath, &tempPath, mode);
	writeToODT(text,grid,tempPath,layout_loc+layout+_T(".odt"), top,header,middle,bottom,mode);

	return tempPath;
}

wxString Maintenance::setPlaceHolders(int mode, wxGrid *grid, int row, wxString middleODT)
{
	wxString s;

	if(grid == this->grid)
			s = setPlaceHoldersService(mode, grid, row, middleODT);		
	else if(grid == repairs)
			s = setPlaceHoldersRepairs(mode, grid, row, middleODT);	
	else if(grid == buyparts)
			s = setPlaceHoldersBuyParts(mode, grid, row, middleODT);

	return s;
}

wxString Maintenance::setPlaceHoldersService(int mode, wxGrid *grid, int row, wxString middleODT)
{
	wxString newMiddleODT;

	newMiddleODT = middleODT;
	newMiddleODT.Replace(wxT("#PRIORITY#"),replaceNewLine(mode,grid->GetCellValue(row,PRIORITY)));
	newMiddleODT.Replace(wxT("#TEXT#"),replaceNewLine(mode,grid->GetCellValue(row,TEXT)));
	newMiddleODT.Replace(wxT("#IF#"),replaceNewLine(mode,grid->GetCellValue(row,IF)));
	newMiddleODT.Replace(wxT("#WARN#"),replaceNewLine(mode,grid->GetCellValue(row,WARN)));
	newMiddleODT.Replace(wxT("#URGENT#"),replaceNewLine(mode,grid->GetCellValue(row,URGENT)));
	newMiddleODT.Replace(wxT("#START#"),replaceNewLine(mode,grid->GetCellValue(row,START)));
	newMiddleODT.Replace(wxT("#ACTIVE#"),replaceNewLine(mode,grid->GetCellValue(row,ACTIVE)));

	return newMiddleODT;
}

wxString Maintenance::setPlaceHoldersRepairs(int mode, wxGrid *grid, int row, wxString middleODT)
{
	wxString newMiddleODT;

	newMiddleODT = middleODT;
	newMiddleODT.Replace(wxT("#PRIORITY#"),replaceNewLine(mode,grid->GetCellValue(row,RPRIORITY)));
	newMiddleODT.Replace(wxT("#REPAIRSTEXT#"),replaceNewLine(mode,grid->GetCellValue(row,RTEXT)));

	return newMiddleODT;
}

wxString Maintenance::setPlaceHoldersBuyParts(int mode, wxGrid *grid, int row, wxString middleODT)
{
	wxString newMiddleODT;
	newMiddleODT = middleODT;

	newMiddleODT.Replace(wxT("#PRIORITY#"),replaceNewLine(mode,grid->GetCellValue(row,PPRIORITY)));
	newMiddleODT.Replace(wxT("#CATEGORY#"),replaceNewLine(mode,grid->GetCellValue(row,PCATEGORY)));
	newMiddleODT.Replace(wxT("#TITLE#"),replaceNewLine(mode,grid->GetCellValue(row,TITLE)));
	newMiddleODT.Replace(wxT("#BUYPARTS#"),replaceNewLine(mode,grid->GetCellValue(row,PARTS)));
	newMiddleODT.Replace(wxT("#DATE#"),replaceNewLine(mode,grid->GetCellValue(row,DATE)));
	newMiddleODT.Replace(wxT("#AT#"),replaceNewLine(mode,grid->GetCellValue(row,AT)));

	return newMiddleODT;
}

wxString Maintenance::replaceNewLine(int mode, wxString str)
{
	switch(mode)
	{
	case 0:
		 // HTML
		 str.Replace(wxT("\n"),wxT("<br>"));
		 break;
	case 1: // ODT
		 str.Replace(wxT("\n"),wxT("<text:line-break/>"));
		 break;
	}

	return str;
}

wxString Maintenance::readLayoutHTML(wxString path1,wxString layoutFileName)
{
	wxString html, path;

	path = path1 + layoutFileName + wxT(".html");;
	wxTextFile layout(path);
	
	layout.Open();

	for(unsigned int i = 0; i < layout.GetLineCount(); i++)
	{
		html += layout.GetLine(i)+_T("\n");
	}

	layout.Close();

	return html;
}

void Maintenance::showDateDialog(int row, int col, wxGrid* grid)
{
	DateDialog* d = new DateDialog(grid);
	if(d->ShowModal()== wxID_OK)
	{
		wxDateTime date = d->m_calendar2->GetDate();
		grid->SetCellValue(row,col,date.FormatDate());
	}
}
/////////////////////////////////////
// DateDialog
/////////////////////////////////////
DateDialog::DateDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxVERTICAL );
	
	m_calendar2 = new wxCalendarCtrl( this, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxCAL_SHOW_HOLIDAYS );
	bSizer21->Add( m_calendar2, 0, wxALL, 5 );
	
	m_sdbSizer6 = new wxStdDialogButtonSizer();
	m_sdbSizer6OK = new wxButton( this, wxID_OK );
	m_sdbSizer6->AddButton( m_sdbSizer6OK );
	m_sdbSizer6Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer6->AddButton( m_sdbSizer6Cancel );
	m_sdbSizer6->Realize();
	bSizer21->Add( m_sdbSizer6, 0, wxALIGN_CENTER, 5 );
	
	this->SetSizer( bSizer21 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_calendar2->Connect( wxEVT_CALENDAR_SEL_CHANGED, wxCalendarEventHandler( DateDialog::OnCalenderSelChanged ), NULL, this );
}

DateDialog::~DateDialog()
{
	// Disconnect Events
	m_calendar2->Disconnect( wxEVT_CALENDAR_SEL_CHANGED, wxCalendarEventHandler( DateDialog::OnCalenderSelChanged ), NULL, this );
	
}