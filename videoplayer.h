#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QFileSystemWatcher>
#include <QUrl>

class QMediaPlayer;
class QVideoWidget;
class QMediaPlaylist;

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

private slots:
    void onDirectoryChanged(const QString &path);
    void onMediaStatusChanged();
    void onStateChanged();
    void handleError();
    void onPositionChanged(qint64 position);

private:
    void loadVideos();
    void setupWatcher();
    void playNext();

    QMediaPlayer *m_player;
    QVideoWidget *m_videoWidget;
    QMediaPlaylist *m_playlist;
    QFileSystemWatcher *m_watcher;

    QString m_videoPath;
    QStringList m_currentFiles;
    int m_currentIndex;
};

#endif // VIDEOPLAYER_H