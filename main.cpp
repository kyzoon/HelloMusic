#include "dialog.h"
#include <QApplication>

#include "msghandler.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifndef QT_DEBUG
    bool add_quit_func = false;
    installReleaseMsgHandler();
    if(!add_quit_func)
    {
        qAddPostRoutine(uninstallMsgHandler);
        add_quit_func = true;
    }
#endif

    Dialog w;
    w.show();

    return a.exec();
}
