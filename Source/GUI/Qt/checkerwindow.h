/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

#ifndef CHECKERWINDOW_H
#define CHECKERWINDOW_H

#include "mainwindow.h"

#include <QFileInfo>
#include <QString>

class QProgressBar;
class QFile;
class QWebView;

namespace MediaConch {

class MainWindow;
class WebView;

class CheckerWindow : public QObject
{
    Q_OBJECT

public:
    explicit CheckerWindow(MainWindow *parent = 0);
    ~CheckerWindow();

    // Functions
    void                        createWebView();
    void                        updateWebView(String file, int policy);
    void                        updateWebView(QList<QFileInfo>& files, int policy);
    void                        setDisplayXslt(QString& d) { displayXslt = d; }
    void                        resetDisplayXslt() { displayXslt.clear(); }

    // Helpers
    void                        checker_add_file(QString& file, int policy);
    void                        checker_add_files(QList<QFileInfo>& file, int policy);
    void                        checker_add_policy_file(QString& file, QString& policy);
    void                        checker_add_policy_files(QList<QFileInfo>& file, QString& policy);
    bool                        is_analyzes_done();

private:
    // Visual elements
    MainWindow*                 mainwindow;
    WebView*                    MainView;
    QProgressBar*               progressBar;
    QString                     displayXslt;
    bool                        analyse;

    void                        clearVisualElements();
    void                        setWebViewContent(QString& html);

//***************************************************************************
// HELPER
//***************************************************************************

    void remove_template_tags(QString& data);
    void load_include_in_template(QString& html);
    void remove_element_in_template(QString& html);
    void load_form_in_template(QString& html);
    QString create_html();
    QString create_html_base(QString& body);
    QString create_html_body();
    QString create_form_upload();
    QString create_form_online();
    QString create_form_repository();
    void remove_form_online(int pos, QString& html);
    void change_collapse_form(QString& html);
    void change_body_in_template(QString& body, QString& html);
    void add_policy_to_form_selection(QString& policies, QString& form, const char *selector);
    void create_policy_options(QString& policies);
    void add_file_detail_to_html(QString& html, String& file, int policy);
    QString create_html_file_detail(String& file, int policy);
    void change_html_file_detail(QString& html, String& file);
    void change_html_file_detail_inform_xml(QString& html, String& file);
    void change_html_file_detail_conformance(QString& html, String& file);
    void change_html_file_detail_policy_report(QString& html, String& file, int policy);
    void change_html_file_detail_trace(QString& html, String& file);
    void remove_html_file_detail_policy_report(QString& html);
    bool report_is_html(QString &report);
    bool is_policy_html_valid(QString &report);
    void change_report_policy_save_name(String& file, QString& policy, bool is_html, QString& html);
    QString file_remove_ext(String& file);

public Q_SLOTS:
    void actionCloseAllTriggered();
private Q_SLOTS:
    void createWebViewFinished(bool ok);
};

}

#endif // MAINWINDOW_H