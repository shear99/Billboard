#include "infobox.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTimer>
#include <QPalette>
#include <QFont>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDir>
#include <QFileSystemWatcher>
#include <QRegExp>
#include <QTime>

// Qt5에서 QTextCodec 사용
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #include <QTextCodec>
#endif

// Qt5에서 랜덤 생성을 위한 호환성
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    #include <QRandomGenerator>
#else
    #include <QTime>
    #include <cstdlib>
#endif

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

    // 파일 감시자 설정
    m_serviceWatcher = new QFileSystemWatcher(this);
    connect(m_serviceWatcher, &QFileSystemWatcher::fileChanged,
            this, &InfoBox::onServiceFileChanged);

    // 예배 스케줄 파일 경로 설정
    m_serviceFilePath = QApplication::applicationDirPath() + "/text/infobox.txt";

    // text 폴더가 없으면 생성
    QDir dir(QApplication::applicationDirPath() + "/text");
    if (!dir.exists()) {
        dir.mkpath(".");
        qDebug() << "text 디렉토리 생성됨:" << dir.absolutePath();
    }

    // 예제 파일 생성
    if (!QFile::exists(m_serviceFilePath)) {
        QFile file(m_serviceFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            stream.setCodec("UTF-8");
#else
            stream.setEncoding(QStringConverter::Utf8);
#endif
            stream << "유아-유치부\t4층\t오전 9시\n";
            stream << "유초등부\t4층\t오전 9시\n";
            stream << "중고등부\t3층\t오전 9시\n";
            stream << "주일 2부\t2층\t오전 11시\n";
            stream << "주일 3부\t2층\t오후 2시\n";
            stream << "대학청년부\t3층\t오후 2시\n";
            file.close();
            qDebug() << "예제 infobox.txt 파일 생성됨";
        }
    }

    // 파일 감시 시작
    if (QFile::exists(m_serviceFilePath)) {
        m_serviceWatcher->addPath(m_serviceFilePath);
    }

    // 초기 예배 스케줄 로드
    loadServiceSchedule();

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

    // 예배 정보 레이블 (새로 추가)
    m_serviceInfoLabel = new QLabel(this);
    m_serviceInfoLabel->setAlignment(Qt::AlignLeft);
    m_serviceInfoLabel->setWordWrap(true);
    QFont serviceFont;
    serviceFont.setPointSize(14);
    serviceFont.setBold(true);
    m_serviceInfoLabel->setFont(serviceFont);
    m_serviceInfoLabel->setStyleSheet("QLabel { color: #87CEEB; background-color: rgba(0, 0, 0, 40); padding: 15px; border-radius: 5px; border: 2px solid #4682B4; }");

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
    m_layout->addWidget(m_serviceInfoLabel);  // 예배 정보 추가
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
        
        // Qt5에서 랜덤 생성
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

    // 예배 정보 업데이트
    QString serviceInfo = getCurrentServiceInfo();
    m_serviceInfoLabel->setText(serviceInfo);

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

void InfoBox::loadServiceSchedule()
{
    QFile file(m_serviceFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "infobox.txt 파일을 열 수 없습니다:" << m_serviceFilePath;
        return;
    }

    m_allServices.clear();
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#else
    stream.setEncoding(QStringConverter::Utf8);
#endif

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split('\t');
        if (parts.size() >= 3) {
            QString serviceName = parts[0];
            QString floorStr = parts[1];
            QString timeStr = parts[2];

            // 층수 추출 (예: "4층" -> 4)
            int floor = 0;
            QRegExp floorRegex("(\\d+)층");
            if (floorRegex.indexIn(floorStr) != -1) {
                floor = floorRegex.cap(1).toInt();
            }

            // 시간 파싱
            QTime startTime;
            if (timeStr.contains("오전")) {
                QString timeOnly = timeStr.replace("오전", "").replace("시", "").trimmed();
                int hour = timeOnly.toInt();
                if (hour == 12) hour = 0;  // 오전 12시는 0시
                startTime = QTime(hour, 0);
            } else if (timeStr.contains("오후")) {
                QString timeOnly = timeStr.replace("오후", "").replace("시", "").trimmed();
                int hour = timeOnly.toInt();
                if (hour != 12) hour += 12;  // 오후는 12시간 추가 (12시는 제외)
                startTime = QTime(hour, 0);
            }

            if (startTime.isValid() && floor > 0) {
                QTime endTime = startTime.addSecs(90 * 60);  // 1시간 30분 후
                ServiceInfo service(serviceName, floor, startTime, endTime);
                m_allServices.append(service);
            }
        }
    }

    file.close();
    initializeServiceQueues();
    qDebug() << "예배 스케줄 로드됨:" << m_allServices.size() << "개 예배";
}

void InfoBox::initializeServiceQueues()
{
    m_floorServices.clear();

    // 각 예배를 해당 층의 큐에 추가
    for (const ServiceInfo &service : m_allServices) {
        if (!m_floorServices.contains(service.floor)) {
            m_floorServices[service.floor] = QQueue<ServiceInfo>();
        }
        m_floorServices[service.floor].enqueue(service);
    }
}

QString InfoBox::getCurrentServiceInfo()
{
    QTime currentTime = QTime::currentTime();
    QString result = "🏛️ 현재 예배 현황\n\n";

    // 각 층별로 현재 예배 상태 확인
    QList<int> floors = m_floorServices.keys();
    
    // Qt5 호환 정렬
    #if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
        std::sort(floors.begin(), floors.end());
    #else
        qSort(floors);
    #endif

    bool hasActiveService = false;

    for (int i = 0; i < floors.size(); ++i) {
        int floor = floors[i];
        QQueue<ServiceInfo> &services = m_floorServices[floor];
        QString floorInfo = QString("%1층: ").arg(floor);

        bool foundCurrentService = false;

        // 현재 진행 중인 예배 찾기
        QQueue<ServiceInfo> tempQueue = services;
        while (!tempQueue.isEmpty()) {
            ServiceInfo service = tempQueue.dequeue();

            // 현재 시간이 예배 시간 범위 내인지 확인
            if (currentTime >= service.startTime && currentTime <= service.endTime) {
                // Qt5 호환성을 위한 시간 계산
                int currentSecs = currentTime.hour() * 3600 + currentTime.minute() * 60 + currentTime.second();
                int endSecs = service.endTime.hour() * 3600 + service.endTime.minute() * 60 + service.endTime.second();
                int secondsToEnd = endSecs - currentSecs;
                
                if (secondsToEnd > 0) {
                    int hours = secondsToEnd / 3600;
                    int minutes = (secondsToEnd % 3600) / 60;
                    floorInfo += QString("%1 (종료까지 %2:%3)")
                        .arg(service.name)
                        .arg(hours, 2, 10, QChar('0'))
                        .arg(minutes, 2, 10, QChar('0'));
                } else {
                    floorInfo += QString("%1 (곧 종료)")
                        .arg(service.name);
                }
                foundCurrentService = true;
                hasActiveService = true;
                break;
            }
            // 예배가 끝났으면 큐에서 제거
            else if (currentTime > service.endTime) {
                services.dequeue();
            }
        }

        // 현재 진행 중인 예배가 없으면 다음 예배 표시
        if (!foundCurrentService) {
            if (!services.isEmpty()) {
                ServiceInfo nextService = services.head();
                if (currentTime < nextService.startTime) {
                    floorInfo += QString("다음: %1 (%2)")
                        .arg(nextService.name)
                        .arg(nextService.startTime.toString("hh:mm"));
                }
            } else {
                floorInfo += "예배 없음";
            }
        }

        result += floorInfo + "\n";
    }

    if (!hasActiveService) {
        result += "\n📢 현재 진행중인 예배가 없습니다";
    }

    return result;
}

void InfoBox::onServiceFileChanged(const QString &path)
{
    Q_UNUSED(path);
    qDebug() << "infobox.txt 파일 변경 감지";

    // 파일이 수정되면 다시 로드
    QTimer::singleShot(100, this, [this]() {
        loadServiceSchedule();
        // 파일 감시 다시 시작
        if (QFile::exists(m_serviceFilePath)) {
            m_serviceWatcher->addPath(m_serviceFilePath);
        }
    });
}