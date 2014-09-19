/***********************************************************************
*PointDownload
*Copyright (C) 2014  PointTeam
*
* Author:     Choldrim <choldrim@foxmail.com>
* Maintainer: Choldrim <choldrim@foxmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#include "xwarepopulateobject.h"

XwarePopulateObject::XwarePopulateObject(QObject *parent) :
    QObject(parent)
{
    spliterBtwData = XWARE_CONSTANTS_STRUCT.SPLITER_BTWN_DATA;
    spliterEnd =  XWARE_CONSTANTS_STRUCT.SPLITER_END;
    defaultPara = XWARE_CONSTANTS_STRUCT.SPLITER_DEFAULT_PARAM;

    connect(this, SIGNAL(sHint(QString,QString)), this, SLOT(handleHintEmit(QString,QString)));
    connect(this, SIGNAL(sError(QString,QString)), this, SLOT(handleErrorEmit(QString,QString)));
}

XwarePopulateObject * XwarePopulateObject::xwarePopulateObject = NULL;
XwarePopulateObject * XwarePopulateObject::getInstance()
{
    if (xwarePopulateObject == NULL)
        xwarePopulateObject = new XwarePopulateObject();
    return xwarePopulateObject;
}

XwarePopulateObject::~XwarePopulateObject()
{
    if(XWARE_CONSTANTS_STRUCT.DEBUG)
        qDebug()<<"~XwarePopulateObject()  was be called !!";
}

void XwarePopulateObject::addNewDownloadTask(QString url, QString storage, QStringList fileList)
{
    emit sJSAddNewDownloadTask(url, storage, fileList);
}

void XwarePopulateObject::addNewBTDownloadTask(QString btFilePath, QString storage, QStringList fileList)
{
    emit addNewBTDownloadTask(btFilePath, storage, fileList);
}

void XwarePopulateObject::suspendDownloadingTask(QString tid)
{
    emit sJSSuspendDownloadingTask(tid);
}

void XwarePopulateObject::resumeDownloadingTask(QString tid)
{
    emit sJSResumeDownloadingTask(tid);
}

void XwarePopulateObject::removeDownloadingTask(QString tid)
{
    emit sJSRemoveDownloadingTask(tid);
}

void XwarePopulateObject::entryOfflineChannel(QString tid)
{
    emit sJSEntryOfflineChannel(tid);
}

void XwarePopulateObject::entryHighSpeedChannel(QString tid)
{
    emit sJSEntryHighSpeedChannel(tid);
}

// called by javascript
void XwarePopulateObject::setAllBindedPeerIds(QString ids)
{
    if(XWARE_CONSTANTS_STRUCT.DEBUG)
        qDebug()<<"recv sReturnAllBindedPeerIds => " << ids;

    QStringList list;
    if(! ids.contains(spliterBtwData))
    {
        // debug
        qDebug()<<"AllBindedPeerIds is empty ~~~";
    }
    else
    {
        list = ids.split(spliterBtwData);
        list.removeLast(); // remove the last empty element
    }

    if(XWARE_CONSTANTS_STRUCT.DEBUG)
        qDebug()<< list << " .....";
    emit sReturnAllBindedPeerIds(list);
}

// emit the singal to javascript to start feedbacking tasks information
void XwarePopulateObject::startFeedbackDloadList()
{
    emit sJSReflashDownloadList();
}

void XwarePopulateObject::login(QString userName, QString pwd, QString vertifyCode)
{
    if(XWARE_CONSTANTS_STRUCT.DEBUG)
        qDebug()<<"======== XwarePopulateObject::login ===========";

    // emit this to javascript
    emit sJSLogin(userName, pwd, vertifyCode);
}

void XwarePopulateObject::logout()
{
    if(XWARE_CONSTANTS_STRUCT.DEBUG)
        qDebug()<<"======== XwarePopulateObject::logout ===========";
    
    // emit this to javascript
    emit sJSLogout();
}

void XwarePopulateObject::getAllBindedPeerIds()
{
    // emit this to javascript
    emit sJSGetAllBindedPeerIds();
}

void XwarePopulateObject::justForJSTest(QString testStr)
{
    if(XWARE_CONSTANTS_STRUCT.DEBUG)
        qDebug()<<" ************* javascript feedback test !!  ==>" << testStr;
}

void XwarePopulateObject::urlParse(QString url)
{
    emit sJSUrlParse(url);
}

void XwarePopulateObject::btParse(QString btFilePath)
{
    emit sJSBTParse(btFilePath);
}

void XwarePopulateObject::feedbackDownloadList(QString tasksInfo)
{
    emit sFeedbackDownloadList(tasksInfo);
}

void XwarePopulateObject::feedbackURLParse(QString taskInfoList)
{
    emit sFeedbackURLParse(taskInfoList);
}

void XwarePopulateObject::loginError(short type, QString errorMsg)
{
    switch (type)
    {
    // username
    case 1:
        emit sError(tr("Login Error"), errorMsg);
        qDebug()<<" login username error =>"<<errorMsg;
        break;

    // password
    case 2:
        emit sError(tr("Login Error"), errorMsg);
        qDebug()<<" login password error =>"<<errorMsg;
        break;

    // vertify code
    case 3:
        // emit vertify code link
        QString vertifyCodeUrl = errorMsg.split(this->spliterBtwData).at(1);
        emit sVertifyCodeLink(vertifyCodeUrl);

        errorMsg = errorMsg.split(this->spliterBtwData).at(0);

        // emit hint
        if(errorMsg.startsWith("请"))
        {
            emit sHint(tr("Login Hint"), errorMsg);
            qDebug()<<" login vertify code hint =>"<<errorMsg;
        }

        // emit error
        else
        {
            emit sError(tr("Login Error"), errorMsg);
            qDebug()<<" login vertify code error =>"<<errorMsg;
        }
        break;
    }
}

void XwarePopulateObject::handleErrorEmit(QString title, QString msg)
{
    NormalNotice::getInstance()->showMessage(title, Notice_Color_Error, msg);
}

void XwarePopulateObject::handleHintEmit(QString title, QString msg)
{
    NormalNotice::getInstance()->showMessage(title, Notice_Color_Warning, msg);
}

QString XwarePopulateObject::getDefaultTaskPara()
{
    return this->defaultPara;
}

QString XwarePopulateObject::getSpliterEnd()
{
    return this->spliterEnd;
}

QString XwarePopulateObject::getSpliterBtwData()
{
    return this->spliterBtwData;
}

