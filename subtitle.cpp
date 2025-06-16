#include "subtitle.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QPalette>
#include <QFont>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QTextCodec>

Subtitle::Subtitle(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(0)
{
    setupUI();

    // 디스플레이 타이머 설정 (5초마다 텍스트 변경)
    m_displayTimer = new QTimer(this);
    connect(m_displayTimer, &QTimer::timeout, this, &Subtitle::showNextText);

    // 파일 감시자 설정
    m_fileWatcher = new QFileSystemWatcher(this);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &Subtitle::onFileChanged);

    // 자막 파일 경로 설정
    m_subtitlePath = QApplication::applicationDirPath() + "/text/subtitle.txt";

    // text 폴더가 없으면 생성
    QDir dir(QApplication::applicationDirPath() + "/text");
    if (!dir.exists()) {
        dir.mkpath(".");
        qDebug() << "text 디렉토리 생성됨:" << dir.absolutePath();

        // 예제 파일 생성
        QFile file(m_subtitlePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            // Qt5에서 UTF-8 설정
            stream.setCodec("UTF-8");
            stream << "시스템이 정상적으로 작동하고 있습니다. 모든 기능이 활성화되었습니다.\n";
            stream << "오늘의 날씨는 맑고 화창합니다. 외출하기 좋은 날씨입니다.\n";
            stream << "새로운 공지사항이 업데이트되었습니다. 확인 부탁드립니다.\n";
            stream << "안전 운전하시고 즐거운 하루 되세요.\n";
            stream << "현재 시스템 점검 중입니다. 잠시만 기다려 주세요.\n";
            file.close();
            qDebug() << "예제 subtitle.txt 파일 생성됨";
        }
    }

    // 파일 감시 시작
    if (QFile::exists(m_subtitlePath)) {
        m_fileWatcher->addPath(m_subtitlePath);
    }

    // 초기 자막 로드
    loadSubtitles();
    start();
}

Subtitle::~Subtitle()
{
}

void Subtitle::setupUI()
{
    // 배경색 설정 (진한 회색)
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(30, 30, 30));
    setPalette(palette);
    setAutoFillBackground(true);

    // 레이블 생성 및 설정
    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);  // 중앙 정렬
    m_label->setWordWrap(true);  // 긴 텍스트 자동 줄바꿈

    // 폰트 설정
    QFont font = m_label->font();
    font.setPointSize(24);
    font.setBold(true);
    m_label->setFont(font);

    // 텍스트 색상 설정 (흰색)
    QPalette labelPalette = m_label->palette();
    labelPalette.setColor(QPalette::WindowText, Qt::white);
    m_label->setPalette(labelPalette);

    // 레이아웃 설정
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 10, 20, 10);
    layout->addWidget(m_label);
    setLayout(layout);

    // 최소/최대 높이 설정 (드래그로 조절 가능하도록)
    setMinimumHeight(30);
    setMaximumHeight(200);  // 최대 200px까지 늘릴 수 있음

    // 투명도 효과를 위한 설정
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
    m_label->setGraphicsEffect(effect);
}

void Subtitle::loadSubtitles()
{
    QFile file(m_subtitlePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "subtitle.txt 파일을 열 수 없습니다:" << m_subtitlePath;
        // 기본 메시지 설정
        m_textList.clear();
        m_textList << "자막 파일을 찾을 수 없습니다. text/subtitle.txt 파일을 확인하세요.";
        return;
    }

    m_textList.clear();
    QTextStream stream(&file);
    // Qt5에서 UTF-8 설정
    stream.setCodec("UTF-8");

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (!line.isEmpty()) {  // 빈 줄은 무시
            m_textList.append(line);
        }
    }

    file.close();

    if (m_textList.isEmpty()) {
        m_textList << "자막 파일이 비어있습니다.";
    }

    qDebug() << "자막 로드됨:" << m_textList.size() << "개 문장";

    // 현재 인덱스 리셋
    m_currentIndex = 0;
}

void Subtitle::onFileChanged(const QString &path)
{
    Q_UNUSED(path);
    qDebug() << "subtitle.txt 파일 변경 감지";

    // 파일이 수정되면 다시 로드
    QTimer::singleShot(100, this, [this]() {
        loadSubtitles();
        // 파일 감시 다시 시작 (파일이 수정되면 감시가 해제될 수 있음)
        if (QFile::exists(m_subtitlePath)) {
            m_fileWatcher->addPath(m_subtitlePath);
        }
    });
}

void Subtitle::setTextList(const QStringList &textList)
{
    m_textList = textList;
    m_currentIndex = 0;
}

void Subtitle::start()
{
    if (!m_textList.isEmpty()) {
        showNextText();
        m_displayTimer->start(5000);  // 5초마다 변경
    }
}

void Subtitle::stop()
{
    m_displayTimer->stop();
}

void Subtitle::showNextText()
{
    if (m_textList.isEmpty()) return;

    // 페이드 아웃 애니메이션
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect*>(m_label->graphicsEffect());
    if (effect) {
        QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity");
        fadeOut->setDuration(500);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);

        // 페이드 아웃 완료 후 텍스트 변경 및 페이드 인
        connect(fadeOut, &QPropertyAnimation::finished, [this, effect]() {
            // 텍스트 변경
            m_label->setText(m_textList[m_currentIndex]);
            m_currentIndex = (m_currentIndex + 1) % m_textList.size();

            // 페이드 인 애니메이션
            QPropertyAnimation *fadeIn = new QPropertyAnimation(effect, "opacity");
            fadeIn->setDuration(500);
            fadeIn->setStartValue(0.0);
            fadeIn->setEndValue(1.0);
            fadeIn->start(QPropertyAnimation::DeleteWhenStopped);
        });

        fadeOut->start(QPropertyAnimation::DeleteWhenStopped);
    }
}

void Subtitle::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 크기 변경 시 특별한 처리 불필요 (레이아웃이 자동 처리)
}