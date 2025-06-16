#ifndef INFOBOX_H
#define INFOBOX_H

#include <QWidget>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QTime>
#include <QMap>
#include <QQueue>

class QLabel;
class QVBoxLayout;

struct ServiceInfo {
    QString name;
    int floor;
    QTime startTime;
    QTime endTime;
    
    ServiceInfo() {}
    ServiceInfo(const QString &n, int f, const QTime &start, const QTime &end)
        : name(n), floor(f), startTime(start), endTime(end) {}
};

class InfoBox : public QWidget
{
    Q_OBJECT

public:
    explicit InfoBox(QWidget *parent = nullptr);
    ~InfoBox();

private slots:
    void updateInfo();
    void loadServiceSchedule();
    void onServiceFileChanged(const QString &path);

private:
    void setupUI();
    QString getWeatherInfo();
    QString getCurrentServiceInfo();
    void initializeServiceQueues();

    QLabel *m_dateTimeLabel;
    QLabel *m_weatherLabel;
    QLabel *m_temperatureLabel;
    QLabel *m_serviceInfoLabel;
    QLabel *m_additionalInfoLabel;

    QTimer *m_updateTimer;
    QVBoxLayout *m_layout;
    QFileSystemWatcher *m_serviceWatcher;

    // 날씨 시뮬레이션을 위한 변수
    int m_currentTemp;
    QString m_currentWeather;
    
    QTimer *m_updateTimer; // 기존 타이머는 시간 표시용으로 사용
    QTimer *m_slowUpdateTimer; // 날씨 등 느린 업데이트용 타이머
    QVBoxLayout *m_layout;

    // 예배 스케줄 관련
    QString m_serviceFilePath;
    QMap<int, QQueue<ServiceInfo> > m_floorServices;  // Qt5 호환성을 위해 공백 추가
    QList<ServiceInfo> m_allServices;  // 전체 예배 목록
};

#endif // INFOBOX_H