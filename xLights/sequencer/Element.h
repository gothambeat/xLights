#ifndef ELEMENT_H
#define ELEMENT_H

#include "wx/wx.h"
#include <vector>
#include "wx/xml/xml.h"
#include "EffectLayer.h"


enum ElementType
{
    ELEMENT_TYPE_MODEL,
    ELEMENT_TYPE_VIEW,
    ELEMENT_TYPE_TIMING
};

class Element
{
    public:
        Element(wxString &name, wxString &type,bool visible,bool collapsed, bool active, bool selected);
        virtual ~Element();

        wxString GetName();
        void SetName(wxString &name);

        bool GetVisible();
        void SetVisible(bool visible);

        bool GetCollapsed();
        void SetCollapsed(bool collapsed);

        bool GetActive();
        void SetActive(bool active);

        bool GetSelected();
        void SetSelected(bool selected);

        int GetFixedTiming();
        void SetFixedTiming(int fixed);

        wxString GetType();
        void SetType(wxString &type);

        EffectLayer* GetEffectLayer(int index);
        int GetEffectLayerCount();

        void AddEffectLayer();
        void RemoveEffectLayer(int index);

        int GetIndex();
        void SetIndex(int index);

        int Index;
    
        void IncrementChangeCount();
        int getChangeCount() const { return changeCount; }
    protected:
    private:
        volatile int changeCount;

        int mIndex;
        wxString mName;
        wxString mElementType;
        bool mVisible;
        bool mCollapsed;
        bool mActive;
        bool mSelected;
        int mFixed;
        std::vector<EffectLayer*> mEffectLayers;
};

#endif // ELEMENT_H
