#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class CanvasProbe final : public QWidget
{
public:
    explicit CanvasProbe(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(360);
        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this] {
            m_phase = (m_phase + 1) % 360;
            update();
        });
        timer->start(16);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QLinearGradient background(rect().topLeft(), rect().bottomRight());
        background.setColorAt(0.0, QColor(31, 39, 54));
        background.setColorAt(1.0, QColor(10, 15, 24));
        painter.fillRect(rect(), background);

        painter.setPen(QPen(QColor(255, 255, 255, 22), 1));
        constexpr int grid = 42;
        for (int x = 0; x < width(); x += grid)
            painter.drawLine(x, 0, x, height());
        for (int y = 0; y < height(); y += grid)
            painter.drawLine(0, y, width(), y);

        const qreal radius = qMin(width(), height()) * 0.24;
        const QPointF center(width() * 0.5, height() * 0.5);
        painter.translate(center);
        painter.rotate(m_phase);

        QConicalGradient colorWheel(QPointF(0, 0), 0);
        colorWheel.setColorAt(0.00, QColor("#ff5577"));
        colorWheel.setColorAt(0.20, QColor("#ffc857"));
        colorWheel.setColorAt(0.40, QColor("#5ee47b"));
        colorWheel.setColorAt(0.60, QColor("#51c9ff"));
        colorWheel.setColorAt(0.80, QColor("#9b7cff"));
        colorWheel.setColorAt(1.00, QColor("#ff5577"));
        painter.setBrush(colorWheel);
        painter.setPen(QPen(QColor(255, 255, 255, 190), 3));
        painter.drawEllipse(QPointF(0, 0), radius, radius);

        painter.setBrush(QColor(15, 20, 30));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(0, 0), radius * 0.57, radius * 0.57);
    }

private:
    int m_phase = 0;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("StellarCanvas Qt Smoke"));

    QWidget window;
    window.setWindowTitle(QStringLiteral("StellarCanvas · Qt 6.12 · HarmonyOS"));
    window.setStyleSheet(QStringLiteral(
        "QWidget { background: #0b1019; color: #eef3ff; }"
        "QLabel#title { font-size: 28px; font-weight: 700; }"
        "QLabel#status { color: #66e38f; font-size: 17px; }"
        "QLabel#details { color: #aeb9cc; font-size: 15px; }"));

    auto *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(34, 28, 34, 28);
    layout->setSpacing(14);

    auto *title = new QLabel(QStringLiteral("StellarCanvas / Krita 鸿蒙移植"));
    title->setObjectName(QStringLiteral("title"));
    layout->addWidget(title);

    auto *status = new QLabel(QStringLiteral("● Qt Widgets + OpenGL ES 运行链路已启动"));
    status->setObjectName(QStringLiteral("status"));
    layout->addWidget(status);

    auto *details = new QLabel(QStringLiteral(
        "Qt 6.12.0 · arm64-v8a · HarmonyOS API 24\n"
        "动画持续刷新可验证事件循环、字体、栅格绘制与窗口合成。"));
    details->setObjectName(QStringLiteral("details"));
    details->setWordWrap(true);
    layout->addWidget(details);

    layout->addWidget(new CanvasProbe, 1);

    window.resize(1100, 720);
    window.show();
    return app.exec();
}
