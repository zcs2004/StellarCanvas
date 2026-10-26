/*
  SPDX-FileCopyrightText: 2006 Gábor Lehel <illissius@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "KoItemToolTip.h"

#include <QApplication>
#include <QTimer>
#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QPersistentModelIndex>
#include <QStyleOptionViewItem>
#include <QTextDocument>
#include <QToolTip>
#include <QScreen>
#include <QWindow>

// for HAVE_WAYLAND
#include <KoConfig.h>

#if defined HAVE_WAYLAND && QT_VERSION >= QT_VERSION_CHECK(6, 11, 0)
#define HAVE_QT_PRIVATE_TOOLTIP_POSITION_API
#endif

#ifdef HAVE_QT_PRIVATE_TOOLTIP_POSITION_API
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#endif

#include <kis_assert.h>


class Q_DECL_HIDDEN KoItemToolTip::Private
{
    public:
        std::unique_ptr<QTextDocument> document;
        QPersistentModelIndex index;
        QPoint pos;
        QTimer timer;
};

KoItemToolTip::KoItemToolTip(QWidget *parent)
    : QFrame(parent, Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint)
    , d(new Private)
{
    Q_ASSERT(parent);

    d->timer.setSingleShot(true);
    d->timer.setInterval(10000);
    connect(&d->timer, &QTimer::timeout, this, &KoItemToolTip::hide);

    d->document.reset(new QTextDocument(this));

    parent->installEventFilter(this);
}

KoItemToolTip::~KoItemToolTip()
{
}

void KoItemToolTip::showTip(const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    std::unique_ptr<QTextDocument> doc(createDocument(index));

    QPoint p = (isVisible() && index == d->index) ? d->pos : pos;

    if (!isVisible() || index != d->index || doc->toHtml() != d->document->toHtml()) {
#ifdef Q_OS_HARMONY
        // A visible tooltip is backed by an embedded ArkUI node. Updating the
        // QWidget geometry while that node is exposed does not run the native
        // show-time geometry synchronization, so changing to another item can
        // leave the reused node at the host origin. Re-open the native tooltip
        // when its contents change; setVisible(true) then consumes the final
        // geometry calculated by updatePosition().
        if (isVisible()) {
            hide();
        }
#endif
        d->pos = p;
        d->index = index;
        d->document.swap(doc);
        updatePosition(p, option);
        if (!isVisible())
            show();
        else
            update();
    }

    d->timer.start();
}

void KoItemToolTip::updatePosition(const QPoint &pos, const QStyleOptionViewItem &option)
{
    QWidget *widget = parentWidget();
    KIS_SAFE_ASSERT_RECOVER_RETURN(widget);

    const QPoint gpos = widget->mapToGlobal(pos);
    const QSize size = sizeHint();

    // resize **must** come before `setParentControlGeometry()`, otherwise
    // compositor will place the window incorrectly!
    resize(size);

    // make sure that all the platform-specific structures for the window
    // are created (i.e. QWaylandWindow) before trying to request them
    // via `windowHandle()->handle()`
    create();

#ifdef HAVE_QT_PRIVATE_TOOLTIP_POSITION_API

    if (auto waylandWindow = dynamic_cast<QNativeInterface::Private::QWaylandWindow *>(windowHandle()->handle())) {

        const QPoint parentPos = widget->window()->mapFromGlobal(gpos);
        const QRect gravityRect(parentPos.x() - 20, parentPos.y() - 20, 40, 40);
        waylandWindow->setParentControlGeometry(gravityRect);
        waylandWindow->setExtendedWindowType(QNativeInterface::Private::QWaylandWindow::ToolTip);
    } else
#endif /* HAVE_QT_PRIVATE_TOOLTIP_POSITION_API */

    {
        QScreen *screen = widget->screen();
        const QRect availableRect = screen->availableGeometry();
        const int width = size.width(), height = size.height();
        const QRect itemRect(widget->mapToGlobal(option.rect.topLeft()), option.rect.size());

        int y = gpos.y() + 20;
        if (y + height > availableRect.bottom()) {
            y = itemRect.bottom();

            if (y + height > availableRect.bottom()) {
                y = gpos.y() - 20 - height;

                if (y < availableRect.top()) {
                    y = itemRect.top() - height;

                    if (y < availableRect.top()) {
                        y = availableRect.top();
                    }
                }
            }
        }

        int x = gpos.x() + 20;
        if (x + width > availableRect.right()) {
            x = itemRect.right();

            if (x + width > availableRect.right()) {
                x = gpos.x() - 20 - width;

                if (x < availableRect.left()) {
                    x = itemRect.left() - width;

                    if (x < availableRect.left()) {
                        x = availableRect.left();
                    }
                }
            }
        }

        const QRect targetGeometry(QPoint(x, y), size);
#ifdef Q_OS_HARMONY
        // HarmonyOS creates Qt tooltips as embedded ArkUI nodes. Their native
        // handle is created above, before the final desktop-style placement is
        // known, so retain the final screen rectangle for the platform plugin
        // to consume immediately before exposing the node.
        if (windowHandle()) {
            windowHandle()->setProperty(
                "_q_ohos_pendingPopupGeometry", targetGeometry);
        }
#endif
        setGeometry(targetGeometry);
    }
}

QSize KoItemToolTip::sizeHint() const
{
    return d->document->size().toSize();
}

void KoItemToolTip::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    d->document->drawContents(&p, rect());
    p.drawRect(0, 0, width() - 1, height() - 1);
}

bool KoItemToolTip::eventFilter(QObject *object, QEvent *event)
{
    switch(event->type())
    {
        case QEvent::Leave:
            hide();
        default: break;
    }

    return QFrame::eventFilter(object, event);
}
