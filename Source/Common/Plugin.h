/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Scheduler functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef PLUGINH
#define PLUGINH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <map>
#if defined(_WIN32)
#include <windows.h>
#endif
#include "MediaConchLib.h"
#include "Container.h"

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// Class Plugin
//***************************************************************************

class Plugin
{
public:
    Plugin();
    virtual ~Plugin();

    virtual int load_plugin(const std::map<std::string, Container::Value>& obj, std::string& error) = 0;
    virtual int run(std::string& error) = 0;

    MediaConchLib::PluginType get_type() const { return type; }
    const std::string&        get_name() const { return name; }
    const std::string&        get_report() const { return report; }
    const std::string&        get_error() const { return error; }

protected:
    MediaConchLib::PluginType type;
    std::string               name;
    std::string               report;
    std::string               error;
    int                       exec_bin(const std::vector<std::string>& params, std::string& error);
    int                       read_report(const std::string& file, std::string& report);

#if defined(_WIN32)
    int                       create_pipe(HANDLE* handler_out_rd, HANDLE* handler_out_wr);
    int                       execute_the_command(std::string& cmd, HANDLE handler_out_wr);
    int                       read_the_stdout(HANDLE handler_out_wr, HANDLE handler_out_rd);
#endif

    void                      unified_string(std::string& str);

private:
    Plugin(const Plugin&);
    Plugin&     operator=(const Plugin&);
};

}

#endif // !PLUGINH
