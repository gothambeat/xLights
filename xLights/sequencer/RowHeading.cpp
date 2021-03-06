#include "RowHeading.h"
#include "Waveform.h"
#include "wx/wx.h"
#include "wx/brush.h"
#include "../xLightsMain.h"
#include "EffectDropTarget.h"


BEGIN_EVENT_TABLE(RowHeading, wxWindow)
//EVT_MOTION(RowHeading::mouseMoved)
EVT_LEFT_DOWN(RowHeading::mouseLeftDown)
//EVT_LEFT_UP(RowHeading::mouseLeftUp)
//EVT_LEAVE_WINDOW(RowHeading::mouseLeftWindow)
EVT_RIGHT_DOWN(RowHeading::rightClick)
//EVT_SIZE(RowHeading::resized)
//EVT_KEY_DOWN(RowHeading::keyPressed)
//EVT_KEY_UP(RowHeading::keyReleased)
//EVT_MOUSEWHEEL(RowHeading::mouseWheelMoved)
EVT_PAINT(RowHeading::render)
END_EVENT_TABLE()

// Menu constants
const long RowHeading::ID_ROW_MNU_ADD_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_PLAY_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS = wxNewId();

// Timing Track popup menu
const long RowHeading::ID_ROW_MNU_ADD_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_TIMING_TRACK = wxNewId();



RowHeading::RowHeading(MainSequencer* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name):
                       wxWindow((wxWindow*)parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    DOUBLE_BUFFER(this);
    mHeaderColorModel = new wxColour(212,208,200,30);
    mHeaderColorView = new wxColour(159,157,152,30);
    mHeaderColorTiming = new wxColour(130,178,207,30);
    mHeaderSelectedColor = new wxColour(130,178,207,30);
    SetDropTarget(new EffectDropTarget((wxWindow*)this,false));

}

RowHeading::~RowHeading()
{
}

void RowHeading::mouseLeftDown( wxMouseEvent& event)
{
    int rowIndex = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if(rowIndex < mSequenceElements->GetRowInformationSize())
    {
        bool result;
        Element* e = mSequenceElements->GetRowInformation(rowIndex)->element;
        if(e->GetType()=="model")
        {
            mSequenceElements->UnSelectAllElements();
            e->SetSelected(true);
            Refresh(false);
        }
        if(HitTestCollapseExpand(rowIndex,event.GetX(),&result))
        {
            e->SetCollapsed(!result);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
        else if(HitTestTimingActive(rowIndex,event.GetX(),&result))
        {
            mSequenceElements->DeactivateAllTimingElements();
            e->SetActive(!result);
            // Set the selected timing row.
            int selectedTimingRow = result?rowIndex:-1;
            mSequenceElements->SetSelectedTimingRow(selectedTimingRow);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
}

void RowHeading::rightClick( wxMouseEvent& event)
{
    wxMenu *mnuLayer;
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetRowInformationSize()) {
        return;
    }

    Element* element = mSequenceElements->GetRowInformation(mSelectedRow)->element;
    if(element->GetType()=="model" || element->GetType()=="view")
    {
        mnuLayer = new wxMenu();
        mnuLayer->Append(ID_ROW_MNU_ADD_LAYER,"Add Layer");
        if(element->GetEffectLayerCount() > 1)
        {
            mnuLayer->Append(ID_ROW_MNU_DELETE_LAYER,"Delete Layer");
        }
        if (element->GetType()=="model") {
            mnuLayer->AppendSeparator();
            mnuLayer->Append(ID_ROW_MNU_PLAY_MODEL,"Play Model");
            mnuLayer->Append(ID_ROW_MNU_EXPORT_MODEL,"Export Model");
        }
    }
    else
    {
        mnuLayer = new wxMenu();
        mnuLayer->Append(ID_ROW_MNU_ADD_TIMING_TRACK,"Add Timing Track");
        mnuLayer->Append(ID_ROW_MNU_DELETE_TIMING_TRACK,"Delete Timing Track");
        mnuLayer->Append(ID_ROW_MNU_IMPORT_TIMING_TRACK,"Import Timing Track");
    }

    mnuLayer->AppendSeparator();
    mnuLayer->Append(ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS,"Edit Display Elements");
    mnuLayer->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&RowHeading::OnLayerPopup, NULL, this);
    PopupMenu(mnuLayer);
}

void RowHeading::OnLayerPopup(wxCommandEvent& event)
{
    Element* element = mSequenceElements->GetRowInformation(mSelectedRow)->element;
    int id = event.GetId();
    if(id == ID_ROW_MNU_ADD_LAYER)
    {
        element->AddEffectLayer();
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }
    else if(id == ID_ROW_MNU_DELETE_LAYER)
    {
        int layerIndex = mSequenceElements->GetRowInformation(mSelectedRow)->layerIndex;
        wxString prompt = wxString::Format("Delete 'Layer %d' of '%s'?",
                                      layerIndex+1,element->GetName());
        wxString caption = "Comfirm Layer Deletion";

        int answer = wxMessageBox(prompt,caption,wxYES_NO);
        if(answer == wxYES)
        {
            element->RemoveEffectLayer(layerIndex);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if(id == ID_ROW_MNU_ADD_TIMING_TRACK)
    {
        wxString name = wxGetTextFromUser("What is name of new timing track?", "Timing Track Name");
        if(name.size()>0)
        {
            // Deactivate active timing mark so new one is selected;
            mSequenceElements->DeactivateAllTimingElements();
            int timingCount = mSequenceElements->GetNumberOfTimingRows();
            wxString type = "timing";
            Element* e = mSequenceElements->AddElement(timingCount,name,type,true,false,true,false);
            e->AddEffectLayer();
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
    else if(id == ID_ROW_MNU_DELETE_TIMING_TRACK)
    {
        wxString prompt = wxString::Format("Delete 'Timing Track '%s'?",element->GetName());
        wxString caption = "Comfirm Timing Track Deletion";

        int answer = wxMessageBox(prompt,caption,wxYES_NO);
        if(answer == wxYES)
        {
            mSequenceElements->DeleteElement(element->GetName());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if(id == ID_ROW_MNU_IMPORT_TIMING_TRACK) {
        wxCommandEvent playEvent(EVT_IMPORT_TIMING);
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_EXPORT_MODEL) {
        wxCommandEvent playEvent(EVT_EXPORT_MODEL);
        playEvent.SetString(element->GetName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_PLAY_MODEL) {
        wxCommandEvent playEvent(EVT_PLAY_MODEL);
        playEvent.SetString(element->GetName());
        wxPostEvent(GetParent(), playEvent);
    }

    else if(id==ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS)
    {
        wxCommandEvent displayElementEvent(EVT_SHOW_DISPLAY_ELEMENTS);
        wxPostEvent(GetParent(), displayElementEvent);
    }


    // Make sure message box is painted over by grid.
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}


bool RowHeading::HitTestCollapseExpand(int row,int x, bool* IsCollapsed)
{
    if(mSequenceElements->GetRowInformation(row)->element->GetType() != "timing" &&
       x<DEFAULT_ROW_HEADING_MARGIN)
    {
        *IsCollapsed = mSequenceElements->GetRowInformation(row)->Collapsed;
        return true;
    }
    else
    {
        return false;
    }
}

bool RowHeading::HitTestTimingActive(int row,int x, bool* IsActive)
{
    if(mSequenceElements->GetRowInformation(row)->element->GetType() == "timing" &&
       x<DEFAULT_ROW_HEADING_MARGIN)
    {
        *IsActive = mSequenceElements->GetRowInformation(row)->Active;
        return true;
    }
    else
    {
        return false;
    }
}

void RowHeading::SetCanvasSize(int width,int height)
{
    SetSize(width,height);
    wxSize s;
    s.SetWidth(width);
    s.SetHeight(height);
    SetMaxSize(s);
    SetMinSize(s);
}

int RowHeading::GetMaxRows()
{
    int max = (int)(getHeight()/DEFAULT_ROW_HEADING_HEIGHT);
    return max;
}

void RowHeading::render( wxPaintEvent& event )
{
    wxCoord w,h;
    wxPaintDC dc(this);
    wxPen penOutline(wxColor(32,32,32), .1);
    dc.GetSize(&w,&h);
    wxBrush brush(*mHeaderColorModel,wxBRUSHSTYLE_SOLID);
    dc.SetBrush(brush);
    dc.SetPen(penOutline);
    int row=0;
    int startY,endY;
    for(int i =0;i< mSequenceElements->GetRowInformationSize();i++)
    {
        wxBrush brush(*GetHeaderColor(mSequenceElements->GetRowInformation(i)),wxBRUSHSTYLE_SOLID);
        dc.SetBrush(brush);
        startY = DEFAULT_ROW_HEADING_HEIGHT*row;
        endY = DEFAULT_ROW_HEADING_HEIGHT*(row+1);
        dc.SetBackgroundMode(wxTRANSPARENT);
        dc.DrawRectangle(0,startY,w,DEFAULT_ROW_HEADING_HEIGHT);
        if(mSequenceElements->GetRowInformation(i)->layerIndex>0)   // If effect layer = 0
        {
            dc.SetPen(*wxLIGHT_GREY_PEN);
            dc.DrawLine(1,startY,w-1,startY);
            dc.DrawLine(1,startY-1,w-1,startY-1);
            dc.SetPen(*wxBLACK_PEN);
        }
        else        // Draw label
        {
            if(mSequenceElements->GetRowInformation(i)->PartOfView)
            {
                wxRect r(INDENT_ROW_HEADING_MARGIN,startY,w-(INDENT_ROW_HEADING_MARGIN),22);
                dc.DrawLabel(mSequenceElements->GetRowInformation(i)->element->GetName(),r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
            }
            else
            {
                wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,22);
                dc.DrawLabel(mSequenceElements->GetRowInformation(i)->element->GetName(),r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
            }
        }

        if(mSequenceElements->GetRowInformation(i)->element->GetType()=="view")
        {
            dc.SetBrush(*wxWHITE_BRUSH);
            dc.SetPen(*wxBLACK_PEN);
            dc.DrawRectangle(2,startY+7,9,9);
            dc.DrawLine(4,startY+11,9,startY+11);
            if(mSequenceElements->GetRowInformation(i)->Collapsed)
            {
                dc.DrawLine(6,startY+9,6,startY+14);
            }
            dc.SetPen(penOutline);
            dc.SetBrush(brush);
        }
        else if (mSequenceElements->GetRowInformation(i)->element->GetType()=="model")
        {
            if(mSequenceElements->GetRowInformation(i)->element->GetEffectLayerCount() > 1 &&
               mSequenceElements->GetRowInformation(i)->layerIndex == 0)
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.SetPen(*wxBLACK_PEN);
                dc.DrawRectangle(2,startY+7,9,9);
                dc.DrawLine(4,startY+11,9,startY+11);
                if(mSequenceElements->GetRowInformation(i)->Collapsed)
                {
                    dc.DrawLine(6,startY+9,6,startY+14);
                }
                dc.SetPen(penOutline);
                dc.SetBrush(brush);
            }
        }
        else if(mSequenceElements->GetRowInformation(i)->element->GetType()=="timing")
        {
            dc.SetPen(*wxBLACK_PEN);
            if(mSequenceElements->GetRowInformation(i)->Active)
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.DrawCircle(7,startY+11,5);

                dc.SetBrush(*wxGREY_BRUSH);
                dc.DrawCircle(7,startY+11,2);
            }
            else
            {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.DrawCircle(7,startY+11,5);
            }
            dc.SetPen(penOutline);
            dc.SetBrush(brush);
        }
        row++;
    }
    wxBrush b(*mHeaderColorModel,wxBRUSHSTYLE_SOLID);
    dc.SetBrush(b);
    dc.DrawRectangle(0,endY,w,h);

}

const wxColour* RowHeading::GetHeaderColor(Row_Information_Struct* info)
{
    if (info->element->GetType() == "model")
    {
        if(info->PartOfView)
        {
            return mHeaderColorView;
        }
        else
        {
            if (info->element->GetSelected())
            {
                return  mHeaderSelectedColor;
            }
            else
            {
                return mHeaderColorModel;
            }
        }
    }
    else if (info->element->GetType() == "view")
    {
        return mHeaderColorView;
    }
    else
    {
        return GetTimingColor(info->colorIndex);
    }
}

void RowHeading::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void RowHeading::DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row)
{
}

int RowHeading::getWidth()
{
    return GetSize().x;
}

int RowHeading::getHeight()
{
    return GetSize().y;
}

const wxColour* RowHeading::GetTimingColor(int colorIndex)
{
    const wxColour* value;
    switch(colorIndex%5)
    {
        case 0:
            //
            value = wxCYAN;
            break;
        case 1:
            value = wxRED;
            break;
        case 2:
            value = wxGREEN;
            break;
        case 3:
            value = wxBLUE;
            break;
        default:
            value = wxYELLOW;
            break;
    }
    return value;
}







