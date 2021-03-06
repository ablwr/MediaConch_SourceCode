/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Core functions
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaConchLibH
#define MediaConchLibH

#include <string>
#include <vector>
#include <map>
#include <bitset>

//---------------------------------------------------------------------------
namespace MediaConch {

class Core;
class DaemonClient;
class Policy;
class XsltPolicyRule;
class Http;

#ifdef _WIN32
    const std::string Path_Separator("\\");
    /* const String WPath_Separator(__T("\\")); */
#else
    const std::string Path_Separator("/");
    /* const String WPath_Separator(__T("/")); */
#endif

//***************************************************************************
// Class MediaConchLib
//***************************************************************************

class MediaConchLib
{
public:
    //Constructor/Destructor
    MediaConchLib(bool no_daemon = false);
    ~MediaConchLib();

    //Config
    enum report
    {
        report_MediaConch = 0,
        report_MediaInfo,
        report_MediaTrace,
        report_MediaVeraPdf,
        report_MediaDpfManager,
        report_MicroMediaTrace,
        report_Max,
    };

    enum format
    {
        format_Text = 0,
        format_Xml,         // XML corresponding to only one of MediaConch, MediaInfo, MediaTrace
        format_MaXml,       // MAXML, can contain one or more of MediaConch, MediaInfo, MediaTrace
        format_JsTree,
        format_Html,
        format_Max,
    };

    enum compression
    {
        compression_None = 0,
        compression_ZLib,
        compression_Max,
    };

    enum errorHttp
    {
        errorHttp_TRUE           = 1,
        errorHttp_NONE           = 0,
        errorHttp_INVALID_DATA   = -1,
        errorHttp_INIT           = -2,
        errorHttp_CONNECT        = -3,
        errorHttp_INTERNAL       = -4,
        errorHttp_DAEMON_RESTART = -5,
        errorHttp_MAX            = -6,
    };

    enum PluginType
    {
        PLUGIN_FORMAT = 0,
        PLUGIN_MAX,
    };

    struct Checker_ReportRes
    {
        std::string           report;
        bool                  has_valid;
        bool                  valid;
        Checker_ReportRes() : has_valid(false), valid(true) {}
    };

    struct Checker_ValidateRes
    {
        std::string             file;
        bool                    valid;
        Checker_ValidateRes() : valid(true) {}
    };

    union XSLT_Child;

    struct XSLT_Policy_Rule
    {
        XSLT_Policy_Rule() : id(-1), occurrence(-1) {}

        int          id;
        std::string  name;
        std::string  tracktype;
        std::string  field;
        std::string  scope;
        int          occurrence;
        std::string  ope;
        std::string  value;
        std::string  to_str() const;
    };

    struct Policy_Policy
    {
        Policy_Policy() : id(-1), parent_id(-1), is_system(false) {}
        Policy_Policy(const Policy_Policy* p) : id(p->id), parent_id(p->parent_id), is_system(p->is_system), kind(p->kind), type(p->type), name(p->name), description(p->description), children(p->children) {}
        int                                       id;
        int                                       parent_id;
        bool                                      is_system;
        std::string                               kind;
        std::string                               type;
        std::string                               name;
        std::string                               description;
        std::vector<std::pair<int, XSLT_Child> >  children;
        std::string to_str() const;
    };

    union XSLT_Child
    {
        XSLT_Policy_Rule *rule;
        Policy_Policy    *policy;
    };

    struct Get_Policies
    {
    Get_Policies() : policies(NULL) {}

        ~Get_Policies()
        {
            if (format == "JSTREE" && jstree)
                delete jstree;
            else if (policies)
            {
                for (size_t i = 0; i < policies->size(); ++i)
                    delete policies->at(i);
                policies->clear();
                delete policies;
            }
        }

        std::string                      format;
        union
        {
            std::vector<Policy_Policy*>* policies;
            std::string*                 jstree;
        };
    };

    struct Get_Policy
    {
        Get_Policy() : policy(NULL) {}

        ~Get_Policy()
        {
            if (format == "JSTREE" && jstree)
                delete jstree;
            else if (policy)
                delete policy;
        }

        std::string          format;
        union
        {
            Policy_Policy   *policy;
            std::string     *jstree;
        };
    };

    static const std::string display_xml_name;
    static const std::string display_maxml_name;
    static const std::string display_text_name;
    static const std::string display_html_name;
    static const std::string display_jstree_name;

    // General
    int  init();
    int  close();
    void reset_daemon_client();

    //Options
    int add_option(const std::string& option, std::string& report);

    // Analyze
    int  checker_analyze(const std::vector<std::string>& files, bool force_analyze = false);
    int  checker_analyze(const std::string& file, bool& registered, bool force_analyze = false);
    int  checker_is_done(const std::vector<std::string>& files, double& percent);
    int  checker_is_done(const std::string& file, double& percent, report& report_kind);

    void checker_list(std::vector<std::string>& vec);
    void checker_file_from_id(int id, std::string& filename);

    // Output
    int  checker_get_report(int user, const std::bitset<report_Max>& Report, format f,
                            const std::vector<std::string>& files,
                            const std::vector<size_t>& policies_ids,
                            const std::vector<std::string>& policies_contents,
                            const std::map<std::string, std::string>& options,
                            Checker_ReportRes* result,
                            const std::string* display_name = NULL,
                            const std::string* display_content = NULL);
    int checker_validate(int user, MediaConchLib::report report, const std::vector<std::string>& files,
                         const std::vector<size_t>& policies_ids,
                         const std::vector<std::string>& policies_contents,
                         const std::map<std::string, std::string>& options,
                         std::vector<Checker_ValidateRes*>& result);

    //Clear
    int remove_report(const std::vector<std::string>& files);

    // Implementation checker arguments
    void               set_implementation_schema_file(const std::string& file);
    const std::string& get_implementation_schema_file();
    void               create_default_implementation_schema();
    void               set_implementation_verbosity(const std::string& verbosity);
    const std::string& get_implementation_verbosity();

    // Xsl Transformation
    int  transform_with_xslt_file(const std::string& report, const std::string& file,
                                  const std::map<std::string, std::string>& opts, std::string& result);
    int  transform_with_xslt_memory(const std::string& report, const std::string& memory,
                                    const std::map<std::string, std::string>& opts, std::string& result);

    // Configuration
    void               load_configuration();
    void               set_configuration_file(const std::string& file);
    const std::string& get_configuration_file() const;
    void               load_plugins_configuration();
    void               set_plugins_configuration_file(const std::string& file);
    void               set_compression_mode(compression compress);
    int                get_ui_poll_request() const;
    int                get_ui_database_path(std::string& path) const;

    bool ReportAndFormatCombination_IsValid(const std::vector<std::string>& files,
                                            const std::bitset<MediaConchLib::report_Max>& reports,
                                            const std::string& display, MediaConchLib::format& Format,
                                            std::string& reason);

    // Policies
    //   Create policy
    int                          policy_duplicate(int user, int id, int dst_policy_id, std::string& err);
    int                          policy_move(int user, int id, int dst_policy_id, std::string& err);
    int                          policy_change_info(int user, int id, const std::string& name, const std::string& description, std::string& err);
    int                          policy_change_type(int user, int id, const std::string& type, std::string& err);
    int                          xslt_policy_create(int user, std::string& err, const std::string& type="and", int parent_id=-1);
    int                          xslt_policy_create_from_file(int user, const std::string& file, std::string& err);
    //   Import policy
    int                          policy_import(int user, const std::string& memory, std::string& err, const char* filename=NULL, bool is_system_policy=false);

    //   Policy helper
    size_t                       policy_get_policies_count(int user) const;
    int                          policy_get(int user, int pos, const std::string& format, Get_Policy&, std::string& err);
    int                          policy_get_name(int user, int id, std::string& name, std::string& err);
    void                         policy_get_policies(int user, const std::vector<int>&, const std::string& format, Get_Policies&);
    void                         policy_get_policies_names_list(int user, std::vector<std::pair<int, std::string> >&);
    int                          policy_save(int user, int pos, std::string& err);
    int                          policy_remove(int user, int pos, std::string& err);
    int                          policy_dump(int user, int id, std::string& memory, std::string& err);
    int                          policy_clear_policies(int user, std::string& err);

    // XSLT Policy Rule
    int                         xslt_policy_rule_create(int user, int policy_id, std::string& err);
    XsltPolicyRule*             xslt_policy_rule_get(int user, int policy_id, int id, std::string& err);
    int                         xslt_policy_rule_edit(int user, int policy_id, int rule_id, const XsltPolicyRule *rule, std::string& err);
    int                         xslt_policy_rule_duplicate(int user, int policy_id, int rule_id, int dst_policy_id, std::string& err);
    int                         xslt_policy_rule_move(int user, int policy_id, int rule_id, int dst_policy_id, std::string& err);
    int                         xslt_policy_rule_delete(int user, int policy_id, int rule_id, std::string& err);

    //Generated Values
    int                         policy_get_values_for_type_field(const std::string& type, const std::string& field, std::vector<std::string>& values);
    int                         policy_get_fields_for_type(const std::string& type, std::vector<std::string>& fields);

    // Daemon
    void set_use_daemon(bool use);
    bool get_use_daemon() const;
    void get_daemon_address(std::string& addr, int& port) const;

    // Helper
    int init_http_client();
    int close_http_client();
    int load_system_policy();
    int load_existing_policy();

private:
    MediaConchLib (const MediaConchLib&);

    std::vector<std::string>  Options;
    bool                      use_daemon;
    bool                      force_no_daemon;
    Core                     *core;
    DaemonClient             *daemon_client;
};

}

#endif
