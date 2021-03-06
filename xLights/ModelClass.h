/***************************************************************
 * Name:      ModelClass.h
 * Purpose:   Represents Display Model
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-10-22
 * Copyright: 2012 by Matt Brown
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#ifndef MODELCLASS_H
#define MODELCLASS_H

#include <memory>
#include <vector>
#include "SequencePreview.h"
#include <wx/xml/xml.h>
#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/window.h>
#include <wx/dcclient.h>
#include <stdint.h>
#include <wx/choice.h>
#include <wx/checklst.h>
#include <wx/listbox.h>

#include "ModelPreview.h" 
#include "Color.h"


typedef std::vector<long> StartChannelVector_t;

#define NODE_RGB_CHAN_CNT           3
#define NODE_RGBW_CHAN_CNT          4
#define NODE_SINGLE_COLOR_CHAN_CNT  1
#define RECT_HANDLE_WIDTH           6
#define BOUNDING_RECT_OFFSET        8

class ModelClass
{
private:

    class NodeBaseClass
    {
    private:

        // buffer and screen coordinates for displayed nodes
        struct CoordStruct
        {
            wxCoord bufX, bufY, screenX, screenY;
        };

    protected:
        // color values in rgb order
        uint8_t c[3];
        // color channel offsets, rgb would be 0,1,2
        uint8_t offsets[3];
        int chanCnt;

    public:

        int sparkle;
        int ActChan;   // 0 is the first channel
        int StringNum; // node is part of this string (0 is the first string)
        std::vector<CoordStruct> Coords;
        std::vector<CoordStruct> OrigCoords;

        NodeBaseClass()
        {
            chanCnt=NODE_RGB_CHAN_CNT;
            offsets[0] = 0;
            offsets[1] = 1;
            offsets[2] = 2;
        }
        
        NodeBaseClass(int StringNumber, size_t NodesPerString)
        {
            StringNum=StringNumber;
            Coords.resize(NodesPerString);
            chanCnt=NODE_RGB_CHAN_CNT;
            offsets[0] = 0;
            offsets[1] = 1;
            offsets[2] = 2;
        }
        NodeBaseClass(int StringNumber, size_t NodesPerString, const wxString &rgbOrder)
        {
            StringNum=StringNumber;
            Coords.resize(NodesPerString);
            chanCnt=NODE_RGB_CHAN_CNT;
            offsets[0]=rgbOrder.Find('R');
            offsets[1]=rgbOrder.Find('G');
            offsets[2]=rgbOrder.Find('B');
        }

        // only for use in initializing the custom model
        void AddBufCoord(wxCoord x, wxCoord y)
        {
            CoordStruct c;
            c.bufX=x;
            c.bufY=y;
            Coords.push_back(c);
        }

        void SetColor(const xlColor& color)
        {
            c[0]=color.red;
            c[1]=color.green;
            c[2]=color.blue;
        }

        void SetColor(uint8_t r,uint8_t g,uint8_t b)
        {
            c[0]=r;
            c[1]=g;
            c[2]=b;
        }
        
        virtual void SetFromChannels(const unsigned char *buf) {
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                    c[x] = buf[offsets[x]];
                }
            }
        }
        virtual void GetForChannels(unsigned char *buf) {
            for (int x = 0; x < 3; x++) {
                if (offsets[x] != 255) {
                     buf[offsets[x]] = c[x];
                }
            }
        }
        
        int GetChanCount()
        {
            return chanCnt;
        }
        bool IsVisible()
        {
            return Coords.size() > 0;
        }

        bool OrigCoordsSaved()
        {
            return Coords.size() == OrigCoords.size();
        }
        void SaveCoords()
        {
            OrigCoords = Coords;
        }

        virtual ~NodeBaseClass()
        {
        }

        virtual void GetColor(xlColor& color)
        {
            color.Set(c[0],c[1],c[2]);
        }
    };

    class NodeClassRed : public NodeBaseClass
    {
    public:
        NodeClassRed(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
            offsets[0] = 0;
            offsets[1] = offsets[2] = 255;
        }
        virtual void GetColor(xlColor& color)
        {
            color.Set(c[0],0,0);
        }
    };

    class NodeClassGreen : public NodeBaseClass
    {
    public:
        NodeClassGreen(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
            offsets[1] = 0;
            offsets[0] = offsets[2] = 255;
        }
        virtual void GetColor(xlColor& color)
        {
            color.Set(0,c[1],0);
        }
    };

    class NodeClassBlue : public NodeBaseClass
    {
    public:
        NodeClassBlue(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
            offsets[2] = 0;
            offsets[0] = offsets[1] = 255;
        }
        virtual void GetColor(xlColor& color)
        {
            color.Set(0,0,c[2]);
        }
    };

    class NodeClassWhite : public NodeBaseClass
    {
    public:
        NodeClassWhite(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        }

        virtual void GetColor(xlColor& color)
        {
            uint8_t cmin=std::min(c[0],std::min(c[1],c[2]));
            color.Set(cmin,cmin,cmin);
        }
        virtual void SetFromChannels(const char *buf) {
            c[0] = c[1] = c[2] = buf[0];
        }
        virtual void GetForChannels(char *buf) {
            buf[0] = std::min(c[0],std::min(c[1],c[2]));
        }
    };
    class NodeClassRGBW : public NodeBaseClass
    {
    public:
        NodeClassRGBW(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_RGBW_CHAN_CNT;
        }
        virtual void SetFromChannels(const char *buf) {
            if (buf[3] != 0) {
                c[0] = c[1] = c[2] = buf[3];
            } else {
                for (int x = 0; x < 3; x++) {
                    if (offsets[x] != 255) {
                        c[x] = buf[offsets[x]];
                    }
                }
            }
        }
        virtual void GetForChannels(char *buf) {
            if (c[0] == c[1] && c[1] == c[2]) {
                buf[0] = buf[1] = buf[2] = 0;
                buf[3] = c[0];
            } else {
                for (int x = 0; x < 3; x++) {
                    if (offsets[x] != 255) {
                        c[x] = buf[offsets[x]];
                    }
                }
            }
        }
    };

    typedef std::unique_ptr<NodeBaseClass> NodeBaseClassPtr;

    void InitVMatrix(int firstExportStrand = 0);
    void InitHMatrix();
    void InitLine();
    void InitFrame();
    void InitStar();
    void InitWreath();
    void InitWholeHouse(wxString);

    void SetBufferSize(int NewHt, int NewWi);
    void SetRenderSize(int NewHt, int NewWi);
    void SetNodeCount(size_t NumStrings, size_t NodesPerString, const wxString &rgbOrder);
    void CopyBufCoord2ScreenCoord();
    void SetTreeCoord(long degrees);
    void SetLineCoord();
    void SetArchCoord();
    int GetCustomMaxChannel(const wxString& customModel);
    void InitCustomMatrix(const wxString& customModel);
    double toRadians(long degrees);
    long toDegrees(double radians);
    void TranslatePoint(double radians,wxCoord x,wxCoord y,wxCoord* x1,wxCoord* y1);

    wxString DisplayAs;  // Tree 360, Tree 270, Tree 180, Tree 90, Vert Matrix, Horiz Matrix, Single Line, Arches, Window Frame
    wxString StringType; // RGB Nodes, 3 Channel RGB, Single Color Red, Single Color Green, Single Color Blue, Single Color White
    long parm1;         /**< Number of strings in the model (except for frames & custom) */
    long parm2;         /**< Number of nodes per string in the model (except for frames & custom) */
    long parm3;         /**< Number of strands per string in the model (except for frames & custom) */
    std::vector<int> starSizes;
    bool IsLtoR;         // true=left to right, false=right to left
    bool isBotToTop;
    long Antialias;      // amount of anti-alias (0,1,2)
    int AliasFactor;     // factor to expand buffer (2 ^ Antialias)

    int TreeDegrees,FrameWidth;
    bool modelv2;
    int StrobeRate;      // 0=no strobing
    double offsetXpct,offsetYpct;
    double PreviewScale;
    int PreviewRotation;
    bool SingleNode;     // true for dumb strings and single channel strings
    bool SingleChannel;  // true for traditional single-color strings
    long ModelVersion;

    int mMinScreenX;
    int mMinScreenY;
    int mMaxScreenX;
    int mMaxScreenY;
    wxPoint mHandlePosition[5];
    int mDragMode;
    int mLastResizeX;
    StartChannelVector_t stringStartChan;
    wxXmlNode* ModelXml;
    wxString rgbOrder;

protected:
    std::vector<NodeBaseClassPtr> Nodes;

public:
    wxString name;       // user-designated model name
    int BufferHt,BufferWi;  // size of the buffer
    int RenderHt,RenderWi;  // size of the rendered output
    bool MyDisplay;
    long ModelBrightness;   // Value from -100 to +100 indicates an adjustment to brightness for this model
    bool Selected=false;
    bool GroupSelected=false;
    void SetFromXml(wxXmlNode* ModelNode, bool zeroBased=false);
    size_t GetNodeCount();
    int GetChanCount();
    int GetChanCountPerNode();
    size_t GetCoordCount(size_t nodenum);
    void UpdateXmlWithScale();
    void SetOffset(double xPct, double yPct);
    void AddOffset(double xPct, double yPct);
    void SetScale(double newscale);
    double GetScale();
    int GetPreviewRotation();
    int GetLastChannel();
    int GetNodeNumber(size_t nodenum);
    wxXmlNode* GetModelXml();
    wxCursor GetResizeCursor(int cornerIndex);
    void DisplayModelOnWindow(ModelPreview* preview);
    void DisplayModelOnWindow(wxWindow* window);
    void DisplayModelOnWindow(ModelPreview* preview, const xlColour &color);
    void DisplayEffectOnWindow(SequencePreview* preview, double pointSize);
    void ResizeWithHandles(ModelPreview* preview, int mouseX,int mouseY);
    void RotateWithHandles(ModelPreview* preview,bool ShiftKeyPressed,  int mouseX,int mouseY);
    bool HitTest(ModelPreview* preview,int x,int y);
    bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2);
    void AddToWholeHouseModel(ModelPreview* preview,std::vector<int>& xPos,std::vector<int>& yPos,std::vector<int>& actChannel);
    void SetMinMaxModelScreenCoordinates(ModelPreview* preview);
    bool CanRotate();
    void Rotate(int degrees);
    const wxString& GetStringType(void) { return StringType; }
    const wxString& GetDisplayAs(void) { return DisplayAs; }
    int NodesPerString();
    void SetModelCoord(int degrees);
    int CheckIfOverHandles(ModelPreview* preview, wxCoord x,wxCoord y);
    int GetRotation();
    int ChannelsPerNode();
    int NodeStartChannel(size_t nodenum);
    
    void GetNodeChannelValues(size_t nodenum, unsigned char *buf);
    void SetNodeChannelValues(size_t nodenum, unsigned char *buf);
    wxChar GetChannelColorLetter(wxByte chidx);
    
    wxString ChannelLayoutHtml();
//    int FindChannelAt(int x, int y);
//    wxSize GetChannelCoords(std::vector<std::vector<int>>& chxy, bool shrink); //for pgo RenderFaces
    bool IsCustom(void);
    size_t GetChannelCoords(wxArrayString& choices); //wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3);
    static bool ParseFaceElement(const wxString& str, std::vector<wxPoint>& first_xy);
//    int FindChannelAtXY(int x, int y, const wxString& model);
    static ModelClass* FindModel(const wxString& name);
    static size_t EnumModels(wxArrayString* choices, const wxString& InactivePrefix);
    wxString GetNodeXY(const wxString& nodenumstr);
    wxString GetNodeXY(int nodeinx);

    void SetTop(ModelPreview* preview,int y);
    void SetBottom(ModelPreview* preview,int y);
    void SetLeft(ModelPreview* preview,int x);
    void SetRight(ModelPreview* preview,int x);
    void SetHcenterOffset(float offset);
    void SetVcenterOffset(float offset);

    int GetTop(ModelPreview* preview);
    int GetBottom(ModelPreview* preview);
    int GetLeft(ModelPreview* preview);
    int GetRight(ModelPreview* preview);
    float GetHcenterOffset();
    float GetVcenterOffset();

    long GetNumArches()
    {
        if (DisplayAs == wxT("Arches"))
            return parm1;
        else
            return 0;
    }

    long GetNodesPerArch()
    {
        if (DisplayAs == wxT("Arches"))
            return parm2;
        else
            return 0;
    }

    static bool IsMyDisplay(wxXmlNode* ModelNode)
    {
        return ModelNode->GetAttribute(wxT("MyDisplay"),wxT("0")) == wxT("1");
    }
    static void SetMyDisplay(wxXmlNode* ModelNode,bool NewValue)
    {
        ModelNode->DeleteAttribute(wxT("MyDisplay"));
        ModelNode->AddAttribute(wxT("MyDisplay"), NewValue ? wxT("1") : wxT("0"));
    }
    static wxString StartChanAttrName(int idx)
    {
        return wxString::Format(wxT("String%d"),idx+1);
    }
    // returns true for models that only have 1 string and where parm1 does NOT represent the # of strings
    static bool HasOneString(const wxString& DispAs)
    {
        return (DispAs == wxT("Window Frame") || DispAs == wxT("Custom"));
    }
    // true for dumb strings and traditional strings
    static bool HasSingleNode(const wxString& StrType)
    {
        return !StrType.EndsWith(wxT(" Nodes"));
    }
    // true for traditional strings
    static bool HasSingleChannel(const wxString& StrType)
    {
        return StrType.StartsWith(wxT("Single")) || StrType.StartsWith(wxT("Strobe"));
    }

};
typedef std::unique_ptr<ModelClass> ModelClassPtr;

#endif // MODELCLASS_H
