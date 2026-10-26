/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceItemListView.h"

#include <QEvent>
#include <QApplication>
#include <QScroller>
#include <QScrollBar>
#include <QHelpEvent>
#include <QPointer>
#include <QTabletEvent>
#include <QTimer>
#include <QTouchEvent>

#include <functional>
#include <utility>

#include "KisIconToolTip.h"

namespace
{

class TouchStylusLongPressFilter final : public QObject
{
public:
    TouchStylusLongPressFilter(QWidget *target,
                              std::function<void(const QPoint &)> showToolTip,
                              std::function<void()> hideToolTip,
                              QObject *parent)
        : QObject(parent)
        , m_target(target)
        , m_showToolTip(std::move(showToolTip))
        , m_hideToolTip(std::move(hideToolTip))
    {
        m_timer.setSingleShot(true);
        m_timer.setInterval(650);
        connect(&m_timer, &QTimer::timeout, this, [this]() {
            if (m_active && m_showToolTip) {
                m_showToolTip(m_pressPosition);
                m_longPressShown = true;
            }
        });
        m_target->setAttribute(Qt::WA_AcceptTouchEvents, true);
        // QScroller may take ownership of a touch sequence after the initial
        // press, so the release is not guaranteed to return to the viewport.
        // Observe application delivery while keeping the press origin scoped
        // to this viewport. Events are never consumed here.
        qApp->installEventFilter(this);
    }

    ~TouchStylusLongPressFilter() override
    {
        if (qApp)
            qApp->removeEventFilter(this);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched != m_target && !m_active)
            return QObject::eventFilter(watched, event);

        switch (event->type()) {
        case QEvent::TouchBegin: {
            // An unconsumed TouchBegin is propagated from the viewport to its
            // parent scroll area. It is still the same physical press, not a
            // second touch and therefore must not cancel the pending hold.
            if (watched != m_target)
                break;
            const auto *touchEvent = static_cast<QTouchEvent *>(event);
            if (touchEvent->points().size() == 1) {
                const auto &point = touchEvent->points().constFirst();
                begin(point.position().toPoint(), point.globalPosition().toPoint());
            } else {
                cancel();
            }
            break;
        }
        case QEvent::TouchUpdate: {
            const auto *touchEvent = static_cast<QTouchEvent *>(event);
            if (touchEvent->points().size() != 1) {
                cancel();
            } else {
                update(touchEvent->points().constFirst().globalPosition().toPoint());
            }
            break;
        }
        case QEvent::TouchEnd:
        case QEvent::TouchCancel:
            cancel();
            break;
        case QEvent::TabletPress: {
            if (watched != m_target)
                break;
            const auto *tabletEvent = static_cast<QTabletEvent *>(event);
            begin(tabletEvent->position().toPoint(), tabletEvent->globalPosition().toPoint());
            break;
        }
        case QEvent::TabletMove: {
            const auto *tabletEvent = static_cast<QTabletEvent *>(event);
            if (tabletEvent->buttons().testFlag(Qt::LeftButton))
                update(tabletEvent->globalPosition().toPoint());
            else
                cancel();
            break;
        }
        case QEvent::TabletRelease:
            cancel();
            break;
        default:
            break;
        }

        // Observation only: existing tap selection and kinetic scrolling keep
        // receiving their original events. Mouse events never enter this path.
        return QObject::eventFilter(watched, event);
    }

private:
    void begin(const QPoint &position, const QPoint &globalPosition)
    {
        dismissShownToolTip();
        m_pressPosition = position;
        m_pressGlobalPosition = globalPosition;
        m_active = true;
        m_timer.start();
    }

    void update(const QPoint &globalPosition)
    {
        if (m_active
            && (globalPosition - m_pressGlobalPosition).manhattanLength() > QApplication::startDragDistance()) {
            cancel();
        }
    }

    void cancel()
    {
        m_active = false;
        m_timer.stop();
        dismissShownToolTip();
    }

    void dismissShownToolTip()
    {
        if (m_longPressShown && m_hideToolTip)
            m_hideToolTip();
        m_longPressShown = false;
    }

    QWidget *m_target {nullptr};
    std::function<void(const QPoint &)> m_showToolTip;
    std::function<void()> m_hideToolTip;
    QTimer m_timer;
    QPoint m_pressPosition;
    QPoint m_pressGlobalPosition;
    bool m_active {false};
    bool m_longPressShown {false};
};

}


struct  Q_DECL_HIDDEN KisResourceItemListView::Private
{
    Private(KisResourceItemListView *_q) : tip(_q) {}

    ListViewMode viewMode = ListViewMode::IconGrid;
    bool strictSelectionMode {false};
    KisIconToolTip tip;

    QScroller* scroller {0};
    QString prev_scrollbar_style;

    QSize requestedItemSize = QSize(56, 56);
    QPointer<TouchStylusLongPressFilter> longPressFilter;
};

KisResourceItemListView::KisResourceItemListView(QWidget *parent)
    : QListView(parent)
    , m_d(new Private(this))
{
    setSelectionMode(QAbstractItemView::SingleSelection);
#ifdef Q_OS_HARMONY
    // Resource tiles are selectors, not inline text editors. Advertising input
    // method support here makes an ordinary preset click open the software
    // keyboard. Real search QLineEdits remain input-method enabled.
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAttribute(Qt::WA_InputMethodEnabled, false);
    viewport()->setAttribute(Qt::WA_InputMethodEnabled, false);
#endif
    setContextMenuPolicy(Qt::DefaultContextMenu);   
    setResizeMode(QListView::Adjust);
    setUniformItemSizes(true);

    // Default configuration
    setViewMode(QListView::IconMode);
    setGridSize(QSize(56, 56));
    setIconSize(QSize(56, 56));
    setResizeMode(QListView::Adjust);
    setUniformItemSizes(true);

    m_d->scroller = KisKineticScroller::createPreconfiguredScroller(this);
    if (m_d->scroller) {
        connect(m_d->scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChange(QScroller::State)));
    }

    connect(this, SIGNAL(clicked(QModelIndex)), SIGNAL(currentResourceClicked(const QModelIndex &)));

    m_d->prev_scrollbar_style = horizontalScrollBar()->styleSheet();
}

KisResourceItemListView::~KisResourceItemListView()
{
}

void KisResourceItemListView::setListViewMode(ListViewMode viewMode)
{
    m_d->viewMode = viewMode;

    auto restoreScrollbar = [&, this] () {
        horizontalScrollBar()->setStyleSheet(m_d->prev_scrollbar_style);
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    };

    switch (viewMode) {
    case ListViewMode::IconGrid: {
        setViewMode(ViewMode::IconMode);
        setFlow(Flow::LeftToRight);
        setWrapping(true);
        restoreScrollbar();

        setItemSize(m_d->requestedItemSize);
        break;
    }
    case ListViewMode::IconStripHorizontal: {
        setViewMode(ViewMode::IconMode);
        setFlow(Flow::LeftToRight);
        setWrapping(false);

        // this is the only way to hide it and not have it occupy space
        horizontalScrollBar()->setStyleSheet("QScrollBar::horizontal {height: 0px;}");
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

        setItemSize(m_d->requestedItemSize);
        break;
    }
    case ListViewMode::Detail: {
        setViewMode(ViewMode::ListMode);
        setFlow(Flow::TopToBottom);
        setWrapping(false);
        restoreScrollbar();
        setItemSize(m_d->requestedItemSize);
        // horizontalScrollBar()->setStyleSheet(m_d->prev_scrollbar_style);
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        break;
    }
    }
}

void KisResourceItemListView::setItemSize(QSize size)
{
    m_d->requestedItemSize = size;

    switch (m_d->viewMode) {
    case ListViewMode::IconGrid: {
        setGridSize(size);
        setIconSize(size);
        break;
    }
    case ListViewMode::IconStripHorizontal: {
        // you can not set the item size in strip mode
        // it is configured automatically based on size
        break;
    }
    case ListViewMode::Detail: {
        const int w = width() - horizontalScrollBar()->width();
        setGridSize(QSize(w, size.height()));
        setIconSize(QSize(size));
        break;
    }
    }
}

void KisResourceItemListView::setStrictSelectionMode(bool enable)
{
    m_d->strictSelectionMode = enable;
}

void KisResourceItemListView::setFixedToolTipThumbnailSize(const QSize &size)
{
    m_d->tip.setFixedToolTipThumbnailSize(size);
}

void KisResourceItemListView::setToolTipShouldRenderCheckers(bool value)
{
    m_d->tip.setToolTipShouldRenderCheckers(value);
}

void KisResourceItemListView::setLongPressToolTipsEnabled(bool enabled)
{
    if (enabled == !m_d->longPressFilter.isNull())
        return;

    if (!enabled) {
        delete m_d->longPressFilter;
        m_d->longPressFilter = nullptr;
        return;
    }

    m_d->longPressFilter = new TouchStylusLongPressFilter(
        viewport(),
        [this](const QPoint &position) {
            if (!model())
                return;

            const QModelIndex index = model()->buddy(indexAt(position));
            if (!index.isValid())
                return;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QStyleOptionViewItem option = viewOptions();
#else
            QStyleOptionViewItem option;
            initViewItemOption(&option);
#endif
            option.rect = visualRect(index);
            m_d->tip.showTip(position, option, index);
        },
        [this]() {
            m_d->tip.hide();
        },
        this);
}

void KisResourceItemListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    // QAbstractItemView moves the current index if the row it points to is removed,
    // which we don't want for strict selections
    QModelIndex current = currentIndex();
    if (selectionMode() == SingleSelection
            && m_d->strictSelectionMode
            && current.isValid()
            && current.row() >= start
            && current.row() <= end) {

        selectionModel()->clear();
    }
    QListView::rowsAboutToBeRemoved(parent, start, end);
}

void KisResourceItemListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // base class takes care of viewport updates
    QListView::selectionChanged(selected, deselected);

    if (selected.isEmpty()) {
        Q_EMIT currentResourceChanged(QModelIndex());
    }
    else {
        Q_EMIT currentResourceChanged(selected.indexes().first());
    }
}

QItemSelectionModel::SelectionFlags KisResourceItemListView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    QItemSelectionModel::SelectionFlags cmd = QListView::selectionCommand(index, event);

    // avoid deselecting the current item by Ctrl-clicking in single selection mode
    if (selectionMode() == SingleSelection
            && m_d->strictSelectionMode
            && cmd.testFlag(QItemSelectionModel::Deselect)) {

        cmd = QItemSelectionModel::NoUpdate;
    }
    return cmd;
}

void KisResourceItemListView::contextMenuEvent(QContextMenuEvent *event)
{
    QListView::contextMenuEvent(event);
    Q_EMIT contextMenuRequested(event->globalPos());
}

bool KisResourceItemListView::viewportEvent(QEvent *event)
{
    if (!model()) return true;

    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *he = static_cast<QHelpEvent *>(event);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QStyleOptionViewItem option = viewOptions();
#else
        QStyleOptionViewItem option;
        initViewItemOption(&option);
#endif
        QModelIndex index = model()->buddy(indexAt(he->pos()));
        if (index.isValid()) {
            option.rect = visualRect(index);
            m_d->tip.showTip(he->pos(), option, index);
            return true;
        }
        m_d->tip.hide();
    }

    return QListView::viewportEvent(event);
}

void KisResourceItemListView::resizeEvent(QResizeEvent *event)
{
    QListView::resizeEvent(event);

    switch (m_d->viewMode) {
    case ListViewMode::IconStripHorizontal: {
        const int height = event->size().height();
        setGridSize(QSize(height, height));
        setIconSize(QSize(height, height));
        break;
    }
    case ListViewMode::Detail: {
        setItemSize(m_d->requestedItemSize);
        break;
    }
    case ListViewMode::IconGrid: {
        break;
    }
    }
    scrollTo(currentIndex(), QAbstractItemView::PositionAtCenter);
}
