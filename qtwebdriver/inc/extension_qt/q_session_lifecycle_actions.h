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

#ifndef WEBDRIVER_QT_SESSION_ACTIONS_H_
#define WEBDRIVER_QT_SESSION_ACTIONS_H_

#include <string>
#include <vector>

#include "webdriver_session.h"

#include <QtNetwork/QNetworkProxy>

namespace webdriver {

class QSessionLifeCycleActions : public SessionLifeCycleActions {
public:
    QSessionLifeCycleActions(Session* session);
    virtual ~QSessionLifeCycleActions() {};

    virtual Error* PostInit(const base::DictionaryValue* desired_capabilities_dict,
                const base::DictionaryValue* required_capabilities_dict);

    virtual void BeforeTerminate(void);

protected:
private:
    bool restore_proxy_;
    QNetworkProxy saved_proxy_;

    Error* ParseAndApplyProxySettings(const base::DictionaryValue* proxy_dict);

    bool AddActualQtVersion();
    Error* CheckRequiredQtVersion(const base::DictionaryValue *capabilities_dict);

    DISALLOW_COPY_AND_ASSIGN(QSessionLifeCycleActions);
};

}  // namespace webdriver

#endif  // WEBDRIVER_QT_SESSION_ACTIONS_H_


