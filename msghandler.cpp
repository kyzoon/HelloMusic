#include "msghandler.h"

#include <QCoreApplication>
#include <QTime>
#include <QDir>
#include <QFile>
#include <QDebug>

//MsgHandler::MsgHandler(QObject *parent) : QObject(parent)
//{

//}

#define _TIME_	qPrintable(QTime::currentTime().toString("hh:mm:ss:zzz"))

static FILE *s_filestream = NULL;
static int s_loglevel = 0;

static void MsgOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray local_msg = msg.toLocal8Bit();
    switch(type)
    {
    case QtDebugMsg:
//        fprintf(s_filestream, "Debug: %s (%s:%u, %s)\n", local_msg.constData(), context.file, context.line, context.function);
        fprintf(s_filestream, "%s\n", local_msg.constData());
        break;
    case QtInfoMsg:
        fprintf(s_filestream, "Info: %s (%s:%u, %s)\n", local_msg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(s_filestream, "Warning: %s (%s:%u, %s)\n", local_msg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(s_filestream, "Critical: %s (%s:%u, %s)\n", local_msg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(s_filestream, "Fatal: %s (%s:%u, %s)\n", local_msg.constData(), context.file, context.line, context.function);
        abort();
        break;
    }
}

void uninstallMsgHandler(void)
{
    if(NULL != s_filestream)
    {
        qDebug("Close log file.");
        fclose(s_filestream);
        s_filestream = NULL;
    }
}

void installMsgHandler(QString strlogfile)
{
    QByteArray balogfile = strlogfile.toLatin1();

    if(NULL != s_filestream)
    {
        fclose(s_filestream);
        s_filestream = NULL;
    }

//    s_filestream = fopen(balogfile.constData(), "w+");
    s_filestream = fopen(balogfile.constData(), "a+");
    if(NULL != s_filestream)
    {
        qDebug("Open log file.");
        qInstallMessageHandler(MsgOutput);
    }
    else
    {
        qDebug("Failure to open log file.");
    }
}

void installReleaseMsgHandler(void)
{
    QString str_app_path = QCoreApplication::applicationDirPath();
    QDir *dir = new QDir(str_app_path);
    if(!dir->exists("log"))
    {
        dir->mkdir("log");
    }

    QString str_log_file = str_app_path + "/log/log.txt";
    installMsgHandler(str_log_file);
}
