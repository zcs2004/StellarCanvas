/*
 * This file is part of the KDE Libraries
 * SPDX-FileCopyrightText: 1999-2000 Espen Sand (espen@kde.org)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 */

// I (espen) prefer that header files are included alphabetically

#include "khelpmenu.h"
#include "config-xmlgui.h"
#include <QTimer>
#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QMenu>
#include <QStyle>
#include <QWidget>
#include <QWhatsThis>
#include <QFile>
#include <QFontDatabase>
#include <QDir>
#include <QUrl>
#include <QBoxLayout>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDebug>
#include <QDialog>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QTabWidget>
#include <QTextBrowser>

#include "kaboutkdedialog_p.h"
#include "kbugreport.h"
#include "kswitchlanguagedialog_p.h"

#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <kstandardaction.h>

#include <kritaversion.h>

using namespace KDEPrivate;

class KisKHelpMenuPrivate
{
public:
    KisKHelpMenuPrivate()
        : mAboutData(KAboutData::applicationData())
    {
    }
    ~KisKHelpMenuPrivate()
    {
        delete mMenu;
        delete mAboutApp;
        delete mAboutKDE;
        delete mBugReport;
        delete mSwitchApplicationLanguage;
        delete mDonationDialog;
    }

    void createActions(KisKHelpMenu *q);

    QMenu *mMenu {nullptr};
    QDialog *mAboutApp {nullptr};
    KisKAboutKdeDialog *mAboutKDE {nullptr};
    KisKBugReport *mBugReport {nullptr};
    KisKSwitchLanguageDialog *mSwitchApplicationLanguage {nullptr};
    QDialog *mDonationDialog {nullptr};
    // TODO evaluate if we use static_cast<QWidget*>(parent()) instead of mParent to win that bit of memory
    QWidget *mParent {nullptr};
    QString mAboutAppText;

    bool mShowWhatsThis {false};
    bool mActionsCreated {false};

    QAction *mHandBookAction {nullptr};
    QAction *mWhatsThisAction {nullptr};
    QAction *mReportBugAction {nullptr};
    QAction *mFeedbackAction {nullptr};
    QAction *mSwitchApplicationLanguageAction {nullptr};
    QAction *mAboutAppAction {nullptr};
    QAction *mAboutKDEAction {nullptr};

    KAboutData mAboutData;
};

#ifdef Q_OS_HARMONY
namespace
{
void resizeStellarCanvasDialog(QDialog *dialog, const QSize &preferredSize)
{
    const QScreen *screen = dialog->parentWidget() && dialog->parentWidget()->screen()
        ? dialog->parentWidget()->screen()
        : QGuiApplication::primaryScreen();
    const QSize availableSize = screen ? screen->availableGeometry().size() : preferredSize;
    dialog->resize(qMin(preferredSize.width(), qMax(360, availableSize.width() - 48)),
                   qMin(preferredSize.height(), qMax(320, availableSize.height() - 48)));
}

QWidget *createQrCodeCard(const QString &title, const QString &resourcePath, QWidget *parent)
{
    QWidget *card = new QWidget(parent);
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 12, 12, 12);

    QLabel *titleLabel = new QLabel(title, card);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel *imageLabel = new QLabel(card);
    const QPixmap pixmap(resourcePath);
    imageLabel->setPixmap(pixmap.scaled(QSize(300, 390), Qt::KeepAspectRatio, Qt::FastTransformation));
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setMinimumSize(260, 300);
    layout->addWidget(imageLabel, 1);
    return card;
}

QString readStellarCanvasTextResource(const QString &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QStringLiteral("Unable to load license text.");
    }
    return QString::fromUtf8(file.readAll());
}

QTextBrowser *createStellarCanvasLicensePage(const QString &resourcePath, QWidget *parent)
{
    QTextBrowser *page = new QTextBrowser(parent);
    page->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    page->setLineWrapMode(QTextEdit::NoWrap);
    page->setPlainText(readStellarCanvasTextResource(resourcePath));
    return page;
}

QDialog *createStellarCanvasAboutDialog(QWidget *parent)
{
    const QString applicationName = QStringLiteral("StellarCanvas");
    const QString repositoryUrl = QStringLiteral("https://github.com/zcs2004/StellarCanvas");

    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(i18n("About") + QLatin1Char(' ') + applicationName);
    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QTabWidget *tabs = new QTabWidget(dialog);
    QWidget *stellarCanvasPage = new QWidget(tabs);
    QVBoxLayout *stellarCanvasLayout = new QVBoxLayout(stellarCanvasPage);

    QLabel *appIcon = new QLabel(stellarCanvasPage);
    const QPixmap iconPixmap(QStringLiteral(":/stellarcanvas-app-icon.png"));
    appIcon->setPixmap(iconPixmap.scaled(QSize(144, 144), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    appIcon->setAlignment(Qt::AlignCenter);
    appIcon->setAttribute(Qt::WA_TranslucentBackground);
    appIcon->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    stellarCanvasLayout->addWidget(appIcon);

    QLabel *heading = new QLabel(applicationName, stellarCanvasPage);
    QFont headingFont = heading->font();
    headingFont.setPointSize(qMax(headingFont.pointSize() + 8, 20));
    headingFont.setBold(true);
    heading->setFont(headingFont);
    heading->setAlignment(Qt::AlignCenter);
    stellarCanvasLayout->addWidget(heading);

    QTextBrowser *details = new QTextBrowser(stellarCanvasPage);
    details->setOpenExternalLinks(true);
    QString portingNote = i18n("Porting note: Ported from {UPSTREAM_APPLICATION}.");
    portingNote.replace(QStringLiteral("{UPSTREAM_APPLICATION}"), QStringLiteral("Krita"));
    QString versionLabel = i18n("Version");
    versionLabel.append(QLatin1Char(':'));
    const QString detailsHtml = QStringLiteral(
                                    "<p><b>%1</b> %2</p>"
                                    "<p>%3<br>%4</p>"
                                    "<p>%5</p>"
                                    "<p><b>%6</b> <a href=\"%7\">%7</a></p>"
                                    "<p><b>%8</b> %9</p>")
                                    .arg(versionLabel.toHtmlEscaped(),
                                         QStringLiteral("v1.0.0"),
                                         i18n("Copyright: %1", QStringLiteral("Zou Changsheng")).toHtmlEscaped(),
                                         i18n("Copyright: %1", QStringLiteral("The Regents of the University of California.")).toHtmlEscaped(),
                                         portingNote.toHtmlEscaped(),
                                         QStringLiteral("GitHub:"),
                                         repositoryUrl,
                                         i18n("Contact:").toHtmlEscaped(),
                                         QStringLiteral("zcs2004@126.com"));
    details->setHtml(detailsHtml);
    stellarCanvasLayout->addWidget(details, 1);

    tabs->addTab(stellarCanvasPage, applicationName);
    tabs->addTab(createStellarCanvasLicensePage(QStringLiteral(":/stellarcanvas-gpl-3.0.txt"), tabs),
                 QStringLiteral("GPL-3.0"));
    tabs->addTab(createStellarCanvasLicensePage(QStringLiteral(":/stellarcanvas-license.txt"), tabs),
                 QStringLiteral("License"));
    layout->addWidget(tabs, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    buttons->button(QDialogButtonBox::Close)->setText(i18n("Close"));
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    layout->addWidget(buttons);
    resizeStellarCanvasDialog(dialog, QSize(820, 700));
    return dialog;
}

QDialog *createStellarCanvasDonationDialog(QWidget *parent)
{
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(i18n("Support with a Donation"));
    QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
    QTabWidget *tabs = new QTabWidget(dialog);

    QScrollArea *paymentScroll = new QScrollArea(tabs);
    paymentScroll->setWidgetResizable(true);
    QWidget *paymentPage = new QWidget(paymentScroll);
    QVBoxLayout *paymentLayout = new QVBoxLayout(paymentPage);
    QHBoxLayout *qrLayout = new QHBoxLayout;
    qrLayout->addWidget(createQrCodeCard(i18n("WeChat Pay"), QStringLiteral(":/stellarcanvas/wechat-pay.png"), paymentPage));
    qrLayout->addWidget(createQrCodeCard(i18n("Alipay"), QStringLiteral(":/stellarcanvas/alipay.png"), paymentPage));
    paymentLayout->addLayout(qrLayout);
    QLabel *paymentNotice = new QLabel(i18n("1. When donating, please include your online nickname (please do not use your real name; if your nickname changes, you may contact customer service to update it) and mobile phone number. This allows us to keep records and grant illustration benefits. We will display only your online nickname and will not disclose private information such as your phone number.\n\n2. If money is currently tight, please take care of yourself first. You are always welcome to support us later when circumstances allow."), paymentPage);
    paymentNotice->setWordWrap(true);
    paymentNotice->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    paymentLayout->addWidget(paymentNotice);
    paymentScroll->setWidget(paymentPage);
    tabs->addTab(paymentScroll, i18n("Donation Methods"));

    QTextBrowser *privileges = new QTextBrowser(tabs);
    privileges->setPlainText(i18n("🎨 Illustration Showcase\n\nThe app provides 9 startup-page illustration slots, updated monthly.\n\n📊 Ranking Rules\n\nAll-time ranking: displayed for 3 months; the top 3 historical donors are listed.\nMonthly ranking: settled monthly; the top 3 donors of the month are listed.\nEvent ranking: at least one event is held every 3 months; the top 3 by votes are listed.\n\n⚡ Slot Conflicts\n\nIf one person qualifies for multiple ranking slots, they may provide multiple illustrations. Slot priority is: Event ranking > All-time ranking > Monthly ranking. If not enough illustrations are provided, the slot passes to the next person.\n\n💝 Sponsor Rewards\n\nSponsors receive exclusive illustration showcases and ranking honors.\nFree users are unaffected and can continue using the app for free."));
    tabs->addTab(privileges, i18n("Donation Benefits"));

    QTextBrowser *illustrationList = new QTextBrowser(tabs);
    illustrationList->setPlainText(i18n("There is currently no list. It will be added with the next version."));
    tabs->addTab(illustrationList, i18n("Illustration List"));

    dialogLayout->addWidget(tabs, 1);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    buttons->button(QDialogButtonBox::Close)->setText(i18n("Close"));
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    dialogLayout->addWidget(buttons);
    resizeStellarCanvasDialog(dialog, QSize(900, 720));
    return dialog;
}
}
#endif

KisKHelpMenu::KisKHelpMenu(QWidget *parent, const QString &aboutAppText,
                     bool showWhatsThis)
    : QObject(parent), d(new KisKHelpMenuPrivate)
{
    d->mAboutAppText = aboutAppText;
    d->mShowWhatsThis = showWhatsThis;
    d->mParent = parent;
    d->createActions(this);
}

KisKHelpMenu::KisKHelpMenu(QWidget *parent, const KAboutData &aboutData,
                     bool showWhatsThis)
    : QObject(parent), d(new KisKHelpMenuPrivate)
{
    d->mShowWhatsThis = showWhatsThis;
    d->mParent = parent;
    d->mAboutData = aboutData;
    d->createActions(this);
}

KisKHelpMenu::~KisKHelpMenu()
{
    delete d;
}

void KisKHelpMenuPrivate::createActions(KisKHelpMenu *q)
{
    if (mActionsCreated) {
        return;
    }
    mActionsCreated = true;
    mHandBookAction = KStandardAction::helpContents(q, SLOT(appHelpActivated()), q);
    if (mShowWhatsThis) {
        mWhatsThisAction = KStandardAction::whatsThis(q, SLOT(contextHelpActivated()), q);
    }

    if (!mAboutData.bugAddress().isEmpty()) {
        mReportBugAction = KStandardAction::reportBug(q, SLOT(reportBug()), q);
    }

    mSwitchApplicationLanguageAction = KStandardAction::create(KStandardAction::SwitchApplicationLanguage, q, SLOT(switchApplicationLanguage()), q);
    mAboutAppAction = KStandardAction::aboutApp(q, SLOT(aboutApplication()), q);
    mAboutKDEAction = KStandardAction::aboutKDE(q, SLOT(aboutKDE()), q);
#ifdef Q_OS_HARMONY
    mHandBookAction->setText(i18n("StellarCanvas User Manual"));
    if (!mReportBugAction) {
        mReportBugAction = KStandardAction::reportBug(q, SLOT(reportBug()), q);
    }
    mReportBugAction->setText(i18n("StellarCanvas Website"));
    mFeedbackAction = new QAction(i18n("Suggestions and Feedback"), q);
    mFeedbackAction->setObjectName(QStringLiteral("help_feedback"));
    QObject::connect(mFeedbackAction, &QAction::triggered, q, &KisKHelpMenu::feedback);
    mAboutAppAction->setText(i18n("About") + QStringLiteral(" StellarCanvas"));
    mAboutKDEAction->setText(i18n("Support with a Donation"));
#endif
}

// Used in the non-xml-gui case, like kfind or ksnapshot's help button.
QMenu *KisKHelpMenu::menu()
{
    if (!d->mMenu) {
        d->mMenu = new QMenu();
        connect(d->mMenu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));

        d->mMenu->setTitle(i18n("&Help"));

        d->createActions(this);

        bool need_separator = false;
        if (d->mHandBookAction) {
            d->mMenu->addAction(d->mHandBookAction);
            need_separator = true;
        }

        if (d->mWhatsThisAction) {
            d->mMenu->addAction(d->mWhatsThisAction);
            need_separator = true;
        }

        if (d->mReportBugAction) {
            if (need_separator) {
                d->mMenu->addSeparator();
            }
            d->mMenu->addAction(d->mReportBugAction);
            need_separator = true;
        }

        if (d->mFeedbackAction) {
            d->mMenu->addAction(d->mFeedbackAction);
            need_separator = true;
        }

        if (d->mSwitchApplicationLanguageAction) {
            if (need_separator) {
                d->mMenu->addSeparator();
            }
            d->mMenu->addAction(d->mSwitchApplicationLanguageAction);
            need_separator = true;
        }

        if (need_separator) {
            d->mMenu->addSeparator();
        }

        if (d->mAboutAppAction) {
            d->mMenu->addAction(d->mAboutAppAction);
        }

        if (d->mAboutKDEAction) {
            d->mMenu->addAction(d->mAboutKDEAction);
        }
    }

    return d->mMenu;
}

QAction *KisKHelpMenu::action(MenuId id) const
{
    switch (id) {
    case menuHelpContents:
        return d->mHandBookAction;
        break;

    case menuWhatsThis:
        return d->mWhatsThisAction;
        break;

    case menuReportBug:
        return d->mReportBugAction;
        break;

    case menuFeedback:
        return d->mFeedbackAction;
        break;

    case menuSwitchLanguage:
        return d->mSwitchApplicationLanguageAction;
        break;

    case menuAboutApp:
        return d->mAboutAppAction;
        break;

    case menuAboutKDE:
        return d->mAboutKDEAction;
        break;
    }

    return 0;
}

void KisKHelpMenu::appHelpActivated()
{
#ifdef Q_OS_HARMONY
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://HelpStellarCanvas.dhuzcs.com")));
#else
    QDesktopServices::openUrl(QUrl(QStringLiteral("help:/")));
#endif
}

void KisKHelpMenu::aboutApplication()
{
#ifdef Q_OS_HARMONY
    if (!d->mAboutApp) {
        d->mAboutApp = createStellarCanvasAboutDialog(d->mParent);
        connect(d->mAboutApp, SIGNAL(finished(int)), this, SLOT(dialogFinished()));
    }
    d->mAboutApp->show();
    d->mAboutApp->raise();
#else
    if (receivers(SIGNAL(showAboutApplication())) > 0) {
        Q_EMIT showAboutApplication();
    }
#endif
}

void KisKHelpMenu::aboutKDE()
{
#ifdef Q_OS_HARMONY
    if (!d->mDonationDialog) {
        d->mDonationDialog = createStellarCanvasDonationDialog(d->mParent);
        connect(d->mDonationDialog, SIGNAL(finished(int)), this, SLOT(dialogFinished()));
    }
    d->mDonationDialog->show();
    d->mDonationDialog->raise();
#else
    if (!d->mAboutKDE) {
        d->mAboutKDE = new KisKAboutKdeDialog(d->mParent);
        connect(d->mAboutKDE, SIGNAL(finished(int)), this, SLOT(dialogFinished()));
    }
    d->mAboutKDE->show();
#endif
}

void KisKHelpMenu::reportBug()
{
#ifdef Q_OS_HARMONY
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://StellarCanvas.dhuzcs.com")));
#else
#ifdef KRITA_STABLE
    QDesktopServices::openUrl(QUrl("https://docs.krita.org/en/untranslatable_pages/reporting_bugs.html"));
#else
    if (!d->mBugReport) {
        d->mBugReport = new KisKBugReport(d->mAboutData, d->mParent);
        connect(d->mBugReport, SIGNAL(finished(int)), this, SLOT(dialogFinished()));
    }
    d->mBugReport->show();
#endif
#endif
}

void KisKHelpMenu::feedback()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://sugback.dhuzcs.com")));
}

void KisKHelpMenu::switchApplicationLanguage()
{
    if (!d->mSwitchApplicationLanguage) {
        d->mSwitchApplicationLanguage = new KisKSwitchLanguageDialog(d->mParent);
        connect(d->mSwitchApplicationLanguage, SIGNAL(finished(int)), this, SLOT(dialogFinished()));
    }
    d->mSwitchApplicationLanguage->show();
}

void KisKHelpMenu::dialogFinished()
{
    QTimer::singleShot(0, this, SLOT(timerExpired()));
}

void KisKHelpMenu::timerExpired()
{
    if (d->mAboutKDE && !d->mAboutKDE->isVisible()) {
        delete d->mAboutKDE; d->mAboutKDE = 0;
    }

    if (d->mBugReport && !d->mBugReport->isVisible()) {
        delete d->mBugReport; d->mBugReport = 0;
    }
    if (d->mSwitchApplicationLanguage && !d->mSwitchApplicationLanguage->isVisible()) {
        delete d->mSwitchApplicationLanguage; d->mSwitchApplicationLanguage = 0;
    }
    if (d->mAboutApp && !d->mAboutApp->isVisible()) {
        delete d->mAboutApp; d->mAboutApp = 0;
    }
    if (d->mDonationDialog && !d->mDonationDialog->isVisible()) {
        delete d->mDonationDialog; d->mDonationDialog = 0;
    }
}

void KisKHelpMenu::menuDestroyed()
{
    d->mMenu = 0;
}

void KisKHelpMenu::contextHelpActivated()
{
    QWhatsThis::enterWhatsThisMode();
}
