#ifndef DIALOGUEBOX_H
#define DIALOGUEBOX_H

#include <QWidget>
#include <QTimer>
#include <QStringList>
#include <QMouseEvent>

class DialogueBox : public QWidget
{
    Q_OBJECT

public:
    explicit DialogueBox(QWidget* parent = nullptr);

    void showMessages(const QStringList& pages, const QString& speaker = QString());
    void advance();
    void clear();

    bool isActive() const { return isVisible() && !m_pages.isEmpty(); }

signals:
    void dialogueFinished();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onTypewriterTick();

private:
    QTimer m_typeTimer;
    QStringList m_pages;
    QString m_speaker;
    int m_pageIndex = 0;
    int m_visibleChars = 0;
    int m_blinkTicks = 0;
};

#endif // DIALOGUEBOX_H