/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

#include "policieswindow.h"
#include "mainwindow.h"

#include "WebPage.h"
#include "WebView.h"
#include "progressbar.h"

#if defined(WEB_MACHINE_ENGINE)
#include <QWebChannel>
#endif
#if defined(WEB_MACHINE_KIT)
#include <QWebFrame>
#endif
#include <QProgressBar>

namespace MediaConch {

//***************************************************************************
// Constructor / Desructor
//***************************************************************************

PoliciesWindow::PoliciesWindow(MainWindow *parent) : mainwindow(parent), web_view(NULL), progress_bar(NULL)
{
}

PoliciesWindow::~PoliciesWindow()
{
    clear_visual_elements();
}

//***************************************************************************
// Visual elements
//***************************************************************************

//---------------------------------------------------------------------------
void PoliciesWindow::display_policies()
{
    create_html();
}

//---------------------------------------------------------------------------
void PoliciesWindow::clear_visual_elements()
{
    if (progress_bar)
    {
        mainwindow->remove_widget_from_layout(progress_bar);
        delete progress_bar;
        progress_bar = NULL;
    }

    if (web_view)
    {
#if defined(WEB_MACHINE_ENGINE)
        WebPage* page = (WebPage*)web_view->page();
        QWebChannel *channel = page ? page->webChannel() : NULL;
        if (channel)
            channel->deregisterObject(page);
#endif
        mainwindow->remove_widget_from_layout(web_view);
        delete web_view;
        web_view = NULL;
    }
}

int PoliciesWindow::add_new_policies(const QStringList& files)
{
    WebPage* page = (WebPage*)web_view->page();
    if (!page)
        return -1;

    return page->import_policy(files);
}

//***************************************************************************
// WEB
//***************************************************************************

//---------------------------------------------------------------------------
void PoliciesWindow::create_web_view_finished(bool ok)
{
    if (!web_view || !ok)
    {
        create_html();
        mainwindow->set_msg_to_status_bar("Problem to load the policy page");
        return;
    }

    if (progress_bar)
    {
        mainwindow->remove_widget_from_layout(progress_bar);
        delete progress_bar;
        progress_bar = NULL;
    }

    web_view->show();
    web_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainwindow->set_widget_to_layout(web_view);
}

//---------------------------------------------------------------------------
void PoliciesWindow::change_qt_scripts_in_template(QString& html)
{
    QRegExp reg("\\{\\{ QT_SCRIPTS \\}\\}");
    QString script;
    int     pos = 0;

    reg.setMinimal(true);
#if defined(WEB_MACHINE_KIT)
    script += "        <script type=\"text/javascript\" src=\"qrc:/policyWebKit.js\"></script>\n";
#elif defined(WEB_MACHINE_ENGINE)
    script += "        <script type=\"text/javascript\" src=\"qrc:/qtwebchannel/qwebchannel.js\"></script>\n"
              "        <script type=\"text/javascript\" src=\"qrc:/webengine.js\"></script>\n"
              "        <script type=\"text/javascript\" src=\"qrc:/policyWebEngine.js\"></script>\n";
#endif
    script += "        <script type=\"text/javascript\" src=\"qrc:/policyTree.js\"></script>\n";
    script += "        <script type=\"text/javascript\" src=\"qrc:/policyTreeAffix.js\"></script>\n";
    script += "        <script type=\"text/javascript\" src=\"qrc:/policyTreePolicies.js\"></script>\n";
    script += "        <script type=\"text/javascript\" src=\"qrc:/policyTreeRules.js\"></script>\n";
    script += "        <script type=\"text/javascript\" src=\"qrc:/policyTreeRulesMI.js\"></script>\n";
    script += "        <script type=\"text/javascript\" src=\"qrc:/policyTreeRulesMT.js\"></script>\n";
    script += "        <script type=\"text/javascript\" src=\"qrc:/policy.js\"></script>\n";
    if ((pos = reg.indexIn(html, pos)) != -1)
        html.replace(pos, reg.matchedLength(), script);
}

//---------------------------------------------------------------------------
void PoliciesWindow::set_webmachine_script_in_template(QString& html)
{
    QRegExp reg("\\{\\{[\\s]+webmachine[\\s]\\}\\}");
    QString machine;
    int     pos = 0;

    reg.setMinimal(true);
#if defined(WEB_MACHINE_KIT)
    machine = "WEB_MACHINE_KIT";
#elif defined(WEB_MACHINE_ENGINE)
    machine = "WEB_MACHINE_ENGINE";
#endif
    if ((pos = reg.indexIn(html, pos)) != -1)
        html.replace(pos, reg.matchedLength(), machine);
}

//---------------------------------------------------------------------------
void PoliciesWindow::remove_result_in_template(QString& html)
{
    QRegExp reg("\\{% block result %\\}\\{% endblock %\\}");
    int pos = 0;

    reg.setMinimal(true);
    while ((pos = reg.indexIn(html, pos)) != -1)
        html.replace(pos, reg.matchedLength(), "");
}

//---------------------------------------------------------------------------
void PoliciesWindow::change_checker_in_template(const QString& policy, QString& html)
{
    QRegExp reg("\\{% block checker %\\}\\{% endblock %\\}");
    int pos = 0;

    reg.setMinimal(true);
    while ((pos = reg.indexIn(html, pos)) != -1)
        html.replace(pos, reg.matchedLength(), policy);
}

//---------------------------------------------------------------------------
void PoliciesWindow::create_html_policy(QString& policy)
{
    QFile template_html(":/xsltPolicy.html");
    template_html.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray html = template_html.readAll();
    template_html.close();

    policy = QString(html);
}

//---------------------------------------------------------------------------
void PoliciesWindow::create_html_base(const QString& policy, QString& base)
{
    QFile template_html(":/base.html");
    template_html.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray html = template_html.readAll();
    template_html.close();

    base = QString(html);

    set_webmachine_script_in_template(base);
    change_qt_scripts_in_template(base);
    change_checker_in_template(policy, base);
    remove_result_in_template(base);
}

//---------------------------------------------------------------------------
void PoliciesWindow::create_html()
{
    clear_visual_elements();
    QString policy;
    create_html_policy(policy);
    QString html;
    create_html_base(policy, html);

    progress_bar = new ProgressBar(mainwindow);
    mainwindow->set_widget_to_layout(progress_bar);
    progress_bar->get_progress_bar()->setValue(0);
    progress_bar->show();

    web_view = new WebView(mainwindow);
    web_view->hide();

    WebPage* page = new WebPage(mainwindow, web_view);
    web_view->setPage(page);

    QObject::connect(web_view, SIGNAL(loadProgress(int)), progress_bar->get_progress_bar(), SLOT(setValue(int)));
    QObject::connect(web_view, SIGNAL(loadFinished(bool)), this, SLOT(create_web_view_finished(bool)));

    QUrl url = QUrl("qrc:/html");
    if (!url.isValid())
        return;

#if defined(WEB_MACHINE_ENGINE)
    QWebChannel *channel = new QWebChannel(page);
    page->setWebChannel(channel);
    channel->registerObject("webpage", page);
    web_view->setHtml(html.toUtf8(), url);
#endif
#if defined(WEB_MACHINE_KIT)
    web_view->setContent(html.toUtf8(), "text/html", url);
#endif
}

//***************************************************************************
// HELPER
//***************************************************************************

}
