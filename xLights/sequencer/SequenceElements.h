#ifndef SEQUENCEELEMENTS_H
#define SEQUENCEELEMENTS_H

#include "wx/wx.h"
#include <vector>
#include "wx/xml/xml.h"
#include "ElementEffects.h"
#include "Element.h"


struct Row_Information_Struct
{
    int Index;
    wxString ElementName;
    wxString ElementType;
    int RowNumber;
    bool Collapsed;
    bool PartOfView;
};


class SequenceElements
{
    public:
        SequenceElements();
        virtual ~SequenceElements();
        bool LoadSequencerFile(wxString filename);
        bool SeqLoadXlightsFile(const wxString& filename, bool ChooseModels);
        void AddElement(wxString &name, wxString &type,bool visible,bool collapsed);
        Element* GetElement(const wxString &name);
        Row_Information_Struct* GetRowInformation(int index);
        int GetRowInformationSize();

        void SetViewsNode(wxXmlNode* viewsNode);
        wxString GetViewModels(wxString viewName);

        void SortElements();
        void MoveElement(int index,int destinationIndex);
        void PopulateRowInformation();

    protected:
    private:
    std::vector<Element> mElements;
    std::vector<Row_Information_Struct> mRowInformation;
    wxXmlNode* mViewsNode;


    static bool SortElementsByIndex(const Element &element1,const Element &element2)
    {
        return (element1.Index<element2.Index);
    }


};


#endif // SEQUENCEELEMENTS_H