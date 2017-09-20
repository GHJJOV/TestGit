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

#ifndef GRAPHICS_WEB_VIEW_TEST_H
#define GRAPHICS_WEB_VIEW_TEST_H

#include "CommonQtTestHeaders.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWebKitWidgets/QGraphicsWebView>
#include <QtWidgets/QGraphicsView>
#else
#include <QtWebKit/QGraphicsWebView>
#include <QtGui/QGraphicsView>
#endif

class GraphicsWebViewTestWindows : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphicsWebViewTestWindows(QWidget *parent = 0);
    ~GraphicsWebViewTestWindows();
private:
    QGraphicsWebView *pWebView;
};

#endif // GRAPHICS_WEB_VIEW_TEST_H
