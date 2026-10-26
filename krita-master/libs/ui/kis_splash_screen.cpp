/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 Alvin Wong <alvin@alvinhc.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_splash_screen.h"

#include <QApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QPainter>
#include <QCheckBox>
#include <kis_debug.h>
#include <QFile>
#include <QFontMetrics>
#include <QScreen>
#include <QWindow>
#include <QVBoxLayout>
#include <QLabel>

#include <KisPart.h>
#include <KisApplication.h>

#include <kis_icon.h>

#include <klocalizedstring.h>
#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

static void addDropShadow(QWidget *widget)
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(widget);
    effect->setBlurRadius(4);
    effect->setOffset(0.5);
    effect->setColor(QColor(0, 0, 0, 255));
    widget->setGraphicsEffect(effect);
}

KisSplashScreen::KisSplashScreen(int height, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
      , m_versionHtml(qApp->applicationVersion().toHtmlEscaped())
{
#ifdef Q_OS_HARMONY
    setWindowTitle(QStringLiteral("StellarCanvas"));
    if (!parent) {
        KConfigGroup harmonyGroup(KSharedConfig::openConfig(), QStringLiteral("HarmonyOS"));
        m_showFirstLaunchHint = !harmonyGroup.readEntry("FirstLaunchNoticeShown", false);
        if (m_showFirstLaunchHint) {
            harmonyGroup.writeEntry("FirstLaunchNoticeShown", true);
            harmonyGroup.sync();
        }
    }
#else
    setWindowTitle(i18n("Krita"));
#endif
#ifndef Q_OS_MACOS
    setWindowIcon(KisIconUtils::loadIcon("krita-branding"));
#endif

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(QMargins());
    m_lblSplash = new QLabel();
    m_lblSplash->setFrameShape(QFrame::NoFrame);
    layout()->addWidget(m_lblSplash);

    m_loadingTextLabel = new QLabel(m_lblSplash);
    m_loadingTextLabel->setTextFormat(Qt::RichText);
    m_loadingTextLabel->setStyleSheet(QStringLiteral("QLabel { color: #fff; background-color: transparent; }"));
    m_loadingTextLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    addDropShadow(m_loadingTextLabel);

    m_brandingIconLabel = new QLabel(m_lblSplash);
    m_brandingIconLabel->setFrameShape(QFrame::NoFrame);
    m_brandingIconLabel->setAlignment(Qt::AlignCenter);
    addDropShadow(m_brandingIconLabel);

    m_brandingNameLabel = new QLabel(QStringLiteral("StellarCanvas"), m_lblSplash);
    m_brandingNameLabel->setFrameShape(QFrame::NoFrame);
    m_brandingNameLabel->setAlignment(Qt::AlignCenter);
    m_brandingNameLabel->setStyleSheet(QStringLiteral("QLabel { color: #fff; background-color: transparent; font-weight: 700; }"));
    addDropShadow(m_brandingNameLabel);

    m_artCreditsLabel = new QLabel(m_lblSplash);
    m_artCreditsLabel->setTextFormat(Qt::PlainText);
    m_artCreditsLabel->setStyleSheet(QStringLiteral("QLabel { color: #fff; background-color: transparent; }"));
    m_artCreditsLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    addDropShadow(m_artCreditsLabel);

    updateSplashImage(height);
    setLoadingText(QString()); // the version string is set here too

    connect(&m_timer, SIGNAL(timeout()), SLOT(raise()));

    m_timer.setSingleShot(true);
    m_timer.start(10);
}

void KisSplashScreen::updateSplashImage(const int height)
{
    Source source = getImageSource();
    QPixmap img(source.resourcePath);

    if (img.isNull() || img.height() == 0) return;

    int artworkHeight = height;
    int artworkWidth = height * img.width() / img.height();

#ifdef Q_OS_HARMONY
    // The first top-level Qt window owns HarmonyOS' auto-created full-screen
    // ability window.  A fixed-size splash therefore cannot be positioned as
    // an independent desktop window: HarmonyOS expands the native host while
    // Qt keeps rendering only the splash-sized backing store, which leaves the
    // artwork pinned to the left with an unused strip on the right.  Match the
    // QWidget to the available application area and center the fixed-aspect
    // artwork inside it.  QScreen exposes logical pixels here, so Qt performs
    // the same DPR conversion used by the platform window and input paths.
    if (!parentWidget()) {
        if (QScreen *screen = QApplication::primaryScreen()) {
            const QSize availableSize = screen->availableGeometry().size();
            QSize artworkSize = img.size();
            artworkSize.scale(availableSize, Qt::KeepAspectRatio);
            artworkWidth = artworkSize.width();
            artworkHeight = artworkSize.height();
            setFixedSize(availableSize);
        } else {
            setFixedSize(artworkWidth, artworkHeight);
        }
        layout()->setAlignment(m_lblSplash, Qt::AlignCenter);
    } else {
        setFixedSize(artworkWidth, artworkHeight);
    }
    setStyleSheet(QStringLiteral("KisSplashScreen { background-color: #303030; }"));
#else
    setFixedSize(artworkWidth, artworkHeight);
#endif

    m_lblSplash->setFixedSize(artworkWidth, artworkHeight);

    const int marginTop = artworkHeight * 0.05;
    const int marginRight = artworkHeight * 0.1;

    // Let scaledContents downscale to the fixedSize
    m_lblSplash->setScaledContents(true);
    m_lblSplash->setPixmap(img);

    // StellarCanvas has a longer product name than the inherited branding.
    // Keep the icon and name on separate rows so neither has to be squeezed.
    const int brandIconExtent = qMax(48, qRound(artworkHeight * 0.13));
    const int brandSpacing = qMax(4, qRound(artworkHeight * 0.008));
    QFont brandNameFont = m_brandingNameLabel->font();
    brandNameFont.setPixelSize(qMax(18, qRound(artworkHeight * 0.045)));
    brandNameFont.setBold(true);
    m_brandingNameLabel->setFont(brandNameFont);
    const QFontMetrics brandNameMetrics(brandNameFont);
    const int brandNameHeight = brandNameMetrics.height() + brandSpacing * 2;
    const int brandWidth = qMax(brandIconExtent,
                                brandNameMetrics.horizontalAdvance(QStringLiteral("StellarCanvas")) + brandSpacing * 4);
    const int brandX = artworkWidth - marginRight - brandWidth;

    const QPixmap brandIcon(QStringLiteral(":/stellarcanvas-app-icon.png"));
    m_brandingIconLabel->setFixedSize(brandIconExtent, brandIconExtent);
    m_brandingIconLabel->setPixmap(brandIcon.scaled(brandIconExtent,
                                                    brandIconExtent,
                                                    Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation));
    m_brandingIconLabel->move(brandX + (brandWidth - brandIconExtent) / 2, marginTop);

    m_brandingNameLabel->setFixedSize(brandWidth, brandNameHeight);
    m_brandingNameLabel->move(brandX, m_brandingIconLabel->geometry().bottom() + brandSpacing);

    // Place version and loading text below the two-row branding block.
    m_loadingTextLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_loadingTextLabel->setFixedSize(brandWidth, qMax(48, qRound(artworkHeight * 0.12)));
    m_loadingTextLabel->move(brandX, m_brandingNameLabel->geometry().bottom() + brandSpacing);

    // Place credits text on bottom right with similar margins.
    // Size the credit from the actual localized text. The old fixed 20-pixel
    // height clipped the text completely on high-density HarmonyOS displays.
    // Keep the label transparent: the artwork credit is plain white text,
    // with only the existing subtle shadow to preserve readability.
    m_artCreditsLabel->setText(source.artistCredit);
    QFont creditsFont = m_artCreditsLabel->font();
    creditsFont.setPixelSize(qMax(14, qRound(artworkHeight * 0.022)));
    m_artCreditsLabel->setFont(creditsFont);
    const QFontMetrics creditsMetrics(creditsFont);
    const int creditWidth = qMin(creditsMetrics.horizontalAdvance(source.artistCredit) + 8,
                                 artworkWidth - marginRight * 2);
    const int creditHeight = creditsMetrics.height() + 6;
    const int creditBottomMargin = qMax(marginTop * 2, qRound(artworkHeight * 0.10));
    m_artCreditsLabel->setFixedSize(creditWidth, creditHeight);
    m_artCreditsLabel->move(artworkWidth - marginRight - creditWidth,
                            artworkHeight - creditBottomMargin - creditHeight);

    // Keep all overlays above the newly assigned splash pixmap.
    m_brandingIconLabel->raise();
    m_brandingNameLabel->raise();
    m_loadingTextLabel->raise();
    m_artCreditsLabel->raise();
}

void KisSplashScreen::setLoadingText(QString text)
{
#ifdef Q_OS_HARMONY
    if (m_showFirstLaunchHint) {
        const QString firstLaunchHint = i18n("The first launch loads many resources. Please wait patiently.");
        text = text.isEmpty() ? firstLaunchHint : text + QStringLiteral("\n") + firstLaunchHint;
    }
#endif
    int larger = 12;
    int notAsLarge = larger - 1;
    QString htmlText = QStringLiteral("<span style='font: %3pt;'><span style='font: bold %4pt;'>%1</span><br><i>%2</i></span>")
            .arg(m_versionHtml, text.toHtmlEscaped(), QString::number(notAsLarge), QString::number(larger));
    m_loadingTextLabel->setText(htmlText);
}

KisSplashScreen::Source KisSplashScreen::getImageSource()
{
    QString artistCredit = i18nc("Normal splash artist name", "xzz");
    // Loading the ginormous 4K PNG splash image increases the startup time on
    // Android by several seconds and at the same time looks really bad when
    // scaled down to a dinky size. Instead of overengineering this into an
    // Enterprise Splash Screen Solution where we choose the image based on
    // screen size or something, we'll just use a HD JPEG instead. It's fine.
#ifdef Q_OS_ANDROID
    QString resourcePath = QStringLiteral(":/splash/hd.jpg");
#else
    QString resourcePath = QStringLiteral(":/splash/0.png");
    // TODO: Re-add the holiday splash...
#if 0
    QDate currentDate = QDate::currentDate();
    if (currentDate > QDate(currentDate.year(), 12, 4) ||
            currentDate < QDate(currentDate.year(), 1, 9)) {
        resourcePath = QStringLiteral(":/splash/1.png");
        artistCredit = QStringLiteral("???")};
    }
#endif
#endif
    if (!artistCredit.isEmpty()) {
        artistCredit = i18nc("splash image credit", "Artwork by: %1", artistCredit);
    }
    return Source{resourcePath, artistCredit};
}

void KisSplashScreen::repaint()
{
    QWidget::repaint();
    qApp->sendPostedEvents();
}

void KisSplashScreen::show()
{
    if (!this->parentWidget()) {
        this->winId(); // Force creation of native window
        QWindow *windowHandle = this->windowHandle();
        QScreen *screen = windowHandle ? windowHandle->screen() : nullptr;
        if (windowHandle && !screen) {
            // At least on Windows, the window may be created on a non-primary
            // screen with a different scale factor. If we don't explicitly
            // move it to the primary screen, the position will be scaled with
            // the wrong factor and the splash will be offset.
            // XXX: In theory this branch should be unreachable, but leaving
            //      this here just in case.
            windowHandle->setScreen(QApplication::primaryScreen());
        }
        if (!screen) {
            screen = QApplication::primaryScreen();
        }
        QRect r(QPoint(), size());
        move(screen->availableGeometry().center() - r.center());
    }
    if (isVisible()) {
        repaint();
    }
    m_timer.setSingleShot(true);
    m_timer.start(1);
    QWidget::show();
}
