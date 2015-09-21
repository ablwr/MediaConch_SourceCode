/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

#include <QWebView>
namespace MediaConch {

class WebView : public QWebView
{
    Q_OBJECT

public:
    explicit WebView(QWidget *parent = 0);

protected:
    void dropEvent(QDropEvent *event);
};

}