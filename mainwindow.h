#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class VideoPlayer;
class Subtitle;
class InfoBox;
class QSplitter;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    VideoPlayer *m_videoPlayer;
    Subtitle *m_subtitle;
    InfoBox *m_infoBox;

    QSplitter *m_mainSplitter;      // 좌우 분할
    QSplitter *m_leftSplitter;      // 상하 분할
    QWidget *m_leftContainer;        // 왼쪽 영역 컨테이너
};

#endif // MAINWINDOW_H
