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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #include <QTextCodec>
#endif

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

    // --- 업데이트 로직 분리 ---
    // 1. 시간 표시 전용 타이머 (1초마다)
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &InfoBox::updateTime);
    m_updateTimer->start(1000);

    // 2. 정보 업데이트용 타이머 (60초마다)
    m_slowUpdateTimer = new QTimer(this);
    connect(m_slowUpdateTimer, &QTimer::timeout, this, &InfoBox::updateDetails);
    m_slowUpdateTimer->start(180000); // 60초

    // 파일 감시자 설정
    m_serviceWatcher = new QFileSystemWatcher(this);
    connect(m_serviceWatcher, &QFileSystemWatcher::fileChanged,
            this, &InfoBox::onServiceFileChanged);

    m_serviceFilePath = QApplication::applicationDirPath() + "/text/infobox.txt";

    QDir dir(QApplication::applicationDirPath() + "/text");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    if (!QFile::exists(m_serviceFilePath)) {
        QFile file(m_serviceFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            stream.setCodec("UTF-8");
#endif
            stream << "유아-유치부\t4층\t오전 9시\n";
            stream << "유초등부\t4층\t오전 9시\n";
            stream << "중고등부\t3층\t오전 9시\n";
            stream << "주일 2부\t2층\t오전 11시\n";
            stream << "주일 3부\t2층\t오후 2시\n";
            stream << "대학청년부\t3층\t오후 2시\n";
            file.close();
        }
    }

    if (QFile::exists(m_serviceFilePath)) {
        m_serviceWatcher->addPath(m_serviceFilePath);
    }

    loadServiceSchedule();

    // 초기 정보 표시를 위해 각 함수를 한 번씩 호출
    updateTime();
    updateDetails();
}

InfoBox::~InfoBox()
{
}

void InfoBox::setupUI()
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(25, 35, 65));
    setPalette(palette);
    setAutoFillBackground(true);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(20, 20, 20, 20);
    m_layout->setSpacing(20);

    m_dateTimeLabel = new QLabel(this);
    m_dateTimeLabel->setAlignment(Qt::AlignCenter);
    QFont dateFont;
    dateFont.setPointSize(18);
    dateFont.setBold(true);
    m_dateTimeLabel->setFont(dateFont);
    m_dateTimeLabel->setStyleSheet("QLabel { color: white; background-color: rgba(0, 0, 0, 50); padding: 10px; border-radius: 5px; }");

    m_weatherLabel = new QLabel(this);
    m_weatherLabel->setAlignment(Qt::AlignCenter);
    QFont weatherFont;
    weatherFont.setPointSize(24);
    weatherFont.setBold(true);
    m_weatherLabel->setFont(weatherFont);
    m_weatherLabel->setStyleSheet("QLabel { color: #FFD700; }");

    m_temperatureLabel = new QLabel(this);
    m_temperatureLabel->setAlignment(Qt::AlignCenter);
    QFont tempFont;
    tempFont.setPointSize(36);
    tempFont.setBold(true);
    m_temperatureLabel->setFont(tempFont);
    m_temperatureLabel->setStyleSheet("QLabel { color: white; }");

    m_serviceInfoLabel = new QLabel(this);
    m_serviceInfoLabel->setAlignment(Qt::AlignLeft);
    m_serviceInfoLabel->setWordWrap(true);
    QFont serviceFont;
    serviceFont.setPointSize(14);
    serviceFont.setBold(true);
    m_serviceInfoLabel->setFont(serviceFont);
    m_serviceInfoLabel->setStyleSheet("QLabel { color: #87CEEB; background-color: rgba(0, 0, 0, 40); padding: 15px; border-radius: 5px; border: 2px solid #4682B4; }");

    m_additionalInfoLabel = new QLabel(this);
    m_additionalInfoLabel->setAlignment(Qt::AlignLeft);
    m_additionalInfoLabel->setWordWrap(true);
    QFont infoFont;
    infoFont.setPointSize(12);
    m_additionalInfoLabel->setFont(infoFont);
    m_additionalInfoLabel->setStyleSheet("QLabel { color: #E0E0E0; background-color: rgba(0, 0, 0, 30); padding: 15px; border-radius: 5px; }");

    m_layout->addWidget(m_dateTimeLabel);
    m_layout->addWidget(m_weatherLabel);
    m_layout->addWidget(m_temperatureLabel);
    m_layout->addWidget(m_serviceInfoLabel);
    m_layout->addStretch(1);
    m_layout->addWidget(m_additionalInfoLabel);

    setLayout(m_layout);
}

// 1초마다 호출되는 시간 업데이트 함수
void InfoBox::updateTime()
{
    QDateTime now = QDateTime::currentDateTime();
    QString dateStr = now.toString("MM월 dd일");
    QString timeStr = now.toString("hh:mm:ss");
    QString dayStr = now.toString("dddd");

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
}

// 60초마다 호출되는 상세 정보 업데이트 함수
void InfoBox::updateDetails()
{
    // 날씨 정보 업데이트
    QStringList weatherTypes;
    weatherTypes << "맑음" << "구름 조금" << "흐림" << "비" << "눈";
    
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    int randomIndex = QRandomGenerator::global()->bounded(weatherTypes.size());
    m_currentTemp = QRandomGenerator::global()->bounded(-5, 36);
#else
    qsrand(QTime::currentTime().msec());
    int randomIndex = qrand() % weatherTypes.size();
    m_currentTemp = qrand() % 41 - 5;
#endif
    
    m_currentWeather = weatherTypes[randomIndex];
    m_weatherLabel->setText(m_currentWeather);
    m_temperatureLabel->setText(QString("%1°C").arg(m_currentTemp));

    // 예배 정보 업데이트
    QString serviceInfo = getCurrentServiceInfo();
    m_serviceInfoLabel->setText(serviceInfo);

    // 추가 정보 업데이트
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    int humidity = QRandomGenerator::global()->bounded(40, 81);
    int windSpeed = QRandomGenerator::global()->bounded(1, 11);
#else
    int humidity = qrand() % 41 + 40;
    int windSpeed = qrand() % 10 + 1;
#endif
    bool airQuality = (humidity < 60);

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
#endif

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split('\t');
        if (parts.size() >= 3) {
            QString serviceName = parts[0];
            QString floorStr = parts[1];
            QString timeStr = parts[2];

            int floor = 0;
            QRegExp floorRegex("(\\d+)층");
            if (floorRegex.indexIn(floorStr) != -1) {
                floor = floorRegex.cap(1).toInt();
            }

            QTime startTime;
            if (timeStr.contains("오전")) {
                QString timeOnly = timeStr.replace("오전", "").replace("시", "").trimmed();
                int hour = timeOnly.toInt();
                if (hour == 12) hour = 0;
                startTime = QTime(hour, 0);
            } else if (timeStr.contains("오후")) {
                QString timeOnly = timeStr.replace("오후", "").replace("시", "").trimmed();
                int hour = timeOnly.toInt();
                if (hour != 12) hour += 12;
                startTime = QTime(hour, 0);
            }

            if (startTime.isValid() && floor > 0) {
                QTime endTime = startTime.addSecs(90 * 60);
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

    QList<int> floors = m_floorServices.keys();
    
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    std::sort(floors.begin(), floors.end());
#else
    qSort(floors);
#endif

    bool hasActiveService = false;

    for (int floor : floors) {
        QQueue<ServiceInfo> &services = m_floorServices[floor];
        QString floorInfo = QString("%1층: ").arg(floor);

        bool foundCurrentService = false;
        
        // C++11 range-based for loop is fine with Qt5
        for(const auto& service : services) {
            if (currentTime >= service.startTime && currentTime <= service.endTime) {
                int currentSecs = QTime(0, 0).secsTo(currentTime);
                int endSecs = QTime(0, 0).secsTo(service.endTime);
                int secondsToEnd = endSecs - currentSecs;
                
                if (secondsToEnd > 0) {
                    int hours = secondsToEnd / 3600;
                    int minutes = (secondsToEnd % 3600) / 60;
                    floorInfo += QString("%1 (종료까지 %2:%3)")
                        .arg(service.name)
                        .arg(hours, 2, 10, QChar('0'))
                        .arg(minutes, 2, 10, QChar('0'));
                } else {
                    floorInfo += QString("%1 (곧 종료)").arg(service.name);
                }
                foundCurrentService = true;
                hasActiveService = true;
                break; 
            }
        }
        
        if (currentTime > services.head().endTime && !services.isEmpty()) {
            services.dequeue();
        }

        if (!foundCurrentService) {
            if (!services.isEmpty()) {
                const ServiceInfo &nextService = services.head();
                if (currentTime < nextService.startTime) {
                    floorInfo += QString("다음: %1 (%2)")
                        .arg(nextService.name)
                        .arg(nextService.startTime.toString("hh:mm"));
                } else {
                     floorInfo += "예배 없음";
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

    QTimer::singleShot(100, this, [this]() {
        loadServiceSchedule();
        if (QFile::exists(m_serviceFilePath)) {
            m_serviceWatcher->addPath(m_serviceFilePath);
        }
    });
}