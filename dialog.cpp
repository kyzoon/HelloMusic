#include "dialog.h"
#include "ui_dialog.h"

#include <QDebug>
#include <QTime>
#include <QMediaMetaData>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSplitter>
#include <QtXml>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    s_play_flag = false;
    s_rotation_flag = false;
    s_list_visible_flag = true;
    s_num = 0;
    s_win_height = 0;

    s_pl_model = new QStringListModel;
    s_model = new QStringListModel;
    s_player = new QMediaPlayer;
    s_playlist = new QMediaPlaylist;

#if 0 // For Test
    ++s_num;
    s_strlist << QString::number(s_num) + ". " + "Beyond-不再犹豫";
    ++s_num;
    s_strlist << QString::number(s_num) + ". " + "Beyond-大地";
    ++s_num;
    s_strlist << QString::number(s_num) + ". " + "Beyond-海阔天空.mp3";
    ++s_num;
    s_strlist << QString::number(s_num) + ". " + "Beyond-灰色轨迹.mp3";
    ++s_num;
    s_strlist << QString::number(s_num) + ". " + "Beyond-冷雨夜.mp3";
    s_model->setStringList(s_strlist);
    ui->lvList->setModel(s_model);

    s_playlist->addMedia(QUrl::fromLocalFile("F:\\Music\\Chain\\Beyond\\Beyond-不再犹豫.mp3"));
    s_playlist->addMedia(QUrl::fromLocalFile("F:\\Music\\Chain\\Beyond\\Beyond-光辉岁月.mp3"));
    s_playlist->addMedia(QUrl::fromLocalFile("F:\\Music\\Chain\\Beyond\\Beyond-海阔天空.mp3"));
    s_playlist->addMedia(QUrl::fromLocalFile("F:\\Music\\Chain\\Beyond\\Beyond-灰色轨迹.mp3"));
    s_playlist->addMedia(QUrl::fromLocalFile("F:\\Music\\Chain\\Beyond\\Beyond-冷雨夜.mp3"));
    s_player->setPlaylist(s_playlist);
#endif // For Test

    setupUI();

    connect(s_player, SIGNAL(positionChanged(qint64)), this, SLOT(updatePosition(qint64)));
    connect(s_player, SIGNAL(durationChanged(qint64)), this, SLOT(updateDuration(qint64)));
    connect(s_player, SIGNAL(mediaChanged(QMediaContent)), this, SLOT(mediaChanged(QMediaContent)));
    connect(s_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));

    connect(s_playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(playlistIndexChanged(int)));
    connect(ui->lvList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(changedMediaFromList(QModelIndex)));
    connect(ui->hsVol, SIGNAL(valueChanged(int)), this, SLOT(updateVol(int)));
}

void Dialog::setupUI(void)
{
    setWindowFlags(Qt::WindowCloseButtonHint);
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);

#if defined(Q_OS_LINUX)
    // For Debug
//    s_settings = new QSettings("/home/kyzoon/.config.ini", QSettings::IniFormat);
    // For Release
    s_settings = new QSettings(".config.ini", QSettings::IniFormat);
#elif defined(Q_OS_WIN)
    s_settings = new QSettings("D:\\QtPorject\\build-HelloMusic-Desktop_Qt_5_8_0_MinGW_32bit-Debug\\debug\\config.ini", QSettings::IniFormat);
#endif
    s_settings->beginGroup("Win");
    int win_x = s_settings->value("x").toInt();
    int win_y = s_settings->value("y").toInt();
    int win_h = s_settings->value("h").toInt();
    s_settings->endGroup();

    setGeometry(win_x, win_y, 370, win_h);

    // Load PlayList
    s_settings->beginGroup("PlayList");
    QString pl_path = s_settings->value("path").toString();
    QString pl_file_name = s_settings->value("name").toString();
    s_pl_strlist << pl_file_name;
    s_settings->endGroup();

    s_pl_model->setStringList(s_pl_strlist);
    ui->lvPlayList->setModel(s_pl_model);

    QDomDocument doc(pl_file_name);
#if defined(Q_OS_LINUX)
    QFile file(pl_path + "/" + pl_file_name + ".hmpl");
#elif defined(Q_OS_WIN)
    QFile file(pl_path + "\\" + pl_file_name + ".hmpl");
#endif
    if(!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    if(!doc.setContent(&file))
    {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
//    qDebug() << docElem.tagName();
//    qDebug() << docElem.attribute("title");
//    qDebug() << docElem.attribute("version");
//    qDebug() << docElem.attribute("generator");

    QDomNode format = docElem.firstChild();
    QDomNode itemsn = format.nextSibling();
    QDomElement items = itemsn.toElement();
//    qDebug() << items.tagName();
//    int count = items.attribute("count").toInt();
//    qDebug() << count;
    QDomNode itemn = items.firstChild();
    s_strlist.clear();
    while(!itemn.isNull())
    {
        QDomElement item = itemn.toElement();
        if(!item.isNull())
        {
            ++s_num;
            // Add to playlist
#if defined(Q_OS_LINUX)
            QString file = item.attribute("file");
            file.replace("\\", "/");
            s_playlist->addMedia(QUrl::fromLocalFile(pl_path + "/" + file));
#elif defined(Q_OS_WIN)
            s_playlist->addMedia(QUrl::fromLocalFile(pl_path + "\\" + item.attribute("file")));
#endif
            // Add to ui tablelist
            s_strlist.append(QString::number(s_num) + ". " + item.attribute("title"));
        }
        itemn = itemn.nextSibling();
    }

    s_player->setPlaylist(s_playlist);

    s_model->setStringList(s_strlist);
    ui->lvList->setModel(s_model);

    s_settings->beginGroup("PlayInfo");
    int index = s_settings->value("index").toInt();
    int vol = s_settings->value("vol").toInt();
    s_settings->endGroup();

    s_playlist->setCurrentIndex(index);
    QModelIndex qmindex = s_model->index(index);
    ui->lvList->setCurrentIndex(qmindex);

    s_player->setVolume(vol);
    ui->hsVol->setValue(vol);
}

// reload closeEvent
void Dialog::closeEvent(QCloseEvent *evt)
{
    QRect rect = geometry();
    s_settings->beginGroup("Win");
    s_settings->setValue("x", QVariant(rect.x()));
    s_settings->setValue("y", QVariant(rect.y()));
    s_settings->setValue("h", QVariant(rect.height()));
    s_settings->endGroup();

    s_settings->beginGroup("PlayList");
#if defined(Q_OS_LINUX)
    s_settings->setValue("path", QVariant("/media/kyzoon/Enjoy/Music"));
#elif defined(Q_OS_WIN)
    s_settings->setValue("path", QVariant("F:\\Music"));
#endif
//    s_settings->setValue("name", QVariant("[默认]"));
    s_settings->setValue("name", QVariant(s_pl_strlist.at(0)));
    s_settings->endGroup();

    s_settings->beginGroup("PlayInfo");
    s_settings->setValue("index", QVariant(ui->lvList->currentIndex().row()));
    s_settings->setValue("vol", QVariant(ui->hsVol->value()));
    s_settings->endGroup();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::updatePosition(qint64 pos)
{
    ui->hsPosition->setValue(pos);
    QTime duration(0, pos / 60000, qRound((pos % 60000) / 1000.0));
    ui->lNow->setText(duration.toString(tr("mm:ss")));
}

void Dialog::updateDuration(qint64 dur)
{
//    QTime duration(0, dur / 60000, qRound((dur % 60000) / 1000.0));
//    ui->lEndTime->setText(duration.toString(tr("mm:ss")));

    ui->hsPosition->setRange(0, dur);
    ui->hsPosition->setEnabled(dur > 0);
    ui->hsPosition->setPageStep(dur / 10);
}

void Dialog::mediaChanged(QMediaContent content)
{
//    qDebug() << "Media Changed";
}

void Dialog::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
//    qDebug() << status;
    switch(status)
    {
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferedMedia:
    {
//        qDebug() << "BufferedMedia";
#if defined(Q_OS_LINUX)
        ui->lTitle->setText(s_strlist.at(s_playlist->currentIndex()));
#elif defined(Q_OS_WIN)
        ui->lTitle->setText(s_player->metaData(QMediaMetaData::Author).toStringList().at(0)
                            + "-"
                            + s_player->metaData(QMediaMetaData::Title).toString());
#endif

//        ui->teInfo->setPlainText("       Title: " + s_player->metaData(QMediaMetaData::Title).toString());
//        ui->teInfo->append("      Author: " + s_player->metaData(QMediaMetaData::Author).toStringList().at(0));
//        ui->teInfo->append("     Comment: " + s_player->metaData(QMediaMetaData::Comment).toString());
//        ui->teInfo->append("Description : " + s_player->metaData(QMediaMetaData::Description).toString());
//        ui->teInfo->append("    Language: " + s_player->metaData(QMediaMetaData::Language).toString());
//        ui->teInfo->append("   Publisher: " + s_player->metaData(QMediaMetaData::Publisher).toString());
//        ui->teInfo->append("   Copyright: " + s_player->metaData(QMediaMetaData::Copyright).toString());
//        ui->teInfo->append("   MediaType: " + s_player->metaData(QMediaMetaData::MediaType).toString());
    }
        break;
    case QMediaPlayer::EndOfMedia:
//        if(true == s_rotation_flag)
//        {
//            s_player->play();
//        }
        break;
    case QMediaPlayer::PlayingState:
//        qDebug() << "PlayingState";
        break;
    default:
        break;
    }
}

void Dialog::changedMediaFromList(QModelIndex index)
{
    s_playlist->setCurrentIndex(index.row());
    s_player->play();

    s_play_flag = true;
    ui->pbPlay->setIcon(QIcon(":/img/btn/4.png"));
}

void Dialog::playlistIndexChanged(int row)
{
    if(-1 == row)
    {
        return;
    }
    QModelIndex qmindex = s_model->index(row, 0);
    ui->lvList->setCurrentIndex(qmindex);
}

void Dialog::updateVol(int val)
{
    s_player->setVolume(val);
}

void Dialog::on_pbPlay_clicked()
{
    if(false == s_play_flag)
    {
        s_play_flag = true;
        ui->pbPlay->setIcon(QIcon(":/img/btn/4.png"));

        s_player->play();
    }
    else
    {
        s_play_flag = false;
        ui->pbPlay->setIcon(QIcon(":/img/btn/6.png"));

        s_player->pause();
    }
}

void Dialog::on_pbPrevious_clicked()
{
    s_playlist->previous();
    s_player->play();

    s_play_flag = true;
    ui->pbPlay->setIcon(QIcon(":/img/btn/4.png"));
}

void Dialog::on_pbNext_clicked()
{
    s_playlist->next();
    s_player->play();

    s_play_flag = true;
    ui->pbPlay->setIcon(QIcon(":/img/btn/4.png"));
}

void Dialog::on_pbStop_clicked()
{
    s_player->stop();

    s_play_flag = false;
    ui->pbPlay->setIcon(QIcon(":/img/btn/6.png"));
}

void Dialog::on_pbRotation_clicked()
{
    if(false == s_rotation_flag)
    {
        s_rotation_flag = true;
        s_playlist->setPlaybackMode(QMediaPlaylist::Random);
        ui->pbRotation->setIcon(QIcon(":/img/btn/10-2.png"));
    }
    else
    {
        s_rotation_flag = false;
        s_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        ui->pbRotation->setIcon(QIcon(":/img/btn/10.png"));
    }
}

void Dialog::on_pbAddSong_clicked()
{
//    QStringList music_path = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    QStringList song_paths = QFileDialog::getOpenFileNames(this,
                                                          tr("添加曲目"),
//                                                          music_path.isEmpty() ? QDir::homePath() : music_path.first(),
                                                          "F:\\Music",
                                                          tr("MP3 files (*.mp3);WMA files (*.wma);;All files(*.*)"));
    foreach(QString song_path, song_paths)
    {
        ++s_num;
        s_strlist << QString::number(s_num) + ". " + QFileInfo(song_path).baseName();

        s_playlist->addMedia(QUrl::fromLocalFile(song_path));
    }

    s_model->setStringList(s_strlist);

    QModelIndex qmindex = s_model->index(s_playlist->currentIndex(), 0);
    ui->lvList->setCurrentIndex(qmindex);
}

void Dialog::on_pbAddList_clicked()
{
////    QStringList music_path = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
//    s_pl_path = QFileDialog::getOpenFileName(this,
//                                           tr("添加曲目"),
////                                           music_path.isEmpty() ? QDir::homePath() : music_path.first(),
//                                           "F:\\Music",
//                                           tr("Play List (*.hmpl)"));
//    s_pl_strlist << QFileInfo(s_pl_path).baseName();
//    s_pl_model->setStringList(s_pl_strlist);
//    ui->lvPlayList->setModel(s_pl_model);

    // Add PlayList to TableList
}

void Dialog::on_pbDel_clicked()
{

}

void Dialog::on_pbList_clicked()
{
    return;

    if(false == s_list_visible_flag)
    {
        s_list_visible_flag = true;
        ui->wList->setVisible(true);

//        setFixedHeight(s_win_height);
//        resize(width(), s_win_height);
    }
    else
    {
        s_list_visible_flag = false;
        ui->wList->setVisible(false);

        s_win_height = width();
        qDebug() << s_win_height;
        setFixedHeight(150);
    }
}
