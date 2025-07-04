#include "videoplayer.h"
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
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
    m_audioOutput = new QAudioOutput(this);
    m_watcher = new QFileSystemWatcher(this);

    // 플레이어 설정
    m_player->setVideoOutput(m_videoWidget);
    m_player->setAudioOutput(m_audioOutput);

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

    // 시그널 연결 (Qt6 방식)
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, &VideoPlayer::handleError);

    // 미디어 상태 변경 감지
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &VideoPlayer::onMediaStatusChanged);

    // 재생 상태 변경 감지
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &VideoPlayer::onStateChanged);

    // 재생 완료 시 다음 비디오 재생
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia) {
                    playNext();
                }
            });

    // 위치 변경 감지 (디버깅용)
    //connect(m_player, SIGNAL(positionChanged(qint64)),this, SLOT(onPositionChanged(qint64)));

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
    // 비디오 파일 목록 초기화
    m_videoFiles.clear();
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

    // 파일들을 목록에 추가
    for (const QFileInfo &fileInfo : files) {
        QString filePath = fileInfo.absoluteFilePath();
        m_videoFiles.append(filePath);
        m_currentFiles.append(fileInfo.fileName());
        qDebug() << "재생 목록 추가:" << fileInfo.fileName();
    }

    // 첫 번째 파일 재생 시작
    if (!m_videoFiles.isEmpty()) {
        m_currentIndex = 0;
        m_audioOutput->setVolume(0.5); // 볼륨 50%로 설정
        m_player->setSource(QUrl::fromLocalFile(m_videoFiles[m_currentIndex]));
        m_player->play();
        qDebug() << "재생 시작, 총" << m_videoFiles.size() << "개 파일";
    }
}

void VideoPlayer::playNext()
{
    if (m_videoFiles.isEmpty()) return;
    
    m_currentIndex = (m_currentIndex + 1) % m_videoFiles.size();
    m_player->setSource(QUrl::fromLocalFile(m_videoFiles[m_currentIndex]));
    m_player->play();
    qDebug() << "다음 비디오 재생:" << QFileInfo(m_videoFiles[m_currentIndex]).fileName();
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
    QMediaPlayer::PlaybackState state = m_player->playbackState();
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

void VideoPlayer::handleError()
{
    qWarning() << "비디오 재생 오류:" << m_player->errorString();
}

void VideoPlayer::onPositionChanged(qint64 position)
{
    Q_UNUSED(position);
    // 디버깅용 함수
}