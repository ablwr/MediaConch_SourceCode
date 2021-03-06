/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Queue functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef QueueH
#define QueueH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifdef MEDIAINFO_DLL_RUNTIME
    #include "MediaInfoDLL/MediaInfoDLL.h"
    #define MediaInfoNameSpace MediaInfoDLL
#elif defined MEDIAINFO_DLL_STATIC
    #include "MediaInfoDLL/MediaInfoDLL_Static.h"
    #define MediaInfoNameSpace MediaInfoDLL
#else
    #include "MediaInfo/MediaInfo.h"
    #define MediaInfoNameSpace MediaInfoLib
#endif
#include "ZenLib/Thread.h"
#include "ZenLib/CriticalSection.h"
//---------------------------------------------------------------------------
#include <map>
#include <list>
#include <vector>

namespace MediaConch
{
    //***************************************************************************
    // Class QueueElement
    //***************************************************************************

    class Queue;
    class Scheduler;

    class QueueElement : public ZenLib::Thread
    {
    public:
        QueueElement(Scheduler *s);
        virtual ~QueueElement();
        int                                id;
        std::string                        filename;
        std::map<std::string, std::string> options;

        void                               Entry();
        void                               stop();
        double                             percent_done();

    private:
        Scheduler*                         scheduler;
        MediaInfoNameSpace::MediaInfo     *MI;
    };

    //***************************************************************************
    // Enum priority
    //***************************************************************************

    enum QueuePriority
    {
        PRIORITY_NONE,
        PRIORITY_LOW,
        PRIORITY_MEDIUM,
        PRIORITY_HIGH,
    };

    //***************************************************************************
    // Class Queue
    //***************************************************************************

    class Queue
    {
    public:
        Queue(Scheduler *s) : scheduler(s){}
        ~Queue();

        int add_element(QueuePriority priority, int id, const std::string& filename, const std::vector<std::string>& options);
        bool has_element(const std::string& filename);
        int remove_element(int id);
        int remove_elements(const std::string& filename);
        void clear();

        QueueElement* run_next();
        size_t queue_size() const { return queue.size(); }

    private:
        std::map<QueuePriority, std::list<QueueElement*> > queue;
        Scheduler *scheduler;
    };
}

#endif
