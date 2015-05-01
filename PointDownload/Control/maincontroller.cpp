#include "maincontroller.h"
#include <QtQml>

MainController * MainController::mainController = NULL;
MainController * MainController::getInstance()
{
    //import时使用Singleton.MainController
    qmlRegisterSingletonType<MainController>("Singleton.MainController", 1, 0, "MainController", dataObj);

    if (mainController == NULL)
    {
        mainController = new MainController();
    }
    return mainController;
}

QObject * MainController::dataObj(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return MainController::getInstance();
}

void MainController::pStartDownload(const TaskInfo &taskInfo)
{
    //确保将要下载的文件不会重复
    deleteFileFromDisk(taskInfo.fileSavePath, taskInfo.fileList.at(0).fileName);

//    //弹出窗口才使用这个接口，每次新建连接前要先检查是否达到最大限制
//    SettingXMLHandler tmphandler;
//    if (tmphandler.getChildElement(GeneralSettings,"MaxJobCount").toInt() <= downloadingListMap.count())
//    {
//        //移除最慢的一项
//        dropSlowest();
//    }

    //将下载项插入全局map中
    taskListMap.insert(taskInfo.fileID, taskInfo.toolType);

    switch (taskInfo.toolType)
    {
    case PDataType::PToolTypePoint:
        startPointDownload(taskInfo);
        break;
    case PDataType::PToolTypeAria2:
        startAria2Download(taskInfo);
        break;
    case PDataType::PToolTypeYouGet:
        startYougetDownload(taskInfo);
        break;
    case PDataType::PToolTypeXware:
        startXwareDownload(taskInfo);
        break;
    default:
        qWarning() << "taskInfo.toolType not defined! At: void UnifiedInterface::startDownload(const TaskInfo &taskInfo)";
    }

    DownloadXMLHandler tmpOpera;
    SDownloading taskStruct;
    taskStruct.fileID = taskInfo.fileID;
    taskStruct.fileName = taskInfo.getTaskName();
    taskStruct.fileTotalSize = taskInfo.getTaskSize();
    taskStruct.fileReadySize = 0;
    taskStruct.fileSavePath = taskInfo.fileSavePath;
    taskStruct.url = taskInfo.url;
    taskStruct.toolType = taskInfo.toolType;
    taskStruct.taskState = PDataType::PTaskStateDownloading;

    emit signalAddDownloadingItem(tmpOpera.getJsonObjFromSDownloading(taskStruct));
}

void MainController::pChangeMaxJobCount(int newCount)
{

}

int MainController::pGetJobCount()
{

}

void MainController::pSuspendAllTask()
{
    qDebug() << "Suspend all task...";
}

void MainController::pResumeAllTask()
{
    qDebug() << "Resume all task...";
}

void MainController::slotTaskItemInfoUpdate(const TaskItemInfo & itemInfo)
{
    QJsonObject infoObj;
    infoObj.insert("fileID",itemInfo.fileID);
    infoObj.insert("taskDLSpeed",itemInfo.taskDLSpeed);
    infoObj.insert("taskULSpeed",itemInfo.taskULSpeed);
    infoObj.insert("thunderOfflineSpeed",itemInfo.thunderOfflineSpeed);
    infoObj.insert("thunderHightSpeed",itemInfo.thunderHightSpeed);
    infoObj.insert("taskState",itemInfo.taskState);
    infoObj.insert("taskProgress",itemInfo.taskProgress);

    emit signalTaskItemInfoUpdate(infoObj);
}

void MainController::slotTaskFinished(const QString &taskID)
{
    DownloadXMLHandler tmpHandler;

    SDownloading taskStruct = tmpHandler.getDLingNode(taskID);

    //add done-xml-node
    SDownloaded doneStruct;
    doneStruct.fileID = taskStruct.fileID;
    doneStruct.fileName = taskStruct.fileName;
    doneStruct.fileTotalSize = taskStruct.fileTotalSize;
    doneStruct.fileSavePath = taskStruct.fileSavePath;
    doneStruct.completeDate = QDateTime::currentDateTime().toString( "yyyy:MM:dd:hh:mm" );
    doneStruct.fileExist = true;
    doneStruct.url = taskStruct.url;
    doneStruct.toolType = taskStruct.toolType;
    tmpHandler.insertDLedNode(doneStruct);

    //delete task-xml-node
    tmpHandler.removeDLingFileNode(taskID);

    //update ui
    emit signalAddDownloadedItem(tmpHandler.getJsonObjFromSDownloaded(doneStruct));
    emit signalTaskFinished(taskID);
}

void MainController::slotControlFileItem(QString &fileID, PDataType::DownloadType dtype, PDataType::OperationType otype)
{
    switch (dtype)
    {
    case PDataType::PDLTypeDownloading:
        handleDownloadingControl(fileID,otype);
        break;
    case PDataType::PDLTypeDownloaded:
        handleDownloadedControl(fileID,otype);
        break;
    case PDataType::PDLTypeDownloadTrash:
        handleDownloadTrashControl(fileID,otype);
        break;
    default:
        break;
    }
}

void MainController::slotGetError(const QString &fileID, const QString &errorMessage, PDataType::ToolType toolType)
{

}

void MainController::initDownloadList()
{
    initDLedList();
    initDLingList();
    initDLtrashList();
}

void MainController::initDownloadingStart()
{
    SettingXMLHandler settingHandler;
    DownloadXMLHandler downloadHandler;

    QStringList keys = taskListMap.keys();
    int jobs = 0;
    for (int i = 0; i < keys.count(); i++)
    {
        if (settingHandler.getChildElement(SettingXMLHandler::GeneralSettings,"MaxJobCount").toInt() < jobs)
            break;
        SDownloading taskStruct = downloadHandler.getDLingNode(keys.at(i));

        if (taskStruct.taskState == PDataType::PTaskStateDownloading)
        {
            jobs ++;
            taskStruct.taskState = PDataType::PTaskStateSuspend;
            //伪造成暂停状态以使用resume方式恢复下载,与直接新建下载不同
            downloadHandler.updateDLingNode(taskStruct);

            dlingResume(keys.at(i));
        }
    }
}

MainController::MainController(QObject *parent) :
    QObject(parent)
{
    QTimer::singleShot(500, this, SLOT(initDownloadList()));
    QTimer::singleShot(501, this, SLOT(initDownloadingStart()));
}

MainController::~MainController()
{

}

void MainController::startPointDownload(const TaskInfo &taskInfo)
{

}

void MainController::startYougetDownload(const TaskInfo &taskInfo)
{
    DownloadXMLHandler tmpOpera;

    if (!tmpOpera.fileIDExist(taskInfo.fileID, PDataType::PDLTypeDownloading))
    {
        SDownloadThread threadStruct;
        threadStruct.startBlockIndex = 1;
        threadStruct.endBlockIndex = 1;
        threadStruct.completedBlockCount = 1;
        QList<SDownloadThread> tmpList;
        tmpList.append(threadStruct);

        //插入xml文件
        SDownloading taskStruct;

        taskStruct.fileID = taskInfo.fileID;
        taskStruct.fileName = taskInfo.getTaskName();
        taskStruct.fileTotalSize = taskInfo.getTaskSize();
        taskStruct.fileReadySize = 0;
        taskStruct.fileSavePath = taskInfo.fileSavePath;
        taskStruct.url = taskInfo.url;
        taskStruct.toolType = PDataType::PToolTypeYouGet;
        taskStruct.taskState = PDataType::PTaskStateDownloading;
        taskStruct.taskMaxSpeed = taskInfo.maxSpeed;
        taskStruct.averageSpeed = 0;
        taskStruct.threadList = tmpList;
        taskStruct.blockCount = 1;
        taskStruct.blockSize = 1;
        taskStruct.enableUpload = false;

        qWarning() << "==>[Info] Inser new task record to config-file:" << tmpOpera.insertDLingNode(taskStruct);
    }
    else
    {
        //必须要及时改变状态
        SDownloading tmpStruct = tmpOpera.getDLingNode(taskInfo.fileID);
        tmpStruct.taskState = PDataType::PTaskStateDownloading;

        tmpOpera.updateDLingNode(tmpStruct);
    }

    YouGetTask::getInstance()->start(taskInfo);
}

void MainController::startAria2Download(const TaskInfo &taskInfo)
{

}

void MainController::startXwareDownload(const TaskInfo &taskInfo)
{

}

void MainController::handleDownloadingControl(const QString &fileID, PDataType::OperationType otype)
{
    switch (otype)
    {
    case PDataType::PCtrlTypeDelete:
        dlingDelete(fileID);
        break;
    case PDataType::PCtrlTypeFinishDownload:
        dlingFinish(fileID);
        break;
    case PDataType::PCtrlTypeHightSpeedChannel:
        dlingHightSpeedChannel(fileID);
        break;
    case PDataType::PCtrlTypeOpenFolder:
        dlingOpenFolder(fileID);
        break;
    case PDataType::PCtrlTypeSuspend:
        dlingSuspend(fileID);
        break;
    case PDataType::PCtrlTypeResume:
        dlingResume(fileID);
        break;
    case PDataType::PCtrlTypeRaise:
        dlingRaise(fileID);
        break;
    case PDataType::PCtrlTypeTrash:
        dlingTrash(fileID);
        break;
    case PDataType::PCtrlTypeOfflineDownload:
        dlingOfflineDownload(fileID);
        break;
    default:
        qDebug() << "==>[Error] Operation type unknown!";
        break;
    }
}

void MainController::handleDownloadedControl(const QString &fileID, PDataType::OperationType otype)
{
    switch (otype)
    {
    case PDataType::PCtrlTypeDelete:
        dledDelete(fileID);
        break;
    case PDataType::PCtrlTypeOpenFolder:
        dledOpenFolder(fileID);
        break;
    case PDataType::PCtrlTypeReDownload:
        dledRedownload(fileID);
        break;
    case PDataType::PCtrlTypeTrash:
        dledTrash(fileID);
        break;
    default:
        qDebug() << "==>[Error] Operation type unknown!";
        break;
    }
}

void MainController::handleDownloadTrashControl(const QString &fileID, PDataType::OperationType otype)
{
    switch (otype)
    {
    case PDataType::PCtrlTypeDelete:
        dltrashDelete(fileID);
        break;
    case PDataType::PCtrlTypeReDownload:
        dltrashRedownload(fileID);
        break;
    default:
        qDebug() << "==>[Error] Operation type unknown!";
        break;
    }
}

void MainController::dlingDelete(const QString &fileID)
{
    switch (taskListMap.value(fileID))
    {
    case PDataType::PToolTypePoint:
        //TODO
        break;
    case PDataType::PToolTypeAria2:
        //TODO
        break;
    case PDataType::PToolTypeYouGet:
        //TODO
        YouGetTask::getInstance()->deleteTask(fileID, true);
        break;
    case PDataType::PToolTypeXware:
        //TODO
        break;
    case PDataType::PToolTypeUndefined:
        //TODO
        break;
    }
}

void MainController::dlingFinish(const QString &fileID)
{

}

void MainController::dlingHightSpeedChannel(const QString &fileID)
{

}

void MainController::dlingOfflineDownload(const QString &fileID)
{

}

void MainController::dlingOpenFolder(const QString &fileID)
{

}

void MainController::dlingRaise(const QString &fileID)
{

}

void MainController::dlingResume(const QString &fileID)
{
    switch (taskListMap.value(fileID))
    {
    case PDataType::PToolTypePoint:
        //TODO
        break;
    case PDataType::PToolTypeAria2:
        //TODO
        break;
    case PDataType::PToolTypeYouGet:
        YouGetTask::getInstance()->resume(fileID);
        break;
    case PDataType::PToolTypeXware:
        //TODO
        break;
    case PDataType::PToolTypeUndefined:
        //TODO
        break;
    }

    DownloadXMLHandler downloadXMLHandler;
    SDownloading taskStruct = downloadXMLHandler.getDLingNode(fileID);
    taskStruct.taskState = PDataType::PTaskStateDownloading;
    //恢复成正在下载状态
    downloadXMLHandler.updateDLingNode(taskStruct);
}

void MainController::dlingSuspend(const QString &fileID)
{
    switch (taskListMap.value(fileID))
    {
    case PDataType::PToolTypePoint:
        //TODO
        break;
    case PDataType::PToolTypeAria2:
        //TODO
        break;
    case PDataType::PToolTypeYouGet:
        //TODO
        YouGetTask::getInstance()->suspend(fileID);
        break;
    case PDataType::PToolTypeXware:
        //TODO
        break;
    case PDataType::PToolTypeUndefined:
        //TODO
        break;
    }

    DownloadXMLHandler downloadXMLHandler;
    SDownloading taskStruct = downloadXMLHandler.getDLingNode(fileID);
    taskStruct.taskState = PDataType::PTaskStateSuspend;
    //更新状态为暂停
    downloadXMLHandler.updateDLingNode(taskStruct);
}

void MainController::dlingTrash(const QString &fileID)
{
    switch (taskListMap.value(fileID))
    {
    case PDataType::PToolTypePoint:
        //TODO
        break;
    case PDataType::PToolTypeAria2:
        //TODO
        break;
    case PDataType::PToolTypeYouGet:
        //TODO
        YouGetTask::getInstance()->trashTask(fileID, true);
        break;
    case PDataType::PToolTypeXware:
        //TODO
        break;
    case PDataType::PToolTypeUndefined:
        //TODO
        break;
    }
}

void MainController::dledDelete(const QString &fileID)
{

}

void MainController::dledOpenFolder(const QString &fileID)
{

}

void MainController::dledRedownload(const QString &fileID)
{

}

void MainController::dledTrash(const QString &fileID)
{

}

void MainController::dltrashDelete(const QString &fileID)
{

}

void MainController::dltrashRedownload(const QString &fileID)
{

}

void MainController::initDLedList()
{
    DownloadXMLHandler tmpOpera;
    QList<SDownloaded> doneList = tmpOpera.getDLedNodes();
    for (int i = 0;i < doneList.count(); i++)
    {
        emit signalAddDownloadedItem(tmpOpera.getJsonObjFromSDownloaded(doneList.at(i)));
    }
}

void MainController::initDLingList()
{
    DownloadXMLHandler tmpOpera;
    QList<SDownloading> taskList = tmpOpera.getDLingNodes();

    for (int i = 0;i < taskList.count(); i++)
    {
        emit signalAddDownloadingItem(tmpOpera.getJsonObjFromSDownloading(taskList.at(i)));

        if (taskList.at(i).taskState == PDataType::PTaskStateDownloading)
        {
            taskListMap.insert(taskList.at(i).fileID, taskList.at(i).toolType); //只添加到下载列表，暂时不下载
        }
    }
}

void MainController::initDLtrashList()
{
    DownloadXMLHandler tmpOpera;
    QList<SDownloadTrash> trashList = tmpOpera.getDLtrashNodes();
    for (int i = 0; i < trashList.count(); i ++)
    {
        emit signalAddDownloadTrashItem(tmpOpera.getJsonObjFromSDownloadTrash(trashList.at(i)));
    }
}

void MainController::deleteFileFromDisk(QString path, QString fileName)
{

}


