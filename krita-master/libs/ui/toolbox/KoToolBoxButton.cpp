/*
 * SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoToolBoxButton_p.h"

#include <kis_debug.h>

#include <KoToolManager.h>
#include <QAction>
#include <QPalette>
#include <QApplication>
#include <QTabletEvent>
#include <QToolTip>
#include <QTouchEvent>
#include <klocalizedstring.h>
#include <QKeySequence>
#include <KoIcon.h>

KoToolBoxButton::KoToolBoxButton(KoToolAction *toolAction, QWidget *parent)
    : QToolButton(parent)
    , m_toolAction(toolAction)
{
    setObjectName(m_toolAction->id());
    // ensure same L&F
    setCheckable(true);
    setAutoRaise(true);
    setIcon(kisIcon(m_toolAction->iconName()));

    // only a fallback
    setToolTip(m_toolAction->toolTip());

    // The toolbox keeps its existing mouse hover behavior. Touch and stylus
    // presses additionally get a long-press route to that same tooltip.
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    m_longPressTimer.setSingleShot(true);
    m_longPressTimer.setInterval(650);
    connect(&m_longPressTimer, &QTimer::timeout, this, [this]() {
        if (m_longPressActive && !toolTip().isEmpty()) {
            QToolTip::showText(mapToGlobal(m_longPressPosition), toolTip(), this, rect(), 10000);
        }
    });

    connect(this, SIGNAL(clicked(bool)), m_toolAction, SLOT(trigger()));
}

bool KoToolBoxButton::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin: {
        const auto *touchEvent = static_cast<QTouchEvent *>(event);
        if (touchEvent->points().size() == 1)
            beginLongPress(touchEvent->points().constFirst().position().toPoint());
        else
            cancelLongPress();
        break;
    }
    case QEvent::TouchUpdate: {
        const auto *touchEvent = static_cast<QTouchEvent *>(event);
        if (touchEvent->points().size() == 1)
            updateLongPress(touchEvent->points().constFirst().position().toPoint());
        else
            cancelLongPress();
        break;
    }
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        cancelLongPress();
        break;
    case QEvent::TabletPress:
        beginLongPress(static_cast<QTabletEvent *>(event)->position().toPoint());
        break;
    case QEvent::TabletMove: {
        const auto *tabletEvent = static_cast<QTabletEvent *>(event);
        if (tabletEvent->buttons().testFlag(Qt::LeftButton))
            updateLongPress(tabletEvent->position().toPoint());
        else
            cancelLongPress();
        break;
    }
    case QEvent::TabletRelease:
        cancelLongPress();
        break;
    default:
        break;
    }

    return QToolButton::event(event);
}

void KoToolBoxButton::beginLongPress(const QPoint &position)
{
    m_longPressPosition = position;
    m_longPressActive = true;
    m_longPressTimer.start();
}

void KoToolBoxButton::updateLongPress(const QPoint &position)
{
    if (m_longPressActive
        && (position - m_longPressPosition).manhattanLength() > QApplication::startDragDistance()) {
        cancelLongPress();
    }
}

void KoToolBoxButton::cancelLongPress()
{
    m_longPressActive = false;
    m_longPressTimer.stop();
}

void KoToolBoxButton::attachAction(QAction *action)
{
    if (!action) {
        return;
    }

    connect(action, SIGNAL(changed()), SLOT(slotUpdateActionData()));
    setDataFromToolAction(action);
}

void KoToolBoxButton::setHighlightColor()
{
    QPalette p = qApp->palette();
    if (isChecked()) {
        QPalette palette_highlight(p);
        QColor c = p.color(QPalette::Highlight);
        palette_highlight.setColor(QPalette::Button, c);
        setPalette(palette_highlight);
    }
    else {
        setPalette(p);
    }
}

void KoToolBoxButton::setDataFromToolAction(QAction *action)
{
    if (!action) {
        warnUI << "No tool action available, using fallback!";
        setToolTip(m_toolAction->toolTip());
        return;
    }
    setToolTip(action->toolTip());
}

void KoToolBoxButton::slotUpdateActionData()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        setDataFromToolAction(action);
    }
}
