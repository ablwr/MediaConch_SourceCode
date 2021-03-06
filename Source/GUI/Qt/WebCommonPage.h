/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

#ifndef WEBCOMMONPAGE_H
#define WEBCOMMONPAGE_H

#include "WebChoose.h"

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QFileInfo>

#if defined(WEB_MACHINE_KIT)
#include <QWebPage>
#include <QWebView>
#elif defined(WEB_MACHINE_ENGINE)
#include <QWebEnginePage>
#include <QWebEngineView>
#endif

class QDir;

namespace MediaConch {

class MainWindow;
class FileRegistered;
class XsltPolicy;
class XsltPolicyRule;

#if defined(WEB_MACHINE_KIT)
class WebCommonPage : public QWebPage
#elif defined(WEB_MACHINE_ENGINE)
class WebCommonPage : public QWebEnginePage
#endif
{
	Q_OBJECT
public:
#if defined(WEB_MACHINE_KIT)
    explicit WebCommonPage(MainWindow *m, QWidget *parent) : QWebPage(parent), mainwindow(m) {}
#elif defined(WEB_MACHINE_ENGINE)
    explicit WebCommonPage(MainWindow *m, QWidget *parent) : QWebEnginePage(parent), mainwindow(m) {}
#endif
    virtual ~WebCommonPage() {}

    void         change_local_files(const QStringList& files);
    virtual void use_javascript(const QString& js) = 0;

protected:
    void         clean_forms();
    void         add_sub_directory_files_to_list(const QDir dir, QFileInfoList& list);
    void         charge_local_dir(const QString& directory, QStringList& tmp);
    QString      choose_file_settings();
    QString      choose_file_import_policy();
    void         string_to_json(QString& str);

public Q_SLOTS:
    void         on_file_upload_selected(const QString& policy, const QString& display, const QString& verbosity);
    void         on_file_online_selected(const QString& url, const QString& policy, const QString& display, const QString& verbosity);
    void         on_file_repository_selected(const QString& policy, const QString& display, const QString& verbosity);
    void         on_save_settings_selected(const QString& policy, const QString& display, const QString& verbosity,
                                           const QString& save_report, const QString& load_files,
                                           const QString& save_policy, const QString& load_policy,
                                           const QString& save_display, const QString& load_display);

    void         menu_link_checker(const QString& name);
    void         on_input_changed(const QString& inputName);

    void         on_download_report(const QString& report, const QString& file, const QString& report_name);
    QString      on_fill_implementation_report(const QString& name, const QString& display, const QString& verbosity);
    void         on_save_implementation_report(const QString& file, const QString& display = "-1", const QString& verbosity="");
    QString      on_fill_policy_report(const QString& file, const QString& policy, const QString& display);
    void         on_save_policy_report(const QString& file, const QString& policy = "-1", const QString& display = "-1");
    QString      on_fill_mediainfo_report(const QString& file);
    void         on_save_mediainfo_report(const QString& file);
    QString      on_fill_mediatrace_report(const QString& file);
    void         on_save_mediatrace_report(const QString& file);
    QString      on_create_policy_from_file(const QString& file);

    void         close_all();
    void         close_element(const QString& file);

    bool         report_is_html(const QString& report);
    bool         report_is_xml(const QString& report);

    QString      get_file_tool(const QString& file);
    QString      get_file_policy_id(const QString& file);
    QString      get_file_display_id(const QString& file);
    QString      get_file_verbosity_id(const QString& file);
    bool         policy_is_valid(const QString& file);
    bool         implementation_is_valid(const QString& file);
    QString      file_is_analyzed(const QString& file);
    QString      change_policy_for_file(const QString& file, const QString& policy, const QString& fileId);


    QString      display_add_file(const QString& name);
    void         display_export_id(const QString& name);
    void         display_delete_id(const QString& name);

    QString      get_policies_tree();
    void         create_rule_tree(XsltPolicyRule *r, QString& rule_data);
    QString      import_policy();
    QString      import_policy(const QString& file);
    int          import_policy(const QStringList& files);
    QString      xslt_policy_create(int parent_id);
    QString      policy_duplicate(int id, int dst_policy_id);
    QString      policy_move(int id, int dst_policy_id);
    QString      policy_export(int id);
    QString      policy_remove(int id);
    QString      policy_edit(int id, const QString& name, const QString& description, const QString& type);
    QString      xslt_policy_rule_create(int policy_id);
    QString      xslt_policy_rule_edit(int rule_id, int policy_id, const QString& title, const QString& type, const QString& field, int occurrence, const QString& ope, const QString& value, const QString& scope);
    QString      xslt_policy_rule_duplicate(int policy_id, int rule_id, int dst_policy_id);
    QString      xslt_policy_rule_move(int policy_id, int rule_id, int dst_policy_id);
    QString      xslt_policy_rule_delete(int policy_id, int rule_id);
    QString      get_values_list(const QString& type, const QString& field, const QString& value);
    QString      get_fields_list(const QString& type, const QString& field);

protected:
    MainWindow                 *mainwindow;
    QMap<QString, QStringList>  file_selector;
    QString                     select_file_name;
};

}

#endif
