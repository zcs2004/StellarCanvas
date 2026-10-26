/*
 * SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KO_TOOLBOXBUTTON_H_
#define _KO_TOOLBOXBUTTON_H_

#include <QToolButton>
#include <QTimer>

class KoToolAction;

class KoToolBoxButton : public QToolButton
{
    Q_OBJECT
public:
    explicit KoToolBoxButton(KoToolAction *toolAction, QWidget * parent);
    void attachAction(QAction *action);
    void setHighlightColor();

private:
    void setDataFromToolAction(QAction *action); // Generates tooltips.
    bool event(QEvent *event) override;
    void beginLongPress(const QPoint &position);
    void updateLongPress(const QPoint &position);
    void cancelLongPress();
private Q_SLOTS:
    void slotUpdateActionData();
private:
    KoToolAction *m_toolAction;
    QTimer m_longPressTimer;
    QPoint m_longPressPosition;
    bool m_longPressActive {false};
};

#endif // _KO_TOOLBOXBUTTON_H_
