#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include <QObject>

//class MsgHandler : public QObject
//{
//    Q_OBJECT
//public:
//    explicit MsgHandler(QObject *parent = 0);

//signals:

//public slots:
//};
void uninstallMsgHandler(void);

void installMsgHandler(QString strlogfile);

void installReleaseMsgHandler(void);

#endif // MSGHANDLER_H
