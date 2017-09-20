/****************************************************************************
**
** Copyright © 1992-2014 Cisco and/or its affiliates. All rights reserved.
** All rights reserved.
** 
** $CISCO_BEGIN_LICENSE:LGPL$
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** $CISCO_END_LICENSE$
**
****************************************************************************/

#ifndef CLICKSCROLLINGTEST_H
#define CLICKSCROLLINGTEST_H

#include "CommonQtTestHeaders.h"

namespace Ui {
class ClickScrollingTestWidget;
}

class ClickScrollingTestWidget : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ClickScrollingTestWidget(QWidget *parent = 0);
    ~ClickScrollingTestWidget();
    
private:
    Ui::ClickScrollingTestWidget *ui;
//private slots:
private Q_SLOTS:
    void ClickOnVisibleButton();
    void ClickOnButtonHiddenByOverflow();
    void ClickOnPartiallyVisibleRadioButton();
};

#endif // CLICKSCROLLINGTEST_H
