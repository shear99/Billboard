#ifndef INFOBOX_H
#define INFOBOX_H

#include <QWidget>
#include <QTimer>

class QLabel;
class QVBoxLayout;

class InfoBox : public QWidget
{
    Q_OBJECT

public:
    explicit InfoBox(QWidget *parent = nullptr);
    ~InfoBox();

private slots:
    void updateInfo();

private:
    void setupUI();
    QString getWeatherInfo();

    QLabel *m_dateTimeLabel;
    QLabel *m_weatherLabel;
    QLabel *m_temperatureLabel;
    QLabel *m_additionalInfoLabel;

    QTimer *m_updateTimer;
    QVBoxLayout *m_layout;

    // 날씨 시뮬레이션을 위한 변수
    int m_currentTemp;
    QString m_currentWeather;
};

#endif // INFOBOX_H