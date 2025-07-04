#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QFileSystemWatcher>
#include <QUrl>

class QMediaPlayer;
class QVideoWidget;
class QAudioOutput;

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
    QAudioOutput *m_audioOutput;
    QFileSystemWatcher *m_watcher;

    QString m_videoPath;
    QStringList m_currentFiles;
    QStringList m_videoFiles;
    int m_currentIndex;
};

#endif // VIDEOPLAYER_H