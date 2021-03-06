#include "wx/wx.h"
#include <wx/utils.h>
#include <algorithm>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include "SequenceElements.h"
#include "TimeLine.h"


SequenceElements::SequenceElements()
{
    mSelectedTimingRow = -1;
    mTimingRowCount = 0;
    mMaxRowsDisplayed = 0;
    mFirstVisibleModelRow = 0;
}

SequenceElements::~SequenceElements()
{
    for (int x = 0; x < mElements.size(); x++) {
        delete mElements[x];
    }
}


Element* SequenceElements::AddElement(wxString &name,wxString &type,bool visible,bool collapsed,bool active, bool selected)
{
    if(!ElementExists(name))
    {
        mElements.push_back(new Element(name,type,visible,collapsed,active,selected));
        return mElements[mElements.size()-1];
    }
}

Element* SequenceElements::AddElement(int index,wxString &name,wxString &type,bool visible,bool collapsed,bool active, bool selected)
{
    if(!ElementExists(name) && index <= mElements.size())
    {
        mElements.insert(mElements.begin()+index,new Element(name,type,visible,collapsed,active,selected));
        return mElements[index];
    }
}

int SequenceElements::GetElementCount()
{
    return mElements.size();
}

bool SequenceElements::ElementExists(wxString elementName)
{
    for(int i=0;i<mElements.size();i++)
    {
        if(mElements[i]->GetName() == elementName)
        {
            return true;
        }
    }
    return false;
}

void SequenceElements::SetViewsNode(wxXmlNode* viewsNode)
{
    mViewsNode = viewsNode;
}

wxString SequenceElements::GetViewModels(wxString viewName)
{
    wxString result="";
    for(wxXmlNode* view=mViewsNode->GetChildren(); view!=NULL; view=view->GetNext() )
    {
        if(view->GetAttribute("name")==viewName)
        {
            result = view->GetAttribute("models");
            break;
        }
    }
    return result;
}

Element* SequenceElements::GetElement(const wxString &name)
{
    for(int i=0;i<mElements.size();i++)
    {
        if(name == mElements[i]->GetName())
        {
            return mElements[i];
        }
    }
    return NULL;
}

Element* SequenceElements::GetElement(int index)
{
    if(index < mElements.size())
    {
        return mElements[index]
;    }
    else
    {
        return nullptr;
    }
}


void SequenceElements::DeleteElement(wxString name)
{
    for(int i=0;i<mElements.size();i++)
    {
        if(name == mElements[i]->GetName())
        {
            Element *e = mElements[i];
            mElements.erase(mElements.begin()+i);
            delete e;
        }
    }
}

Row_Information_Struct* SequenceElements::GetRowInformation(int index)
{
    if(index < mVisibleRowInformation.size())
    {
        return &mVisibleRowInformation[index];
    }
    else
    {
        return NULL;
    }
}

int SequenceElements::GetRowInformationSize()
{
    return mVisibleRowInformation.size();
}

void SequenceElements::SortElements()
{
    if (mElements.size()>1)
        std::sort(mElements.begin(),mElements.end(),SortElementsByIndex);
}

void SequenceElements::MoveElement(int index,int destinationIndex)
{
    if(index<destinationIndex)
    {
        mElements[index]->Index = destinationIndex;
        for(int i=index+1;i<destinationIndex;i++)
        {
            mElements[i]->Index = i-1;
        }
    }
    else
    {
        mElements[index]->Index = destinationIndex;
        for(int i=destinationIndex;i<index;i++)
        {
            mElements[i]->Index = i+1;
        }
    }
    SortElements();

}


bool SequenceElements::LoadSequencerFile(xLightsXmlFile& xml_file)
{
    wxXmlDocument& seqDocument = xml_file.GetXmlDocument();

    int gridCol;

    wxXmlNode* root=seqDocument.GetRoot();
    std::vector<wxString> effectStrings;
    mElements.clear();
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "DisplayElements")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                bool active=false;
                bool selected=false;
                bool collapsed=false;
                wxString name = element->GetAttribute("name");
                wxString type = element->GetAttribute("type");
                bool visible = element->GetAttribute("visible")=='1'?true:false;

                if (type=="timing")
                {
                    active = element->GetAttribute("active")=='1'?true:false;
                }
                else
                {
                    collapsed = element->GetAttribute("collapsed")=='1'?true:false;
                }
                Element* e = AddElement(name,type,visible,collapsed,active,selected);
                // Add models for each view
                if(type=="view")
                {
                    wxString models = GetViewModels(name);
                    if(models.length()> 0)
                    {
                        wxArrayString model=wxSplit(models,',');
                        for(int m=0;m<model.size();m++)
                        {
                           wxString modelName =  model[m];
                           wxString elementType = "model";
                           Element* e = AddElement(modelName,elementType,false,false,false,false);
                        }
                    }
                }
            }
       }
       else if (e->GetName() == "EffectDB")
       {
           effectStrings.clear();
           for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
           {
               if(elementNode->GetName()=="Effect")
               {
                   effectStrings.push_back(elementNode->GetNodeContent());
               }
           }
       }
       else if (e->GetName() == "ColorPalettes")
       {
           colorPalettes.clear();
           for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
           {
               if(elementNode->GetName()=="ColorPalette")
               {
                   colorPalettes.push_back(elementNode->GetNodeContent());
               }
           }
       }
       else if (e->GetName() == "ElementEffects")
        {
            for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
            {
                if(elementNode->GetName()=="Element")
                {
                    Element* element = GetElement(elementNode->GetAttribute("name"));
                    if (element !=NULL)
                    {
                        // check for fixed timing interval
                        double interval = 0.0;
                        if( elementNode->GetAttribute("type") == "timing" )
                        {
                            elementNode->GetAttribute("fixed").ToDouble(&interval);
                        }
                        if( interval > 0.0 )
                        {
                            element->SetFixedTiming((int)interval);
                            interval /= 1000.0;
                            element->AddEffectLayer();
                            EffectLayer* effectLayer = element->GetEffectLayer(0);
                            double time = 0.0;
                            double end_time = xml_file.GetSequenceDurationDouble();
                            double startTime, endTime, next_time;
                            while( time <= end_time )
                            {
                                next_time = (time + interval <= end_time) ? time + interval : end_time;
                                startTime = TimeLine::RoundToMultipleOfPeriod(time,mFrequency);
                                endTime = TimeLine::RoundToMultipleOfPeriod(next_time,mFrequency);
                                effectLayer->AddEffect(0,0,wxEmptyString,wxEmptyString,-1,startTime,endTime,EFFECT_NOT_SELECTED,false);
                                time += interval;
                            }
                        }
                        else
                        {
                            int layerIndex=0;
                            for(wxXmlNode* effectLayerNode=elementNode->GetChildren(); effectLayerNode!=NULL; effectLayerNode=effectLayerNode->GetNext())
                            {
                                if (effectLayerNode->GetName() == "EffectLayer")
                                {
                                    element->AddEffectLayer();
                                    EffectLayer* effectLayer = element->GetEffectLayer(layerIndex);
                                    for(wxXmlNode* effect=effectLayerNode->GetChildren(); effect!=NULL; effect=effect->GetNext())
                                    {
                                        if (effect->GetName() == "Effect")
                                        {
                                            wxString effectName;
                                            wxString settings;
                                            int id = 0;
                                            int effectIndex = 0;
                                            long palette = -1;
                                            bool bProtected=false;

                                            // Start time
                                            double startTime;
                                            effect->GetAttribute("startTime").ToDouble(&startTime);
                                            startTime = TimeLine::RoundToMultipleOfPeriod(startTime,mFrequency);
                                            // End time
                                            double endTime;
                                            effect->GetAttribute("endTime").ToDouble(&endTime);
                                            endTime = TimeLine::RoundToMultipleOfPeriod(endTime,mFrequency);
                                            // Protected
                                            bProtected = effect->GetAttribute("protected")=='1'?true:false;
                                            if(elementNode->GetAttribute("type") != "timing")
                                            {
                                                // Name
                                                effectName = effect->GetAttribute("name");
                                                effectIndex = Effect::GetEffectIndex(effectName);
                                                // ID
                                                id = wxAtoi(effect->GetAttribute("id"));
                                                if (effect->GetAttribute("ref") != "") {
                                                    settings = effectStrings[wxAtoi(effect->GetAttribute("ref"))];
                                                } else {
                                                    settings = effect->GetNodeContent();
                                                }

                                                wxString tmp;
                                                if (effect->GetAttribute("palette", &tmp)) {
                                                    tmp.ToLong(&palette);
                                                }
                                            }
                                            else
                                            {
                                                // store timing labels in name attribute
                                                effectName = effect->GetAttribute("label");

                                            }
                                            effectLayer->AddEffect(id,effectIndex,effectName,settings,palette,startTime,endTime,EFFECT_NOT_SELECTED,bProtected);
                                        }
                                    }
                                }
                                layerIndex++;
                            }
                        }
                    }
                }
            }
        }
    }
    PopulateRowInformation();
    // Set to the first model/view
    mFirstVisibleModelRow = 0;
    return true;
}

void SequenceElements::SetFrequency(double frequency)
{
    mFrequency = frequency;
}

double SequenceElements::GetFrequency()
{
    return mFrequency;
}


void SequenceElements::SetSelectedTimingRow(int row)
{
    mSelectedTimingRow = row;
}

int SequenceElements::GetSelectedTimingRow()
{
    return mSelectedTimingRow;
}


void SequenceElements::PopulateRowInformation()
{
    int rowIndex=0;
    int timingColorIndex=0;
    mSelectedTimingRow = -1;
    mRowInformation.clear();
    mTimingRowCount = 0;
    for(int i=0;i<mElements.size();i++)
    {
        if(mElements[i]->GetVisible())
        {
            if (mElements[i]->GetType()=="model")
            {
                if(!mElements[i]->GetCollapsed())
                {
                    for(int j =0; j<mElements[i]->GetEffectLayerCount();j++)
                    {
                        Row_Information_Struct ri;
                        ri.element = mElements[i];
                        ri.Collapsed = mElements[i]->GetCollapsed();
                        ri.Active = mElements[i]->GetActive();
                        ri.PartOfView = false;
                        ri.colorIndex = 0;
                        ri.layerIndex = j;
                        ri.Index = rowIndex++;
                        mRowInformation.push_back(ri);
                    }
                }
                else
                {
                    Row_Information_Struct ri;
                    ri.element = mElements[i];
                    ri.Collapsed = mElements[i]->GetCollapsed();
                    ri.Active = mElements[i]->GetActive();
                    ri.PartOfView = false;
                    ri.colorIndex = 0;
                    ri.layerIndex = 0;
                    ri.Index = rowIndex++;
                    mRowInformation.push_back(ri);
                }
            }
            else if (mElements[i]->GetType()=="timing")
            {
                Row_Information_Struct ri;
                ri.element = mElements[i];
                ri.Collapsed = mElements[i]->GetCollapsed();
                ri.Active = mElements[i]->GetActive();
                ri.PartOfView = false;
                ri.colorIndex = timingColorIndex;
                ri.layerIndex = 0;
                if(mSelectedTimingRow<0)
                {
                    mSelectedTimingRow = ri.Active?rowIndex:-1;
                }

                ri.Index = rowIndex++;
                mRowInformation.push_back(ri);
                timingColorIndex++;
                mTimingRowCount++;
            }
            else        // View
            {
                Row_Information_Struct ri;
                ri.element = mElements[i];
                ri.Collapsed = mElements[i]->GetCollapsed();
                ri.Active = mElements[i]->GetActive();
                ri.PartOfView = false;
                ri.colorIndex = 0;
                ri.layerIndex = 0;
                ri.Index = rowIndex++;
                mRowInformation.push_back(ri);
                if(!mElements[i]->GetCollapsed())
                {
                    // Add models/effect layers in view
                    wxString models = GetViewModels(mElements[i]->GetName());
                    if(models.length()> 0)
                    {
                        wxArrayString model=wxSplit(models,',');
                        for(int m=0;m<model.size();m++)
                        {
                            Element* element = GetElement(model[m]);
                            if(element->GetCollapsed())
                            {
                                for(int j=0;element->GetEffectLayerCount();j++)
                                {
                                    Row_Information_Struct r;
                                    r.element = element;
                                    r.Collapsed = element->GetCollapsed();
                                    r.Active = mElements[i]->GetActive();
                                    r.PartOfView = false;
                                    r.colorIndex = 0;
                                    r.layerIndex = j;
                                    r.Index = rowIndex++;
                                    mRowInformation.push_back(r);
                                }
                            }
                            else
                            {
                                Row_Information_Struct r;
                                r.element = element;
                                r.Collapsed = element->GetCollapsed();
                                r.Active = mElements[i]->GetActive();
                                r.PartOfView = false;
                                r.colorIndex = 0;
                                r.layerIndex = 0;
                                r.Index = rowIndex++;
                                mRowInformation.push_back(r);
                            }
                        }
                    }
                }
            }
        }
    }
    PopulateVisibleRowInformation();
}

void SequenceElements::PopulateVisibleRowInformation()
{
    if(mRowInformation.size()==0){return;}
    int row=0;
    mVisibleRowInformation.clear();
    // Add all timing element rows. They should always be first in the list
    for(row=0;row<mTimingRowCount;row++)
    {
        if(row<mMaxRowsDisplayed)
        {
            mVisibleRowInformation.push_back(mRowInformation[row]);
        }
    }

    for(;row<mMaxRowsDisplayed && row+mFirstVisibleModelRow<mRowInformation.size();row++)
    {
        mVisibleRowInformation.push_back(mRowInformation[row+mFirstVisibleModelRow]);
    }
}

void SequenceElements::SetFirstVisibleModelRow(int row)
{
    // They all fit on screen. So set to first model element.
    if(mRowInformation.size() <= mMaxRowsDisplayed)
    {
        mFirstVisibleModelRow = 0;
    }
    else
    {
        int maxModelRowsThatCanBeDisplayed = mMaxRowsDisplayed - mTimingRowCount;
        int totalModelRowsToDisplay = mRowInformation.size() - mTimingRowCount;

        int leastRowToDisplay = (totalModelRowsToDisplay - maxModelRowsThatCanBeDisplayed);
        if(row > leastRowToDisplay)
        {
            mFirstVisibleModelRow = leastRowToDisplay;
        }
        else
        {
            mFirstVisibleModelRow = row;
        }
        for(int i=mFirstVisibleModelRow;i<mRowInformation.size();i++)
        {
            // Make sure the first visible row is a layer 0 row.
            // We want the first row to always be a layer 0
            if(mRowInformation[i+mTimingRowCount].layerIndex == 0)
            {
                mFirstVisibleModelRow = i;
                break;
            }
        }
    }
    PopulateVisibleRowInformation();
}

int SequenceElements::GetNumberOfTimingRows()
{
    return mTimingRowCount;
}

// Returns the last view index or if no views in vector
// the last timing index or if no timing in vector returns 0
int SequenceElements::GetLastViewIndex()
{
    for(int i=0;i<mElements.size();i++)
    {
        if(mElements[i]->GetType() != "view" && mElements[i]->GetType() != "timing")
        {
            return i;
        }
    }
    return 0;
}

void SequenceElements::DeactivateAllTimingElements()
{
    for(int i=0;i<mElements.size();i++)
    {
        if(mElements[i]->GetType()=="timing")
        {
            mElements[i]->SetActive(false);
        }
    }
}

void SequenceElements::SelectEffectsInRowAndPositionRange(int startRow, int endRow, int startX,int endX, int &FirstSelected)
{
    if(startRow<mVisibleRowInformation.size())
    {
        if(endRow>=mVisibleRowInformation.size())
        {
            endRow = mVisibleRowInformation.size()-1;
        }
        for(int i=startRow;i<=endRow;i++)
        {
            EffectLayer* effectLayer = mVisibleRowInformation[i].element->GetEffectLayer(mVisibleRowInformation[i].layerIndex);
            effectLayer->SelectEffectsInPositionRange(startX,endX,FirstSelected);
        }
    }
}

Effect* SequenceElements::GetSelectedEffectAtRowAndPosition(int row, int x,int &index, int &selectionType)
{
    EffectLayer* effectLayer = mVisibleRowInformation[row].element->GetEffectLayer(mVisibleRowInformation[row].layerIndex);

    index = effectLayer->GetEffectIndexThatContainsPosition(x,selectionType);
    if(index<0)
    {
        return nullptr;
    }
    else
    {
        return effectLayer->GetEffect(index);
    }
}



void SequenceElements::UnSelectAllEffects()
{
    for(int i=0;i<mRowInformation.size();i++)
    {
        EffectLayer* effectLayer = mRowInformation[i].element->GetEffectLayer(mRowInformation[i].layerIndex);
        effectLayer->UnSelectAllEffects();
    }
}

void SequenceElements::UnSelectAllElements()
{
    for(int i=0;i<mElements.size();i++)
    {
        mElements[i]->SetSelected(false);
    }
}

// Functions to manage selected ranges
int SequenceElements::GetSelectedRangeCount()
{
    return mSelectedRanges.size();
}

EffectRange* SequenceElements::GetSelectedRange(int index)
{
    return &mSelectedRanges[index];
}

void SequenceElements::AddSelectedRange(EffectRange* range)
{
    mSelectedRanges.push_back(*range);
}

void SequenceElements::DeleteSelectedRange(int index)
{
    if(index < mSelectedRanges.size())
    {
        mSelectedRanges.erase(mSelectedRanges.begin()+index);
    }
}

void SequenceElements::ClearSelectedRanges()
{
    mSelectedRanges.clear();
}

void SequenceElements::SetMaxRowsDisplayed(int maxRows)
{
    mMaxRowsDisplayed = maxRows;
}

int SequenceElements::GetMaxModelsDisplayed()
{
    return mMaxRowsDisplayed - mTimingRowCount;
}

int SequenceElements::GetTotalNumberOfModelRows()
{
    return mRowInformation.size() - mTimingRowCount;
}

int SequenceElements::GetFirstVisibleModelRow()
{
    return mFirstVisibleModelRow;
}


int SequenceElements::getPaletteIndex(const wxString &p) {
    for (int x = 0; x < colorPalettes.size(); x++) {
        if (p == colorPalettes[x]) {
            return x;
        }
    }
    colorPalettes.push_back(p);
    return colorPalettes.size() - 1;
}
static const wxString EMPTY_STRING;
const wxString& SequenceElements::getPalette(int i) {
    if (i == -1) {
        return EMPTY_STRING;
    }
    return colorPalettes[i];
}
int SequenceElements::getNumberOfPalettes() {
    return colorPalettes.size();
}

void SequenceElements::SetVisibilityForAllModels(bool visibility)
{
    for(int i=0;i<GetElementCount();i++)
    {
        Element * e = GetElement(i);
        if(e->GetType() == "model")
        {
            e->SetVisible(visibility);
        }
    }
}

void SequenceElements::MoveSequenceElement(int index, int dest)
{
    if(index<mElements.size() && dest<mElements.size())
    {
        Element* e = mElements[index];
        mElements.erase(mElements.begin()+index);
        if(index >= dest)
        {
            mElements.insert(mElements.begin()+dest,e);
        }
        else
        {
            mElements.insert(mElements.begin()+(dest-1),e);
        }
    }
}


