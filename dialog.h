#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStringListModel>
#include <QSettings>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void on_pbPlay_clicked();

    void on_pbPrevious_clicked();

    void on_pbNext_clicked();

private:
    Ui::Dialog *ui;

    QMediaPlayer *s_player;
    QMediaPlaylist *s_playlist;
    QStringList s_pl_strlist;
    QStringList s_strlist;
    QStringListModel *s_pl_model;
    QStringListModel *s_model;
    bool s_play_flag = false;
    bool s_rotation_flag = false;
    bool s_list_visible_flag = true;
    int s_num = 0;
    int s_win_height = 0;
    QString s_pl_path;
    QSettings *s_settings;

    virtual void closeEvent(QCloseEvent *evt);

private:
    void setupUI(void);

private slots:
    void updatePosition(qint64 pos);
    void updateDuration(qint64 dur);
    void on_pbStop_clicked();
    void mediaChanged(QMediaContent content);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void on_pbRotation_clicked();
    void on_pbAddSong_clicked();
    void on_pbAddList_clicked();
    void changedMediaFromList(QModelIndex index);
    void on_pbDel_clicked();
    void playlistIndexChanged(int row);
    void updateVol(int val);
    void on_pbList_clicked();
};

#endif // DIALOG_H
