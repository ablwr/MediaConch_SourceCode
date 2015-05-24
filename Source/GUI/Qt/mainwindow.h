/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a GPLv3+/MPLv2+ license that can
 *  be found in the License.html file in the root of the source tree.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Common/Core.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class QPlainTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // Functions
    void dragEnterEvent         (QDragEnterEvent *event);
    void dropEvent              (QDropEvent *event);

    // UI
    void                        Ui_Init                     ();

    // Visual elements
    QPlainTextEdit*             plainTextEdit;

    // Helpers
    void                        Run();

private:
    Ui::MainWindow *ui;

    // Internal
    Core C;

private Q_SLOTS:

    void on_actionOpen_triggered();
    void on_actionCloseAll_triggered();
    void on_actionConch_triggered();
    void on_actionInfo_triggered();
    void on_actionTrace_triggered();
    void on_actionText_triggered();
    void on_actionXml_triggered();
};

#endif // MAINWINDOW_H
