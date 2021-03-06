#ifndef WAVEFORM_H
#define WAVEFORM_H

#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "wx/glcanvas.h"
#endif

#include "mpg123.h"
#include <vector>

#define VERTICAL_PADDING            10
#define NO_WAVE_VIEW_SELECTED       -1
#define WAVEFORM_SIDE_MARGIN        25
wxDECLARE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);
wxDECLARE_EVENT(EVT_TIME_SELECTED, wxCommandEvent);
wxDECLARE_EVENT(EVT_ZOOM, wxCommandEvent);

class TimeLine;

class Waveform : public wxGLCanvas
{
    public:

        static int GetLengthOfMusicFileInMS(const char* filename);
        static int GetTrackSize(mpg123_handle *mh,int bits, int channels);

        int OpenfileMediaFile(const char* filename);
        void CloseMediaFile();

        void SetZoomLevel(int level);
        int GetZoomLevel();

        int SetStartPixelOffset(int startPixel);
        int GetStartPixelOffset();

        void SetShadedRegion(int x1,int x2);
        void GetShadedRegion(int* x1, int* x2);

        void SetTimeFrequency(int startPixel);
        int GetTimeFrequency();

        void SetTimeline(TimeLine* timeLine);

        void PositionSelected(int x);

        Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,long style=0, const wxString &name=wxPanelNameStr);
        virtual ~Waveform();


        struct MINMAX
        {
            float min;
            float max;
        };



    protected:
    private:
      	DECLARE_EVENT_TABLE()
        void LoadTrackData(mpg123_handle *mh,char  * data);
        void SplitTrackDataAndNormalize(signed short* trackData,int trackSize,float* leftData,float* rightData);
        void GetMinMaxSampleSet(int setSize, float*sampleData,int trackSize, MINMAX* minMax);
        float GetSamplesPerLineFromZoomLevel(int ZoomLevel);
        wxGLContext*	m_context;
        TimeLine* mTimeline;
        wxPanel* mParent;
        wxWindow* mMainWindow;
        int mStartPixelOffset;
        int mCurrentWaveView;
        int mMediaTrackSize;
        int mFrequency;
        int mZoomLevel;
        int m_bits;
        int m_rate;
        int m_channels;
        bool mIsInitialized;
        bool mPointSize;
        bool m_dragging;
        bool m_scrolling;
        bool m_scroll_speed;
        wxTimer* tmrScrollLeft;
        wxTimer* tmrScrollRight;
        int m_shaded_region_x1;
        int m_shaded_region_x2;
        float* m_left_data;
        float* m_right_data;
        int mSelectedPosition;
        int mPaintOnIdleCounter;

        class WaveView
        {
            private:
            float mSamplesPerPixel;
            int mZoomLevel;
            public:

            std::vector<MINMAX> MinMaxs;
            WaveView(int ZoomLevel,float SamplesPerPixel, float*sampleData, int trackSize)
            {
                mZoomLevel = ZoomLevel;
                mSamplesPerPixel = SamplesPerPixel;
                SetMinMaxSampleSet(SamplesPerPixel,sampleData,trackSize);
            }


            WaveView(int ZoomLevel)
            {

            }

            virtual ~WaveView()
            {
            }

            int GetZoomLevel()
            {
                return  mZoomLevel;
            }

            void SetMinMaxSampleSet(float SamplesPerPixel, float*sampleData, int trackSize)
            {
                int sampleIndex=0;
                float minimum=1;
                float maximum=-1;
                int iSamplesPerPixel = (int)SamplesPerPixel;
                int totalMinMaxs = (int)((float)trackSize/SamplesPerPixel)+1;
                MinMaxs.clear();
                for(int i=0;i<totalMinMaxs;i++)
                {
                    // Use float calculation to minimize compounded rounding of position
                    sampleIndex = (int)((float)i*SamplesPerPixel);
                    minimum=1;
                    maximum=-1;
                    int j;
                    for(j=sampleIndex;j<sampleIndex+iSamplesPerPixel && j<trackSize;j++)
                    {
                        if(sampleData[j]< minimum)
                        {
                            minimum = sampleData[j];
                        }
                        if(sampleData[j]> maximum)
                        {
                            maximum = sampleData[j];
                        }
                    }
                    MINMAX mm;
                    mm.min = minimum;
                    mm.max = maximum;
                    MinMaxs.push_back(mm);
                    if (j>=trackSize)
                        break;
                }
                int l = MinMaxs.size();
                l++;
            }


        };

        void DrawWaveView(const WaveView &wv);
        void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
        void StartDrawing(wxDouble pointSize);
        int getWidth();
        int getHeight();
        void render( wxPaintEvent& event );
      	void mouseMoved(wxMouseEvent& event);
      	void mouseLeftDown(wxMouseEvent& event);
      	void mouseLeftUp( wxMouseEvent& event);
      	void mouseLeftWindow( wxMouseEvent& event);
      	void OnIdle(wxIdleEvent &event);
      	void OnLeftDClick(wxMouseEvent& event);
      	void OnWaveScrollLeft(wxTimerEvent& event);
      	void OnWaveScrollRight(wxTimerEvent& event);
        void OutputText(GLfloat x, GLfloat y, char *text);
        void drawString (void * font, char *s, float x, float y, float z);
        void ScrollWaveLeft(int xBasedSpeed);
        void ScrollWaveRight(int xBasedSpeed);
        void StopScrolling();
        std::vector<WaveView> views;


};

#endif // WAVEFORM_H
