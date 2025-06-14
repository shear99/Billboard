#ifndef SUBTITLE_H
#define SUBTITLE_H

#include <QWidget>
#include <QTimer>
#include <QStringList>
#include <QFileSystemWatcher>

class QLabel;

class Subtitle : public QWidget
{
    Q_OBJECT

public:
    explicit Subtitle(QWidget *parent = nullptr);
    ~Subtitle();

    void setTextList(const QStringList &textList);
    void start();
    void stop();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void showNextText();
    void loadSubtitles();
    void onFileChanged(const QString &path);

private:
    QLabel *m_label;
    QTimer *m_displayTimer;
    QStringList m_textList;
    int m_currentIndex;
    QFileSystemWatcher *m_fileWatcher;
    QString m_subtitlePath;

    void setupUI();
};

#endif // SUBTITLE_H
