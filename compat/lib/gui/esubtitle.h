#ifndef __lib_gui_subtitle_h
#define __lib_gui_subtitle_h

#include <lib/gui/ewidget.h>
#include <lib/base/object.h>
#include <lib/base/ebase.h>
#include <vector>
#include <string>

struct SubtitleTrack
{
        int type;
        int pid;
        int page_number;
        int magazine_number;
        std::string language_code;
        bool operator<(const SubtitleTrack &other) const {
                if (type != other.type) return type < other.type;
                if (pid != other.pid) return pid < other.pid;
                if (page_number != other.page_number) return page_number < other.page_number;
                return magazine_number < other.magazine_number;
        }
};

struct ePangoSubtitlePageElement
{
    gRGBA m_color;
    std::string m_text;
    ePangoSubtitlePageElement(const gRGBA &color, const char *text)
        :m_color(color), m_text(text) {}
};

struct ePangoSubtitlePage
{
    std::vector<ePangoSubtitlePageElement> m_elements;
    pts_t m_show_pts;
    int m_timeout;
    void clear() { m_elements.clear(); }
};

class iSubtitleUser: public iObject
{
public:
        virtual void setSubtitle(ePtr<gPixmap> &pixmap, eRect &where)=0;
        virtual void setSubtitle(const std::string &text)=0;
        // DreamOS compatibility
        virtual void setPage(ePangoSubtitlePage &page) {}
        virtual void destroy() {}
};

class iSubtitleOutput: public iObject
{
public:
        virtual RESULT enableSubtitles(iSubtitleUser *user, struct SubtitleTrack &track)=0;
        virtual RESULT disableSubtitles()=0;
        virtual RESULT getCachedSubtitle(struct SubtitleTrack &track)=0;
        virtual RESULT getSubtitleList(std::vector<struct SubtitleTrack> &subtitlelist)=0;
};

class eSubtitleWidget: public eWidget, public Object
{
public:
        eSubtitleWidget(eWidget *parent);
        void clearPage();
        void setPixmap(ePtr<gPixmap> &pixmap, gRegion changed, eRect dest = eRect(0, 0, 720, 576));
};

// DreamOS compatibility dummies
class iTapService: public iObject {};
class iStreamBufferInfo: public iObject {};

#endif
