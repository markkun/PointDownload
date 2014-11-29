/***********************************************************************
*PointDownload
*Copyright (C) 2014  PointTeam
*
* Author:     Match <2696627729@qq.com>
* Maintainer: Match <2696627729@qq.com>
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

#include "yougettask.h"

#include "taskinfo.h"

YouGetTask_::YouGetTask_(QObject *parent) :
    QObject(parent)
{
    initConnection();
}


YouGetTask_ * YouGetTask_::youGetTask = NULL;
YouGetTask_ * YouGetTask_::getInstance()
{
    if (youGetTask == NULL)
    {
        youGetTask = new YouGetTask_();
    }
    return youGetTask;
}

void YouGetTask_::startDownload(const TaskInfo &taskInfo)
{
    YouGetProcess * yougetProcess = new YouGetProcess(taskInfo);
    connect(yougetProcess, SIGNAL(updateData(DownloadingItemInfo)), this ,SIGNAL(sRealTimeData(DownloadingItemInfo)));
    connect(yougetProcess, SIGNAL(yougetError(QString,QString,int))
            ,this ,SIGNAL(sYouGetError(QString,QString,int)));
    connect(yougetProcess, SIGNAL(sFinishYouGetDownload(QString)),
            this, SLOT(slotFinishDownload(QString)));

    yougetProcess->startDownload();

    //保存下载列表
    gProcessMap.insert(taskInfo.rawUrl.toString(), yougetProcess);
}

void YouGetTask_::stopDownload(QString URL)
{
    if (gProcessMap.value(URL) == NULL)
        return;
    gProcessMap.value(URL)->stopDownload();
}

void YouGetTask_::suspendDownloading(QString URL)
{
    if (gProcessMap.value(URL) == NULL)
        return
    gProcessMap.value(URL)->stopDownload();
}

void YouGetTask_::resumeDownloading(QString URL)
{
    if (gProcessMap.value(URL) == NULL)
    {
        startDownload(getPrepareInfoFromXML(URL));
    }
    else
        gProcessMap.value(URL)->startDownload();
}

void YouGetTask_::slotFinishDownload(QString URL)
{
    gProcessMap.remove(URL);

    UnifiedInterface::getInstance()->cleanDownloadFinishItem(URL);
}

TaskInfo YouGetTask_::getPrepareInfoFromXML(QString URL)
{
    DownloadXMLHandler xmlOpera;
    SDownloading ingNode = xmlOpera.getDownloadingNode(URL);

    TaskInfo taskInfo;
    TaskFileItem fileItem;

    taskInfo.rawUrl = ingNode.URL;
    taskInfo.taskIconPath = ingNode.iconPath;
    taskInfo.maxSpeed = 0;
    taskInfo.parseUrl = ingNode.redirectURL;
    taskInfo.savePath = ingNode.savePath;
    taskInfo.maxThreads = ingNode.threadList.size();
    taskInfo.toolType = TOOL_YOUGET;

    fileItem.fileName = ingNode.name;
    fileItem.fileSize = ingNode.totalSize.toInt();
    taskInfo.fileList.append(fileItem);

    return taskInfo;
}


void YouGetTask_::initConnection()
{
    connect(this, SIGNAL(sRealTimeData(DownloadingItemInfo)),
            UnifiedInterface::getInstance(), SIGNAL(sRealTimeData(DownloadingItemInfo)));
    connect(this, SIGNAL(sYouGetError(QString,QString,int)),
            UnifiedInterface::getInstance(), SLOT(downloadGetError(QString,QString,int)));
}
