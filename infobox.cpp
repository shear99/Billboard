#include "infobox.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTimer>
#include <QPalette>
#include <QFont>
#include <QRandomGenerator>
#include <QDebug>

InfoBox::InfoBox(QWidget *parent)
    : QWidget(parent)
    , m_currentTemp(20)
    , m_currentWeather("맑음")
{
    setupUI();

    // 업데이트 타이머 설정 (1초마다)
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &InfoBox::updateInfo);
    m_updateTimer->start(1000);

    // 초기 정보 표시
    updateInfo();
}

InfoBox::~InfoBox()
{
}

void InfoBox::setupUI()
{
    // 배경색 설정 (네이비 블루)
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(25, 35, 65));
    setPalette(palette);
    setAutoFillBackground(true);

    // 메인 레이아웃
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(20, 20, 20, 20);
    m_layout->setSpacing(20);

    // 날짜/시간 레이블
    m_dateTimeLabel = new QLabel(this);
    m_dateTimeLabel->setAlignment(Qt::AlignCenter);
    QFont dateFont;
    dateFont.setPointSize(18);
    dateFont.setBold(true);
    m_dateTimeLabel->setFont(dateFont);
    m_dateTimeLabel->setStyleSheet("QLabel { color: white; background-color: rgba(0, 0, 0, 50); padding: 10px; border-radius: 5px; }");

    // 날씨 레이블
    m_weatherLabel = new QLabel(this);
    m_weatherLabel->setAlignment(Qt::AlignCenter);
    QFont weatherFont;
    weatherFont.setPointSize(24);
    weatherFont.setBold(true);
    m_weatherLabel->setFont(weatherFont);
    m_weatherLabel->setStyleSheet("QLabel { color: #FFD700; }");

    // 온도 레이블
    m_temperatureLabel = new QLabel(this);
    m_temperatureLabel->setAlignment(Qt::AlignCenter);
    QFont tempFont;
    tempFont.setPointSize(36);
    tempFont.setBold(true);
    m_temperatureLabel->setFont(tempFont);
    m_temperatureLabel->setStyleSheet("QLabel { color: white; }");

    // 추가 정보 레이블
    m_additionalInfoLabel = new QLabel(this);
    m_additionalInfoLabel->setAlignment(Qt::AlignLeft);
    m_additionalInfoLabel->setWordWrap(true);
    QFont infoFont;
    infoFont.setPointSize(12);
    m_additionalInfoLabel->setFont(infoFont);
    m_additionalInfoLabel->setStyleSheet("QLabel { color: #E0E0E0; background-color: rgba(0, 0, 0, 30); padding: 15px; border-radius: 5px; }");

    // 레이아웃에 위젯 추가
    m_layout->addWidget(m_dateTimeLabel);
    m_layout->addWidget(m_weatherLabel);
    m_layout->addWidget(m_temperatureLabel);
    m_layout->addStretch(1);
    m_layout->addWidget(m_additionalInfoLabel);

    setLayout(m_layout);
}

void InfoBox::updateInfo()
{
    // 현재 날짜와 시간
    QDateTime now = QDateTime::currentDateTime();
    QString dateStr = now.toString("MM월 dd일");
    QString timeStr = now.toString("hh:mm:ss");
    QString dayStr = now.toString("dddd");

    // 한글 요일 변환
    QMap<QString, QString> dayMap;
    dayMap["Monday"] = "월요일";
    dayMap["Tuesday"] = "화요일";
    dayMap["Wednesday"] = "수요일";
    dayMap["Thursday"] = "목요일";
    dayMap["Friday"] = "금요일";
    dayMap["Saturday"] = "토요일";
    dayMap["Sunday"] = "일요일";

    QString koreanDay = dayMap.value(dayStr, dayStr);

    m_dateTimeLabel->setText(QString("%1\n%2\n%3").arg(dateStr).arg(koreanDay).arg(timeStr));

    // 날씨 정보 업데이트 (1분마다 변경)
    static int weatherUpdateCounter = 0;
    weatherUpdateCounter++;

    if (weatherUpdateCounter % 60 == 0) {
        // 날씨 랜덤 변경
        QStringList weatherTypes;
        weatherTypes << "맑음" << "구름 조금" << "흐림" << "비" << "눈";
        
        // Qt5에서 랜덤 생성 (QRandomGenerator가 없는 경우 대비)
        #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            int randomIndex = QRandomGenerator::global()->bounded(weatherTypes.size());
            m_currentTemp = QRandomGenerator::global()->bounded(-5, 36);
        #else
            qsrand(QTime::currentTime().msec());
            int randomIndex = qrand() % weatherTypes.size();
            m_currentTemp = qrand() % 41 - 5; // -5 ~ 35도
        #endif
        
        m_currentWeather = weatherTypes[randomIndex];
    }

    m_weatherLabel->setText(m_currentWeather);
    m_temperatureLabel->setText(QString("%1°C").arg(m_currentTemp));

    // 추가 정보
    #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        int humidity = QRandomGenerator::global()->bounded(40, 81);
        int windSpeed = QRandomGenerator::global()->bounded(1, 11);
        bool airQuality = QRandomGenerator::global()->bounded(0, 100) < 50;
    #else
        int humidity = qrand() % 41 + 40; // 40-80%
        int windSpeed = qrand() % 10 + 1; // 1-10 m/s
        bool airQuality = (qrand() % 100) < 50;
    #endif

    QString additionalInfo = QString(
                                 "오늘의 정보\n\n"
                                 "습도: %1%\n"
                                 "풍속: %2 m/s\n"
                                 "미세먼지: %3\n\n"
                                 "일출: 06:30\n"
                                 "일몰: 18:45\n\n"
                                 "공지사항:\n"
                                 "- 시스템 정상 작동 중\n"
                                 "- 비디오 재생 중"
                                 ).arg(humidity)
                                 .arg(windSpeed)
                                 .arg(airQuality ? "좋음" : "보통");

    m_additionalInfoLabel->setText(additionalInfo);
}

QString InfoBox::getWeatherInfo()
{
    // 실제 구현에서는 날씨 API를 호출하여 정보를 가져옴
    // 여기서는 더미 데이터 반환
    return m_currentWeather;
}