#ifndef DRAGEFFECTBITMAPBUTTON_H
#define DRAGEFFECTBITMAPBUTTON_H
#include "wx/wx.h"

wxDECLARE_EVENT(EVT_SELECTED_EFFECT_CHANGED, wxCommandEvent);

class DragEffectBitmapButton : public wxBitmapButton
{
    public:
//        DragEffectBitmapButton();

        DragEffectBitmapButton (wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos=wxDefaultPosition,
                                const wxSize &size=wxDefaultSize, long style=wxBU_AUTODRAW, const wxValidator &validator=wxDefaultValidator,
                                const wxString &name=wxButtonNameStr);

        virtual ~DragEffectBitmapButton();
        void OnMouseLeftDown (wxMouseEvent& event);
        void SetBitmap(const char** xpm);
        void SetEffectIndex(int index);
    protected:
    private:
        const char** mDragIconBuffer;

        wxString (*GetStringFunction)();

        int mEffectIndex;
};

#endif // DRAGEFFECTBITMAPBUTTON_H
