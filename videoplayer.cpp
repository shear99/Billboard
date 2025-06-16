#include "videoplayer.h"
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMediaPlaylist>
#include <QVBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>
#include <QFileSystemWatcher>
#include <QTimer>

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(0)
{
    // 검은색 배경 설정
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    setPalette(p);
    setAutoFillBackground(true);

    // 미디어 플레이어 구성요소 초기화
    m_player = new QMediaPlayer(this);
    m_videoWidget = new QVideoWidget(this);
    m_playlist = new QMediaPlaylist(this);
    m_watcher = new QFileSystemWatcher(this);

    // 플레이어 설정
    m_player->setVideoOutput(m_videoWidget);
    m_player->setPlaylist(m_playlist);

    // 플레이리스트 설정 (반복 재생)
    m_playlist->setPlaybackMode(QMediaPlaylist::Loop);

    // 레이아웃 설정
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_videoWidget);
    setLayout(layout);

    // 비디오 화면 비율 설정 - 16:9 비율 유지하면서 검은 여백 표시
    m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);

    // 비디오 위젯이 보이도록 설정
    m_videoWidget->show();

    // 비디오 경로 설정
    m_videoPath = QApplication::applicationDirPath() + "/video";

    // 디렉토리가 없으면 생성
    QDir dir(m_videoPath);
    if (!dir.exists()) {
        dir.mkpath(".");
        qDebug() << "video 디렉토리 생성됨:" << m_videoPath;
    }

    // 파일 시스템 감시 설정
    setupWatcher();

    // 시그널 연결 (Qt5 방식 - SIGNAL/SLOT 매크로 사용)
    connect(m_player, SIGNAL(error(QMediaPlayer::Error)),
            this, SLOT(handleError()));

    // 미디어 상태 변경 감지
    connect(m_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(onMediaStatusChanged()));

    // 재생 상태 변경 감지
    connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(onStateChanged()));

    // 위치 변경 감지 (디버깅용)
    connect(m_player, SIGNAL(positionChanged(qint64)),
            this, SLOT(onPositionChanged(qint64)));

    // 초기 비디오 로드 (약간의 지연 후)
    QTimer::singleShot(100, this, &VideoPlayer::loadVideos);
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::setupWatcher()
{
    // 디렉토리 변경 감지
    m_watcher->addPath(m_videoPath);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &VideoPlayer::onDirectoryChanged);
}

void VideoPlayer::loadVideos()
{
    // 플레이리스트 초기화
    m_playlist->clear();
    m_currentFiles.clear();

    // 비디오 파일 검색
    QDir videoDir(m_videoPath);
    QStringList filters;
    filters << "*.mp4" << "*.avi" << "*.mkv" << "*.mov" << "*.wmv" << "*.flv" << "*.MP4" << "*.MOV";

    QFileInfoList files = videoDir.entryInfoList(filters, QDir::Files, QDir::Name);

    if (files.isEmpty()) {
        qWarning() << "경고: video 폴더에 비디오 파일이 없습니다.";
        return;
    }

    // 파일들을 재생 목록에 추가
    for (const QFileInfo &fileInfo : files) {
        QString filePath = fileInfo.absoluteFilePath();
        m_playlist->addMedia(QUrl::fromLocalFile(filePath));
        m_currentFiles.append(fileInfo.fileName());
        qDebug() << "재생 목록 추가:" << fileInfo.fileName();
    }

    // 플레이리스트에 파일이 있으면 재생 시작
    if (m_playlist->mediaCount() > 0) {
        m_player->setVolume(50); // 볼륨 50%로 설정
        m_player->play();
        qDebug() << "재생 시작, 총" << m_playlist->mediaCount() << "개 파일";
    }
}

void VideoPlayer::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path);

    // 디렉토리 변경 감지 시 약간의 지연 후 파일 목록 갱신
    QTimer::singleShot(500, this, [this]() {
        QDir videoDir(m_videoPath);
        QStringList filters;
        filters << "*.mp4" << "*.avi" << "*.mkv" << "*.mov" << "*.wmv" << "*.flv" << "*.MP4" << "*.MOV";

        QFileInfoList files = videoDir.entryInfoList(filters, QDir::Files, QDir::Name);
        QStringList newFiles;

        for (const QFileInfo &fileInfo : files) {
            newFiles.append(fileInfo.fileName());
        }

        // 파일 목록이 변경되었는지 확인
        if (newFiles != m_currentFiles) {
            qDebug() << "비디오 파일 목록 변경 감지, 재로드 중...";
            loadVideos();
        }
    });
}

void VideoPlayer::onMediaStatusChanged()
{
    QMediaPlayer::MediaStatus status = m_player->mediaStatus();
    qDebug() << "미디어 상태 변경:" << status;

    switch (status) {
    case QMediaPlayer::LoadedMedia:
        qDebug() << "미디어 로드 완료";
        break;
    case QMediaPlayer::InvalidMedia:
        qWarning() << "비디오 로드 실패:" << m_player->errorString();
        break;
    case QMediaPlayer::EndOfMedia:
        qDebug() << "미디어 재생 종료";
        break;
    default:
        break;
    }
}

void VideoPlayer::onStateChanged()
{
    QMediaPlayer::State state = m_player->state();
    qDebug() << "재생 상태 변경:" << state;

    switch (state) {
    case QMediaPlayer::PlayingState:
        qDebug() << "재생 중";
        break;
    case QMediaPlayer::PausedState:
        qDebug() << "일시정지";
        break;
    case QMediaPlayer::StoppedState:
        qDebug() << "정지";
        break;
    }
}

void VideoPlayer::onPositionChanged(qint64 position)
{
    // 디버깅용 - 매 10초마다 위치 출력
    static qint64 lastLogTime = 0;
    if (position - lastLogTime > 10000) {
        lastLogTime = position;
        qDebug() << "재생 위치:" << position / 1000 << "초";
    }
}

void VideoPlayer::playNext()
{
    // Qt5에서는 플레이리스트가 자동으로 다음 파일을 재생함
    m_playlist->next();
}

void VideoPlayer::handleError()
{
    QMediaPlayer::Error error = m_player->error();
    qWarning() << "미디어 플레이어 에러:" << error << "-" << m_player->errorString();

    // 에러 발생 시 다음 비디오 재생 시도
    if (m_playlist->mediaCount() > 1) {
        QTimer::singleShot(1000, this, &VideoPlayer::playNext);
    }
}