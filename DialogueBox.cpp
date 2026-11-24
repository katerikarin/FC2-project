#include "DialogueBox.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>

namespace {
constexpr int TypewriterIntervalMs = 24;
}

DialogueBox::DialogueBox(QWidget* parent)
    : QWidget(parent)
{
    hide();
    connect(&m_typeTimer, &QTimer::timeout, this, &DialogueBox::onTypewriterTick);
}

void DialogueBox::showMessages(const QStringList& pages, const QString& speaker)
{
    if (pages.isEmpty())
        return;

    m_pages = pages;
    m_speaker = speaker;
    m_pageIndex = 0;
    m_visibleChars = 0;
    m_blinkTicks = 0;

    show();
    raise();
    update();
    m_typeTimer.start(TypewriterIntervalMs);
}

void DialogueBox::clear()
{
    m_typeTimer.stop();
    m_pages.clear();
    m_speaker.clear();
    m_pageIndex = 0;
    m_visibleChars = 0;
    hide();
}

void DialogueBox::advance()
{
    if (!isActive())
        return;

    const QString& currentText = m_pages[m_pageIndex];
    if (m_visibleChars < currentText.length()) {
        // Finish typing the current page immediately
        m_visibleChars = currentText.length();
        update();
        return;
    }

    // Move to the next page or close the dialogue box
    ++m_pageIndex;
    if (m_pageIndex < m_pages.size()) {
        m_visibleChars = 0;
        m_blinkTicks = 0;
        update();
    } else {
        clear();
        emit dialogueFinished();
    }
}

void DialogueBox::onTypewriterTick()
{
    if (!isActive()) {
        m_typeTimer.stop();
        return;
    }

    const QString& currentText = m_pages[m_pageIndex];
    if (m_visibleChars < currentText.length()) {
        ++m_visibleChars;
    } else {
        ++m_blinkTicks;
    }
    update();
}

void DialogueBox::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isActive()) {
        advance();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void DialogueBox::paintEvent(QPaintEvent*)
{
    if (!isActive())
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // Classic FireRed-inspired outer dark frame + red/gold inner trim + cream box
    QRect outer = rect().adjusted(2, 2, -3, -3);
    p.fillRect(outer, QColor("#283038"));

    QRect trim = outer.adjusted(4, 4, -4, -4);
    p.fillRect(trim, QColor("#c84848"));

    QRect inner = trim.adjusted(4, 4, -4, -4);
    p.fillRect(inner, QColor("#f8f8f0"));

    // Subtle inner bevel line
    p.setPen(QPen(QColor("#d0c8b8"), 2));
    p.drawRect(inner.adjusted(1, 1, -1, -1));

    // Optional speaker label tab at top-left
    int textTop = inner.top() + 14;
    if (!m_speaker.isEmpty()) {
        QFont badgeFont(QStringLiteral("Consolas"), 11, QFont::Bold);
        p.setFont(badgeFont);
        const int badgeW = p.fontMetrics().horizontalAdvance(m_speaker) + 20;
        QRect badgeRect(inner.left() + 14, outer.top() - 2, badgeW, 22);
        p.fillRect(badgeRect, QColor("#283038"));
        p.fillRect(badgeRect.adjusted(2, 2, -2, -2), QColor("#3868a8"));
        p.setPen(QColor("#ffffff"));
        p.drawText(badgeRect, Qt::AlignCenter, m_speaker);
        textTop = inner.top() + 22;
    }

    // Dialogue text (revealed progressively via m_visibleChars)
    QFont textFont(QStringLiteral("Consolas"), 14, QFont::Bold);
    p.setFont(textFont);
    p.setPen(QColor("#282828"));

    const QString shown = m_pages[m_pageIndex].left(m_visibleChars);
    QRect textRect(inner.left() + 20, textTop, inner.width() - 48, inner.height() - (textTop - inner.top()) - 12);
    p.drawText(textRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, shown);

    // Blinking red downward triangle when the current page finishes typing
    if (m_visibleChars >= m_pages[m_pageIndex].length() && ((m_blinkTicks / 14) % 2 == 0)) {
        const int cx = inner.right() - 20;
        const int cy = inner.bottom() - 16;
        QPolygonF arrow;
        arrow << QPointF(cx - 7, cy - 5) << QPointF(cx + 7, cy - 5) << QPointF(cx, cy + 5);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#d83838"));
        p.drawPolygon(arrow);
    }
}