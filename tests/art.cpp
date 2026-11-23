#include <QImage>
#include <QtTest>

class ArtTest : public QObject
{
    Q_OBJECT
private slots:
    void bundledFrames();
};

void ArtTest::bundledFrames()
{
    QImage a(":/sprites/explorer_a.png");
    QImage b(":/sprites/explorer_b.png");
    QVERIFY(!a.isNull() && !b.isNull());
    QVERIFY(a != b);
    for (const QImage &sheet : {a, b}) {
        QCOMPARE(sheet.size(), QSize(96, 112));
        QVERIFY(sheet.hasAlphaChannel());
        for (int row = 0; row < 4; row++) {
            QList<QImage> poses;
            for (int col = 0; col < 4; col++) {
                QImage cell = sheet.copy(col * 24, row * 28, 24, 28);
                QCOMPARE(cell.pixelColor(0, 0).alpha(), 0);
                QVERIFY(cell.pixelColor(12, 7).alpha() > 0);
                QVERIFY(!poses.contains(cell));
                poses.append(cell);
            }
        }
    }
}

QTEST_APPLESS_MAIN(ArtTest)
#include "art.moc"
