/**************************************************************************
**
** This file is part of Qt Creator
**
** SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
**
** SPDX-License-Identifier: LGPL-2.1-only
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#ifndef MULTIFEEDRSSMODEL_H
#define MULTIFEEDRSSMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QDateTime>
#include <QPointer>
#include <cstdint>

#include <KisRssReader.h>

#include <kritaui_export.h>


class QNetworkReply;
class KisNetworkAccessManager;

#ifdef Q_OS_HARMONY
struct Http_Request;
struct Http_Response;
#endif

class KRITAUI_EXPORT MultiFeedRssModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int articleCount READ articleCount WRITE setArticleCount NOTIFY articleCountChanged)
public:
    explicit MultiFeedRssModel(QObject *parent = 0);
    explicit MultiFeedRssModel(KisNetworkAccessManager* nam, QObject *parent = 0);
    ~MultiFeedRssModel() override;

    QHash<int, QByteArray> roleNames() const override;
    virtual void addFeed(const QString& feed);
    void removeFeed(const QString& feed);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int articleCount() const {
        return m_articleCount;
    }

public Q_SLOTS:
    void setArticleCount(int arg) {
        if (m_articleCount != arg) {
            m_articleCount = arg;
            Q_EMIT articleCountChanged(arg);
        }
    }

Q_SIGNALS:
    void articleCountChanged(int arg);
    void feedDataChanged();

private Q_SLOTS:
    void appendFeedData(QNetworkReply *reply);

private:
    QStringList m_sites;
    RssItemList m_aggregatedFeed;
    KisNetworkAccessManager *m_networkAccessManager;
    int m_articleCount;

#ifdef Q_OS_HARMONY
    bool m_useNativeHttp;
    QStringList m_pendingNativeFeeds;
    QString m_activeNativeFeed;
    Http_Request *m_activeNativeRequest {nullptr};
    static QPointer<MultiFeedRssModel> s_activeNativeModel;

    static void nativeHttpResponse(Http_Response *response, uint32_t errorCode);
    void startNextNativeFeed();
    void appendNativeFeedData(const QString &feed, const QByteArray &data, int statusCode, uint32_t errorCode);
#endif

    void sortAggregatedFeed();
    void initialize();

    friend class MockMultiFeedRssModel;
};

#endif // MULTIFEEDRSSMODEL_H
