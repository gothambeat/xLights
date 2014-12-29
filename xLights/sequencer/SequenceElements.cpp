#include "wx/wx.h"
#include <wx/utils.h>
#include <algorithm>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include "SequenceElements.h"


SequenceElements::SequenceElements()
{
}

SequenceElements::~SequenceElements()
{
}


void SequenceElements::AddElement(wxString &name,wxString &type,bool visible,bool collapsed)
{
    Element e(name,type,visible,collapsed);
    mElements.push_back(e);
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
        if(name == mElements[i].GetName())
        {
            return &mElements[i];
        }
    }
    return NULL;
}


Row_Information_Struct* SequenceElements::GetRowInformation(int index)
{
    if(index < mRowInformation.size())
    {
        return &mRowInformation[index];
    }
    else
    {
        return NULL;
    }
}

int SequenceElements::GetRowInformationSize()
{
    return mRowInformation.size();
}

void SequenceElements::SortElements()
{
    if (mRowInformation.size()>1)
        std::sort(mElements.begin(),mElements.end(),SortElementsByIndex);
}

void SequenceElements::MoveElement(int index,int destinationIndex)
{
    if(index<destinationIndex)
    {
        mElements[index].Index = destinationIndex;
        for(int i=index+1;i<destinationIndex;i++)
        {
            mElements[i].Index = i-1;
        }
    }
    else
    {
        mElements[index].Index = destinationIndex;
        for(int i=destinationIndex;i<index;i++)
        {
            mElements[i].Index = i+1;
        }
    }
    SortElements();

}

bool SequenceElements::LoadSequencerFile(wxString filename)
{
    wxString tmpStr;
    wxXmlDocument seqDocument;
    double startTime;
    double endTime;
    // read xml sequence info
    wxFileName FileObj(filename);
    FileObj.SetExt("xml");
    wxString SeqXmlFileName=FileObj.GetFullPath();
    int gridCol;
    if (!FileObj.FileExists())
    {
        //if (ChooseModels) ChooseModelsForSequence();
        return false;
    }
    // read xml
    //  first fix any version specific changes
    //FixVersionDifferences(SeqXmlFileName);
    if (!seqDocument.Load(SeqXmlFileName))
    {
        wxMessageBox(_("Error loading: ")+SeqXmlFileName);
        return false;
    }
    wxXmlNode* root=seqDocument.GetRoot();
    //wxString tempstr=root->GetAttribute("BaseChannel", "1");
    //tempstr.ToLong(&SeqBaseChannel);
    //tempstr=root->GetAttribute("ChanCtrlBasic", "0");
    //SeqChanCtrlBasic=tempstr!="0";
    //tempstr=root->GetAttribute("ChanCtrlColor", "0");
    //SeqChanCtrlColor=tempstr!="0";

    mElements.clear();
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "DisplayElements")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                wxString name = element->GetAttribute("name");
                wxString type = element->GetAttribute("type");
                bool visible = element->GetAttribute("visible")=='1'?1:0;
                bool collapsed = element->GetAttribute("collapsed")=='1'?1:0;
                AddElement(name,type,visible,collapsed);
            }
            PopulateRowInformation();
       }
       if (e->GetName() == "ElementEffects")
        {
            for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
            {
                if(elementNode->GetName()=="Element")
                {
                    Element* element = GetElement(elementNode->GetAttribute("name"));
                    if (element !=NULL)
                    {
                        for(wxXmlNode* effect=elementNode->GetChildren(); effect!=NULL; effect=effect->GetNext())
                        {
                            if (effect->GetName() == "Effect")
                            {
                                wxString effectText = effect->GetContent();
                                effect->GetAttribute("startTime").ToDouble(&startTime);
                                effect->GetAttribute("endTime").ToDouble(&endTime);
                                bool bProtected = effect->GetAttribute("protected")=='1'?1:0;
                                element->AddEffect(effectText,startTime,endTime,bProtected);
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

void SequenceElements::PopulateRowInformation()
{
    int rowIndex=0;
    Row_Information_Struct ri;
    mRowInformation.clear();
    for(int i=0;i<mElements.size();i++)
    {
        if(mElements[i].GetVisible())
        {
            ri.ElementName = mElements[i].GetName();
            ri.ElementType = mElements[i].GetType();
            ri.Collapsed = mElements[i].GetCollapsed();
            ri.PartOfView = false;
            ri.Index = rowIndex++;
            mRowInformation.push_back(ri);
            if(mElements[i].GetType()== "view" && !mElements[i].GetCollapsed())
            {
                wxString models = GetViewModels(mElements[i].GetName());
                if(models.length()> 0)
                {
                    wxArrayString model=wxSplit(models,',');
                    for(int m=0;m<model.size();m++)
                    {
                        ri.ElementName = model[m];
                        ri.ElementType = "model";
                        ri.Collapsed = false;
                        ri.PartOfView = true;
                        ri.Index = rowIndex++;
                        mRowInformation.push_back(ri);
                    }
                }
            }
        }
    }
}

