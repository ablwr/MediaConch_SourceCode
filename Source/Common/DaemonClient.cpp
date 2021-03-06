/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

//---------------------------------------------------------------------------
#include "DaemonClient.h"
#include "MediaConchLib.h"
#include "XsltPolicy.h"
#include "Http.h"
#include "LibEventHttp.h"
#include "REST_API.h"
#ifdef _WIN32
#include <ZenLib/Ztring.h>
#include <Winsock2.h>
#endif //_WIN32
#ifdef _WIN32
#else
#include <stdlib.h>
#endif //_WIN32

//---------------------------------------------------------------------------
namespace MediaConch {

//***************************************************************************
// DaemonClient
//***************************************************************************

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
DaemonClient::DaemonClient(MediaConchLib* m) : mcl(m), http_client(NULL)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        printf("WSAStartup failed\n");
    }
#endif //_WIN32
}

//---------------------------------------------------------------------------
DaemonClient::~DaemonClient()
{
#ifdef _WIN32
    WSACleanup();
#endif //_WIN32
}

//---------------------------------------------------------------------------
int DaemonClient::init()
{
#ifdef HAVE_LIBEVENT
    http_client = new LibEventHttp;
#else
    return -1;
#endif

    std::string server;
    int port = -1;
    mcl->get_daemon_address(server, port);

    if (server.length() != 0)
        http_client->set_address(server);
    if (port != -1)
        http_client->set_port(port);

    http_client->init();

    return 0;
}

//---------------------------------------------------------------------------
int DaemonClient::close()
{
    if (http_client)
    {
        delete http_client;
        http_client = NULL;
    }
    return 0;
}

//---------------------------------------------------------------------------
void DaemonClient::reset()
{
    if (http_client)
        http_client->reset_daemon_id();
}

//***************************************************************************
// Checker
//***************************************************************************

//---------------------------------------------------------------------------
int DaemonClient::checker_list(std::vector<std::string>& vec)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Checker_List_Req req;

    int ret = http_client->start();
    if (ret < 0)
        return ret;
    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Checker_List_Res *res = rest.parse_list_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    for (size_t i = 0; i < res->files.size(); ++i)
        vec.push_back(res->files[i]->file);

    delete res;
    return MediaConchLib::errorHttp_NONE;
}

//---------------------------------------------------------------------------
int DaemonClient::checker_file_from_id(int id, std::string& filename)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Checker_File_From_Id_Req req;
    req.id = id;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Checker_File_From_Id_Res *res = rest.parse_file_from_id_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    filename = res->file;
    delete res;
    return MediaConchLib::errorHttp_NONE;
}

//---------------------------------------------------------------------------
int DaemonClient::default_values_for_type(const std::string& type, std::vector<std::string>& values)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Default_Values_For_Type_Req req;
    req.type = type;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Default_Values_For_Type_Res *res = rest.parse_default_values_for_type_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    for (size_t i = 0; i < res->values.size(); ++i)
        values.push_back(res->values[i]);
    delete res;
    return MediaConchLib::errorHttp_NONE;
}

//---------------------------------------------------------------------------
int DaemonClient::checker_analyze(const std::string& file, bool& registered, bool force_analyze)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Checker_Analyze_Req req;
    RESTAPI::Checker_Analyze_Arg arg;

    arg.id = 0;

    std::string real_file(file);
#ifdef _WIN32
    ZenLib::Ztring path = ZenLib::Ztring().From_UTF8(real_file);

    DWORD path_size = GetFullPathName(path.c_str(), 0, NULL, NULL);
    ZenLib::Char* tmp = new ZenLib::Char[path_size + 1];
    if (GetFullPathName(path.c_str(), path_size + 1, tmp, NULL))
    {
        path = ZenLib::Ztring(tmp);
        real_file = path.To_UTF8();
    }
    delete [] tmp;
#else
    char *path = realpath(file.c_str(), NULL);
    if (path)
    {
        real_file = std::string(path);
        free(path);
    }
#endif //_WIN32

    arg.file = real_file;
    if (force_analyze)
    {
        arg.has_force_analyze = true;
        arg.force_analyze = force_analyze;
    }
    req.args.push_back(arg);

    int ret = http_client->start();
    if (ret < 0)
        return ret;
    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Checker_Analyze_Res *res = rest.parse_analyze_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    if (res->ok.size() != 1)
    {
        delete res;
        return MediaConchLib::errorHttp_INVALID_DATA;
    }

    registered = !res->ok[0]->create;

    file_ids[file] = res->ok[0]->outId;
    delete res;
    return MediaConchLib::errorHttp_NONE;
}

//---------------------------------------------------------------------------
int DaemonClient::checker_is_done(const std::string& file, double& done, MediaConchLib::report& report_kind)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Checker_Status_Req req;
    std::map<std::string, int>::iterator it = file_ids.find(file);
    if (it == file_ids.end())
        return MediaConchLib::errorHttp_MAX;
    req.ids.push_back(it->second);

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return MediaConchLib::errorHttp_INVALID_DATA;

    RESTAPI rest;
    RESTAPI::Checker_Status_Res *res = rest.parse_status_res(data);
    if (!res || res->ok.size() != 1)
        return MediaConchLib::errorHttp_INVALID_DATA;

    RESTAPI::Checker_Status_Ok *ok = res->ok[0];

    if (ok->finished)
    {
        report_kind = MediaConchLib::report_MediaConch;
        if (ok->has_tool)
        {
            if (ok->tool == RESTAPI::VERAPDF)
                report_kind = MediaConchLib::report_MediaVeraPdf;
            else if (ok->tool == RESTAPI::DPFMANAGER)
                report_kind = MediaConchLib::report_MediaDpfManager;
        }
        delete res;
        return MediaConchLib::errorHttp_TRUE;
    }

    if (ok->has_percent)
        done = ok->done;
    else
        done = 0.0;

    report_kind = MediaConchLib::report_MediaConch;

    delete res;
    return MediaConchLib::errorHttp_NONE;
}

//---------------------------------------------------------------------------
int DaemonClient::checker_get_report(int user, const std::bitset<MediaConchLib::report_Max>& report_set,
                                     MediaConchLib::format f, const std::vector<std::string>& files,
                                     const std::vector<size_t>& policies_ids,
                                     const std::vector<std::string>& policies_contents,
                                     const std::map<std::string, std::string>& options,
                                     MediaConchLib::Checker_ReportRes* result,
                                     const std::string* display_name,
                                     const std::string* display_content)
{
    if (!http_client)
        return -1;

    // FILE
    RESTAPI::Checker_Report_Req req;
    req.user = user;

    for (size_t i = 0; i < files.size(); ++i)
    {
        std::map<std::string, int>::iterator it = file_ids.find(files[i]);
        if (it == file_ids.end())
            continue;
        req.ids.push_back(it->second);
    }

    // REPORT KIND
    if (report_set[MediaConchLib::report_MediaConch])
        req.reports.push_back(RESTAPI::IMPLEMENTATION);
    if (report_set[MediaConchLib::report_MediaInfo])
        req.reports.push_back(RESTAPI::MEDIAINFO);
    if (report_set[MediaConchLib::report_MediaTrace])
        req.reports.push_back(RESTAPI::MEDIATRACE);
    if (report_set[MediaConchLib::report_MediaVeraPdf])
        req.reports.push_back(RESTAPI::VERAPDF);
    if (report_set[MediaConchLib::report_MediaDpfManager])
        req.reports.push_back(RESTAPI::DPFMANAGER);

    // POLICY
    if (policies_ids.size())
    {
        req.reports.push_back(RESTAPI::POLICY);
        for (size_t i = 0; i < policies_ids.size(); ++i)
            req.policies_ids.push_back(policies_ids[i]);
    }
    if (policies_contents.size())
    {
        req.reports.push_back(RESTAPI::POLICY);
        for (size_t i = 0; i < policies_contents.size(); ++i)
            req.policies_contents.push_back(policies_contents[i]);
    }

    // FORMAT
    if (f == MediaConchLib::format_Xml)
    {
        if (display_name)
            req.display_name = *display_name;
        else if (!display_content)
            req.display_name = MediaConchLib::display_xml_name;
    }
    else if (f == MediaConchLib::format_MaXml)
        req.display_name = MediaConchLib::display_maxml_name;
    else if (f == MediaConchLib::format_Text)
        req.display_name = MediaConchLib::display_text_name;
    else if (f == MediaConchLib::format_Html)
        req.display_name = MediaConchLib::display_html_name;
    else if (f == MediaConchLib::format_JsTree)
        req.display_name = MediaConchLib::display_jstree_name;

    if (display_content)
        req.display_content = *display_content;

    std::map<std::string, std::string>::const_iterator it = options.begin();
    for (; it != options.end(); ++it)
    {
        if (!it->first.size())
            continue;
        req.options[it->first.c_str()] = it->second;
    }

    http_client->start();
    if (http_client->send_request(req) < 0)
        return -1;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return -1;

    RESTAPI rest;
    RESTAPI::Checker_Report_Res *res = rest.parse_report_res(data);
    if (!res)
        return -1;

    if (!res->ok.report.length())
    {
        delete res;
        return -1;
    }

    result->report = res->ok.report;

    result->has_valid = res->ok.has_valid;
    result->valid = res->ok.valid;

    delete res;
    return 0;
}

//---------------------------------------------------------------------------
int DaemonClient::checker_validate(int user, MediaConchLib::report report,
                                   const std::vector<std::string>& files,
                                   const std::vector<size_t>& policies_ids,
                                   const std::vector<std::string>& policies_contents,
                                   const std::map<std::string, std::string>& options,
                                   std::vector<MediaConchLib::Checker_ValidateRes*>& result)
{
    if (!http_client)
        return -1;

    // FILE
    RESTAPI::Checker_Validate_Req req;
    req.user = user;

    std::map<int, std::string> saved_ids;
    for (size_t i = 0; i < files.size(); ++i)
    {
        std::map<std::string, int>::iterator it = file_ids.find(files[i]);
        if (it == file_ids.end())
            continue;
        req.ids.push_back(it->second);
        saved_ids[it->second] = files[i];
    }

    // REPORT KIND
    if (report == MediaConchLib::report_MediaConch)
        req.report = RESTAPI::IMPLEMENTATION;
    else if (report == MediaConchLib::report_MediaVeraPdf)
        req.report = RESTAPI::VERAPDF;
    else if (report == MediaConchLib::report_MediaDpfManager)
        req.report = RESTAPI::DPFMANAGER;
    else if (policies_ids.size())
    {
        // POLICY
        req.report = RESTAPI::POLICY;
        for (size_t i = 0; i < policies_ids.size(); ++i)
            req.policies_ids.push_back(policies_ids[i]);
    }
    else if (policies_contents.size())
    {
        // POLICY
        req.report = RESTAPI::POLICY;
        for (size_t i = 0; i < policies_contents.size(); ++i)
            req.policies_contents.push_back(policies_contents[i]);
    }
    else
        return -1;

    std::map<std::string, std::string>::const_iterator it = options.begin();
    for (; it != options.end(); ++it)
    {
        if (!it->first.size())
            continue;
        req.options[it->first.c_str()] = it->second;
    }

    http_client->start();
    if (http_client->send_request(req) < 0)
        return -1;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return -1;

    RESTAPI rest;
    RESTAPI::Checker_Validate_Res *res = rest.parse_validate_res(data);
    if (!res)
        return -1;

    if (!res->ok.size())
    {
        delete res;
        return -1;
    }

    for (size_t i = 0; i < res->ok.size(); ++i)
    {
        MediaConchLib::Checker_ValidateRes* v = new MediaConchLib::Checker_ValidateRes;
        v->file = saved_ids[res->ok[i]->id];
        v->valid = res->ok[i]->valid;
        result.push_back(v);
    }

    delete res;
    return 0;
}

//***************************************************************************
// Policy
//***************************************************************************

//---------------------------------------------------------------------------
int DaemonClient::xslt_policy_create(int user, const std::string& type, int parent_id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::XSLT_Policy_Create_Req req;
    req.user = user;
    req.parent_id = parent_id;
    req.type = type;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::XSLT_Policy_Create_Res *res = rest.parse_xslt_policy_create_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = res->id;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_import(int user, const std::string& memory, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Import_Req req;
    req.xml = memory;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Import_Res *res = rest.parse_policy_import_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = res->id;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_duplicate(int user, int id, int dst_policy_id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Duplicate_Req req;
    req.id = id;
    req.dst_policy_id = dst_policy_id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Duplicate_Res *res = rest.parse_policy_duplicate_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = res->id;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_move(int user, int id, int dst_policy_id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Move_Req req;
    req.id = id;
    req.dst_policy_id = dst_policy_id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Move_Res *res = rest.parse_policy_move_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = res->id;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_remove(int user, int id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Remove_Req req;
    req.id = id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Remove_Res *res = rest.parse_policy_remove_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = 0;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_save(int user, int id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Save_Req req;
    req.id = id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Save_Res *res = rest.parse_policy_save_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = 0;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_dump(int user, int id, std::string& memory, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Dump_Req req;
    req.id = id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Dump_Res *res = rest.parse_policy_dump_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
    {
        ret = 0;
        memory = res->xml;
    }
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_change_info(int user, int id, const std::string& name, const std::string& description, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Change_Info_Req req;
    req.id = id;
    req.name = name;
    req.description = description;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Change_Info_Res *res = rest.parse_policy_change_info_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = 0;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_change_type(int user, int id, const std::string& type, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Change_Type_Req req;
    req.id = id;
    req.type = type;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Change_Type_Res *res = rest.parse_policy_change_type_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = 0;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_get(int user, int id, const std::string& format, MediaConchLib::Get_Policy& policy, std::string& err)
{
    if (!http_client)
        return -1;

    RESTAPI::Policy_Get_Req req;
    req.id = id;
    req.user = user;
    req.format = format;

    int ret = http_client->start();
    if (ret < 0)
        return -1;

    ret = http_client->send_request(req);
    if (ret < 0)
        return -1;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return -1;

    RESTAPI rest;
    RESTAPI::Policy_Get_Res *res = rest.parse_policy_get_res(data);
    if (!res)
        return -1;

    if (res->nok)
    {
        err = res->nok->error;
        delete res;
        return -1;
    }
    else if (res->policy)
    {
        policy.format = format;
        policy.policy = new MediaConchLib::Policy_Policy(res->policy);
    }
    else if (res->policyTree.size())
    {
        policy.jstree = new std::string;
        *policy.jstree = res->policyTree;
        policy.format = format;
    }

    delete res;
    res = NULL;
    return 0;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_get_name(int user, int id, std::string& name, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Get_Name_Req req;
    req.id = id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Get_Name_Res *res = rest.parse_policy_get_name_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
    {
        ret = 0;
        name = res->name;
    }
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
void DaemonClient::policy_get_policies(int user, const std::vector<int>& ids, const std::string& format, MediaConchLib::Get_Policies& policies)
{
    if (!http_client)
        return;

    RESTAPI::Policy_Get_Policies_Req req;
    req.user = user;
    req.ids = ids;
    req.format = format;

    int ret = http_client->start();
    if (ret < 0)
        return;

    ret = http_client->send_request(req);
    if (ret < 0)
        return;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return;

    RESTAPI rest;
    RESTAPI::Policy_Get_Policies_Res *res = rest.parse_policy_get_policies_res(data);
    if (!res)
        return;

    if (!res->nok)
    {
        policies.format = format;
        if (format == "JSTREE")
        {
            policies.jstree = new std::string;
            *policies.jstree = res->policiesTree;
        }
        else
        {
            policies.policies = new std::vector<MediaConchLib::Policy_Policy*>;
            for (size_t i = 0; i < res->policies.size(); ++i)
            {
                if (!res->policies[i])
                    continue;

                MediaConchLib::Policy_Policy* p = new MediaConchLib::Policy_Policy(res->policies[i]);
                policies.policies->push_back(p);
            }
        }
    }

    delete res;
    res = NULL;
}

//---------------------------------------------------------------------------
void DaemonClient::policy_get_policies_names_list(int user, std::vector<std::pair<int, std::string> >& policies)
{
    if (!http_client)
        return;

    RESTAPI::Policy_Get_Policies_Names_List_Req req;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return;

    ret = http_client->send_request(req);
    if (ret < 0)
        return;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return;

    RESTAPI rest;
    RESTAPI::Policy_Get_Policies_Names_List_Res *res = rest.parse_policy_get_policies_names_list_res(data);
    if (!res)
        return;

    if (!res->nok)
        policies = res->policies;

    delete res;
    res = NULL;
}

//---------------------------------------------------------------------------
size_t DaemonClient::policy_get_policies_count(int user)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Get_Policies_Count_Req req;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Get_Policies_Count_Res *res = rest.parse_policy_get_policies_count_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = res->size;

    delete res;
    res = NULL;
    return (size_t)ret;
}

//---------------------------------------------------------------------------
int DaemonClient::policy_clear_policies(int user, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::Policy_Clear_Policies_Req req;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::Policy_Clear_Policies_Res *res = rest.parse_policy_clear_policies_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = 0;
    else
        err = res->nok->error;

    delete res;
    res = NULL;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::xslt_policy_create_from_file(int user, const std::string& file)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    std::map<std::string, int>::iterator it = file_ids.find(file);
    if (it == file_ids.end())
        return MediaConchLib::errorHttp_MAX;
    int id = it->second;

    RESTAPI::XSLT_Policy_Create_From_File_Req req;
    req.id = id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::XSLT_Policy_Create_From_File_Res *res = rest.parse_xslt_policy_create_from_file_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    int policy_id = res->policy_id;
    delete res;
    return policy_id;
}

//---------------------------------------------------------------------------
int DaemonClient::xslt_policy_rule_create(int user, int policy_id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::XSLT_Policy_Rule_Create_Req req;
    req.policy_id = policy_id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::XSLT_Policy_Rule_Create_Res *res = rest.parse_xslt_policy_rule_create_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    int rule = -1;
    if (!res->nok)
        rule = res->id;
    else
        err = res->nok->error;
    delete res;
    return rule;
}

//---------------------------------------------------------------------------
XsltPolicyRule *DaemonClient::xslt_policy_rule_get(int user, int policy_id, int id, std::string& err)
{
    if (!http_client)
        return NULL;

    RESTAPI::XSLT_Policy_Rule_Get_Req req;
    req.policy_id = policy_id;
    req.id = id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return NULL;

    ret = http_client->send_request(req);
    if (ret < 0)
        return NULL;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return NULL;

    RESTAPI rest;
    RESTAPI::XSLT_Policy_Rule_Get_Res *res = rest.parse_xslt_policy_rule_get_res(data);
    if (!res)
        return NULL;

    XsltPolicyRule *rule = NULL;
    if (!res->nok)
    {
        rule = new XsltPolicyRule;
        rule->id = res->rule.id;
        rule->node_name = res->rule.name;
        rule->track_type = res->rule.tracktype;
        rule->field = res->rule.field;
        rule->scope = res->rule.scope;
        rule->occurrence = res->rule.occurrence;
        rule->ope = rule->ope;
        rule->value = res->rule.value;
    }
    else
        err = res->nok->error;
    delete res;
    return rule;
}

//---------------------------------------------------------------------------
int DaemonClient::xslt_policy_rule_edit(int user, int policy_id, int rule_id, const XsltPolicyRule *rule, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::XSLT_Policy_Rule_Edit_Req req;
    req.policy_id = policy_id;

    req.rule.id = rule_id;
    req.rule.name = rule->node_name;
    req.rule.tracktype = rule->track_type;
    req.rule.field = rule->field;
    req.rule.scope = rule->scope;
    req.rule.occurrence = rule->occurrence;
    req.rule.ope = rule->ope;
    req.rule.value = rule->value;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::XSLT_Policy_Rule_Edit_Res *res = rest.parse_xslt_policy_rule_edit_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = 0;
    else
        err = res->nok->error;
    delete res;
    return ret;
}

//---------------------------------------------------------------------------
int DaemonClient::xslt_policy_rule_duplicate(int user, int policy_id, int rule_id, int dst_policy_id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::XSLT_Policy_Rule_Duplicate_Req req;
    req.policy_id = policy_id;
    req.dst_policy_id = dst_policy_id;
    req.id = rule_id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::XSLT_Policy_Rule_Duplicate_Res *res = rest.parse_xslt_policy_rule_duplicate_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    int rule = -1;
    if (!res->nok)
        rule = res->id;
    else
        err = res->nok->error;
    delete res;
    return rule;
}

//---------------------------------------------------------------------------
int DaemonClient::xslt_policy_rule_move(int user, int policy_id, int rule_id, int dst_policy_id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::XSLT_Policy_Rule_Move_Req req;
    req.policy_id = policy_id;
    req.dst_policy_id = dst_policy_id;
    req.id = rule_id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::XSLT_Policy_Rule_Move_Res *res = rest.parse_xslt_policy_rule_move_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    int rule = -1;
    if (!res->nok)
        rule = res->id;
    else
        err = res->nok->error;
    delete res;
    return rule;
}

//---------------------------------------------------------------------------
int DaemonClient::xslt_policy_rule_delete(int user, int policy_id, int rule_id, std::string& err)
{
    if (!http_client)
        return MediaConchLib::errorHttp_INIT;

    RESTAPI::XSLT_Policy_Rule_Delete_Req req;
    req.policy_id = policy_id;
    req.id = rule_id;
    req.user = user;

    int ret = http_client->start();
    if (ret < 0)
        return ret;

    ret = http_client->send_request(req);
    if (ret < 0)
        return ret;

    std::string data = http_client->get_result();
    http_client->stop();
    if (!data.length())
        return http_client->get_error();

    RESTAPI rest;
    RESTAPI::XSLT_Policy_Rule_Delete_Res *res = rest.parse_xslt_policy_rule_delete_res(data);
    if (!res)
        return MediaConchLib::errorHttp_INVALID_DATA;

    ret = -1;
    if (!res->nok)
        ret = 0;
    else
        err = res->nok->error;
    delete res;
    return ret;
}

}
