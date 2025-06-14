#include "mainwindow.h"
#include "videoplayer.h"
#include "subtitle.h"
#include "infobox.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // 윈도우 설정
    setWindowTitle("Video Display System");

    // 검은색 배경 설정
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::black);
    setPalette(palette);
    setAutoFillBackground(true);

    // 컴포넌트 생성
    m_videoPlayer = new VideoPlayer(this);
    m_subtitle = new Subtitle(this);
    m_infoBox = new InfoBox(this);

    // 왼쪽 영역용 수직 스플리터 생성 (비디오 + 자막)
    m_leftSplitter = new QSplitter(Qt::Vertical, this);
    m_leftSplitter->addWidget(m_videoPlayer);
    m_leftSplitter->addWidget(m_subtitle);

    // 비디오와 자막의 초기 비율 설정 (9:1)
    m_leftSplitter->setStretchFactor(0, 9);  // 비디오
    m_leftSplitter->setStretchFactor(1, 1);  // 자막

    // 메인 수평 스플리터 생성 (왼쪽 영역 + 오른쪽 정보창)
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->addWidget(m_leftSplitter);
    m_mainSplitter->addWidget(m_infoBox);

    // 좌우 영역의 초기 비율 설정 (7:3)
    m_mainSplitter->setStretchFactor(0, 7);  // 왼쪽 영역
    m_mainSplitter->setStretchFactor(1, 3);  // 오른쪽 정보창

    // 스플리터 핸들 스타일 설정 (더 눈에 띄게)
    QString splitterStyle = R"(
        QSplitter::handle {
            background-color: #555;
        }
        QSplitter::handle:horizontal {
            width: 4px;
        }
        QSplitter::handle:vertical {
            height: 4px;
        }
        QSplitter::handle:hover {
            background-color: #888;
        }
    )";
    m_mainSplitter->setStyleSheet(splitterStyle);
    m_leftSplitter->setStyleSheet(splitterStyle);

    // 메인 레이아웃
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_mainSplitter);
    setLayout(layout);

    // 초기 크기 설정 (FHD 기준)
    resize(1920, 1080);

    // 16:9 비율을 위한 비디오 영역 크기 계산
    // 전체 높이에서 자막 높이(약 50px)를 뺀 나머지가 비디오 영역
    // 왼쪽 영역이 전체의 70%라고 가정하면: 1920 * 0.7 = 1344px
    // 16:9 비율로 높이 계산: 1344 / 16 * 9 = 756px
    // 실제로는 사용자가 조절 가능하므로 초기값만 설정

    QList<int> leftSizes;
    leftSizes << 900 << 100;  // 비디오 900px, 자막 100px
    m_leftSplitter->setSizes(leftSizes);

    QList<int> mainSizes;
    mainSizes << 1344 << 576;  // 왼쪽 70%, 오른쪽 30%
    m_mainSplitter->setSizes(mainSizes);
}

MainWindow::~MainWindow()
{
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // ESC 키로 전체화면 종료
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen()) {
            showNormal();
        }
    }
    // F11 키로 전체화면 토글
    else if (event->key() == Qt::Key_F11) {
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    }

    QWidget::keyPressEvent(event);
}
