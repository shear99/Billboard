#include "videoplayer.h"
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QVBoxLayout>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QApplication>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QAudioOutput>

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
    m_watcher = new QFileSystemWatcher(this);

    // 오디오 출력 설정 (Qt6에서 필요)
    QAudioOutput *audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(0.5);  // 볼륨 50%로 설정
    m_player->setAudioOutput(audioOutput);

    // 플레이어 설정
    m_player->setVideoOutput(m_videoWidget);

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

    // 에러 처리 연결 (Qt6 방식)
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, &VideoPlayer::handleError);

    // 미디어 상태 변경 감지
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &VideoPlayer::onMediaStatusChanged);

    // 재생 상태 변경 감지
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &VideoPlayer::onPlaybackStateChanged);

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
    // 현재 재생 중인 미디어 정보 저장
    qint64 currentPosition = m_player->position();
    bool wasPlaying = (m_player->playbackState() == QMediaPlayer::PlayingState);

    // 재생 목록 초기화
    m_playlist.clear();
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
        m_playlist.append(QUrl::fromLocalFile(filePath));
        m_currentFiles.append(fileInfo.fileName());
        qDebug() << "재생 목록 추가:" << fileInfo.fileName();
    }

    // 이전 재생 위치 복원 시도
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size()) {
        m_player->setSource(m_playlist[m_currentIndex]);
        if (currentPosition > 0) {
            // 위치 복원을 위해 약간의 지연 후 실행
            QTimer::singleShot(100, [this, currentPosition, wasPlaying]() {
                m_player->setPosition(currentPosition);
                if (wasPlaying) {
                    m_player->play();
                }
            });
        } else if (wasPlaying) {
            m_player->play();
        }
    } else {
        // 첫 번째 비디오부터 재생
        m_currentIndex = 0;
        if (!m_playlist.isEmpty()) {
            m_player->setSource(m_playlist[0]);
            QTimer::singleShot(100, [this]() {
                m_player->play();
                qDebug() << "재생 시작:" << m_playlist[0].fileName();
                qDebug() << "재생 상태:" << m_player->playbackState();
                qDebug() << "미디어 상태:" << m_player->mediaStatus();
            });
        }
    }
}

void VideoPlayer::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path);

    // 디렉토리 변경 감지 시 약간의 지연 후 파일 목록 갱신
    QTimer::singleShot(500, this, [this]() {
        QDir videoDir(m_videoPath);
        QStringList filters;
        filters << "*.mp4" << "*.avi" << "*.mkv" << "*.mov" << "*.wmv" << "*.flv";

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
    qDebug() << "미디어 상태 변경:" << m_player->mediaStatus();

    // 미디어가 끝까지 재생된 경우
    if (m_player->mediaStatus() == QMediaPlayer::EndOfMedia) {
        playNext();
    }
    // 미디어 로드 실패 시 다음 비디오로 넘어가기
    else if (m_player->mediaStatus() == QMediaPlayer::InvalidMedia) {
        qWarning() << "비디오 로드 실패, 다음 비디오로 이동";
        playNext();
    }
    // 미디어가 로드되고 준비되면 재생
    else if (m_player->mediaStatus() == QMediaPlayer::LoadedMedia) {
        if (m_player->playbackState() != QMediaPlayer::PlayingState) {
            m_player->play();
            qDebug() << "비디오 재생 시작";
        }
    }
}

void VideoPlayer::onPlaybackStateChanged()
{
    // 정지 상태가 되면 다음 비디오 재생
    if (m_player->playbackState() == QMediaPlayer::StoppedState
        && m_player->mediaStatus() == QMediaPlayer::EndOfMedia) {
        playNext();
    }
}

void VideoPlayer::playNext()
{
    if (m_playlist.isEmpty()) return;

    // 다음 인덱스로 이동 (순환)
    m_currentIndex = (m_currentIndex + 1) % m_playlist.size();
    m_player->setSource(m_playlist[m_currentIndex]);
    m_player->play();
}

void VideoPlayer::handleError()
{
    qWarning() << "미디어 플레이어 에러:" << m_player->errorString();

    // 에러 발생 시 다음 비디오 재생 시도
    if (!m_playlist.isEmpty() && m_playlist.size() > 1) {
        playNext();
    }
}
