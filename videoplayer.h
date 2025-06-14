#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QFileSystemWatcher>
#include <QUrl>

class QMediaPlayer;
class QVideoWidget;

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

private slots:
    void onDirectoryChanged(const QString &path);
    void onMediaStatusChanged();
    void onPlaybackStateChanged();
    void handleError();

private:
    void loadVideos();
    void setupWatcher();
    void playNext();

    QMediaPlayer *m_player;
    QVideoWidget *m_videoWidget;
    QFileSystemWatcher *m_watcher;

    QString m_videoPath;
    QList<QUrl> m_playlist;
    int m_currentIndex;
    QStringList m_currentFiles;
};

#endif // VIDEOPLAYER_H
