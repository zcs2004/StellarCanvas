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

#include "KisMultiFeedRSSModel.h"

#include <QThread>
#include <QXmlStreamReader>
#include <QCoreApplication>
#include <QLocale>
#include <QFile>
#include <QTextBlock>
#include <QTextDocument>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <KisNetworkAccessManager.h>

#include <KisRssReader.h>
#include <QApplication>
#include <QDebug>
#include <QMetaObject>

#ifdef Q_OS_HARMONY
#include <network/netstack/net_http.h>
#endif

#ifdef Q_OS_HARMONY
QPointer<MultiFeedRssModel> MultiFeedRssModel::s_activeNativeModel;
#endif

MultiFeedRssModel::MultiFeedRssModel(QObject *parent) :
    QAbstractListModel(parent),
    m_networkAccessManager(new KisNetworkAccessManager),
    m_articleCount(0)
#ifdef Q_OS_HARMONY
    , m_useNativeHttp(true)
#endif
{
    initialize();
}

MultiFeedRssModel::MultiFeedRssModel(KisNetworkAccessManager* nam, QObject* parent)
    : QAbstractListModel(parent),
      m_networkAccessManager(nam),
      m_articleCount(0)
#ifdef Q_OS_HARMONY
      , m_useNativeHttp(false)
#endif
{
    initialize();
}


MultiFeedRssModel::~MultiFeedRssModel()
{
#ifdef Q_OS_HARMONY
    if (s_activeNativeModel == this) {
        s_activeNativeModel.clear();
        if (m_activeNativeRequest) {
            OH_Http_Destroy(&m_activeNativeRequest);
        }
    }
#endif
    delete m_networkAccessManager;
}

QHash<int, QByteArray> MultiFeedRssModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames[KisRssReader::RssRoles::TitleRole] = "title";
    roleNames[KisRssReader::RssRoles::DescriptionRole] = "description";
    roleNames[KisRssReader::RssRoles::PubDateRole] = "pubDate";
    roleNames[KisRssReader::RssRoles::LinkRole] = "link";
    roleNames[KisRssReader::RssRoles::CategoryRole] = "category";
    roleNames[KisRssReader::RssRoles::BlogNameRole] = "blogName";
    roleNames[KisRssReader::RssRoles::BlogIconRole] = "blogIcon";
    return roleNames;
}

void MultiFeedRssModel::addFeed(const QString& feed)
{
    if (m_sites.contains(feed)) {
        // do not add the feed twice
        return;
    }

    m_sites << feed;

#ifdef Q_OS_HARMONY
    // Qt for HarmonyOS is currently packaged without a TLS backend. Use the
    // platform HTTP client so HTTPS news feeds still use the system trust
    // store and never need to fall back to clear-text transport.
    if (m_useNativeHttp) {
        m_pendingNativeFeeds << feed;
        startNextNativeFeed();
        return;
    }
#endif

    const QUrl feedUrl(feed);
    m_networkAccessManager->getUrl(feedUrl);
}

bool sortForPubDate(const RssItem& item1, const RssItem& item2)
{
    return item1.pubDate > item2.pubDate;
}

void MultiFeedRssModel::appendFeedData(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Unable to retrieve news feed" << reply->request().url()
                   << reply->error() << reply->errorString();
        Q_EMIT feedDataChanged();
        return;
    }

    beginResetModel();
    KisRssReader reader;
    const RssItemList items = reader.parse(reply);
    m_aggregatedFeed.append(items);
    sortAggregatedFeed();
    setArticleCount(m_aggregatedFeed.size());
    endResetModel();

    qInfo() << "Retrieved" << items.size() << "news item(s) from" << reply->request().url();

    Q_EMIT feedDataChanged();
}

void MultiFeedRssModel::sortAggregatedFeed()
{
    std::sort(m_aggregatedFeed.begin(), m_aggregatedFeed.end(), sortForPubDate);
}

void MultiFeedRssModel::initialize()
{
    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            SLOT(appendFeedData(QNetworkReply*)), Qt::QueuedConnection);
}

void MultiFeedRssModel::removeFeed(const QString &feed)
{
#ifdef Q_OS_HARMONY
    m_pendingNativeFeeds.removeAll(feed);
#endif
    bool isRemoved = m_sites.removeOne(feed);
    if (isRemoved) {
        beginResetModel();
        QMutableListIterator<RssItem> it(m_aggregatedFeed);
        while (it.hasNext()) {
            RssItem item = it.next();
            if (item.source == feed)
                it.remove();
        }
        setArticleCount(m_aggregatedFeed.size());
        endResetModel();

        Q_EMIT feedDataChanged();
    }
}

#ifdef Q_OS_HARMONY
void MultiFeedRssModel::startNextNativeFeed()
{
    if (m_activeNativeRequest || !s_activeNativeModel.isNull()) {
        return;
    }

    while (!m_pendingNativeFeeds.isEmpty()) {
        const QString feed = m_pendingNativeFeeds.takeFirst();
        if (!m_sites.contains(feed)) {
            continue;
        }

        const QByteArray encodedUrl = feed.toUtf8();
        m_activeNativeRequest = OH_Http_CreateRequest(encodedUrl.constData());
        if (!m_activeNativeRequest) {
            qWarning() << "Unable to create native HarmonyOS news request for" << feed;
            continue;
        }

        m_activeNativeFeed = feed;
        s_activeNativeModel = this;

        if (m_activeNativeRequest->options) {
            m_activeNativeRequest->options->method = NET_HTTP_METHOD_GET;
            m_activeNativeRequest->options->connectTimeout = 15000;
            m_activeNativeRequest->options->readTimeout = 15000;
            m_activeNativeRequest->options->httpProtocol = OH_HTTP_NONE;
            m_activeNativeRequest->options->headers = OH_Http_CreateHeaders();
            if (m_activeNativeRequest->options->headers) {
                OH_Http_SetHeaderValue(m_activeNativeRequest->options->headers, "Accept", "application/rss+xml, application/xml;q=0.9, */*;q=0.8");
                OH_Http_SetHeaderValue(m_activeNativeRequest->options->headers, "User-Agent", "StellarCanvas/1.0 (HarmonyOS News Reader)");
            }
        }

        Http_EventsHandler events {};
        const int result = OH_Http_Request(m_activeNativeRequest, &MultiFeedRssModel::nativeHttpResponse, events);
        if (result == OH_HTTP_RESULT_OK) {
            qInfo() << "Requesting StellarCanvas news with the HarmonyOS network stack:" << feed;
            return;
        }

        qWarning() << "Unable to start native HarmonyOS news request" << feed << "error" << result;
        s_activeNativeModel.clear();
        OH_Http_Destroy(&m_activeNativeRequest);
        m_activeNativeFeed.clear();
    }
}

void MultiFeedRssModel::nativeHttpResponse(Http_Response *response, uint32_t errorCode)
{
    const QPointer<MultiFeedRssModel> model = s_activeNativeModel;
    s_activeNativeModel.clear();

    QByteArray data;
    int statusCode = 0;
    QString feed;
    Http_Request *request = nullptr;

    if (model) {
        feed = model->m_activeNativeFeed;
        model->m_activeNativeFeed.clear();
        request = model->m_activeNativeRequest;
        model->m_activeNativeRequest = nullptr;
    }

    if (response) {
        statusCode = static_cast<int>(response->responseCode);
        if (response->body.buffer && response->body.length > 0) {
            data = QByteArray(response->body.buffer, static_cast<qsizetype>(response->body.length));
        }
        if (response->destroyResponse) {
            response->destroyResponse(&response);
        }
    }

    if (request) {
        OH_Http_Destroy(&request);
    }

    if (!model) {
        return;
    }

    QCoreApplication *application = QCoreApplication::instance();
    if (!application) {
        return;
    }

    QMetaObject::invokeMethod(application, [model, feed, data, statusCode, errorCode]() {
        if (!model) {
            return;
        }
        model->appendNativeFeedData(feed, data, statusCode, errorCode);
        model->startNextNativeFeed();
    }, Qt::QueuedConnection);
}

void MultiFeedRssModel::appendNativeFeedData(const QString &feed, const QByteArray &data, int statusCode, uint32_t errorCode)
{
    if (!m_sites.contains(feed)) {
        return;
    }

    if (errorCode != OH_HTTP_RESULT_OK || statusCode < 200 || statusCode >= 300) {
        qWarning() << "Unable to retrieve StellarCanvas news feed" << feed
                   << "HTTP status" << statusCode << "HarmonyOS error" << errorCode;
        Q_EMIT feedDataChanged();
        return;
    }

    KisRssReader reader;
    const RssItemList items = reader.parse(data, feed);

    beginResetModel();
    m_aggregatedFeed.append(items);
    sortAggregatedFeed();
    setArticleCount(m_aggregatedFeed.size());
    endResetModel();

    qInfo() << "Retrieved" << items.size() << "StellarCanvas news item(s) from" << feed;
    Q_EMIT feedDataChanged();
}
#endif

int MultiFeedRssModel::rowCount(const QModelIndex &) const
{
    return m_aggregatedFeed.size();
}

QVariant MultiFeedRssModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < m_aggregatedFeed.size()) {
        RssItem item = m_aggregatedFeed.at(index.row());

        switch (role) {
        case Qt::DisplayRole:
        {
            QTextDocument doc;
            doc.setHtml(item.description);
            // Extract the first text block, which is the `<p>` element containing
            // the shortened post text, excluding the "This post [...] appeared
            // first on [...]" text.
            QString text = doc.firstBlock().text();
            if (text.length() > 292) {
                text.truncate(290);
                text.append("...");
            }
            return QString("<b><a href=\"" + item.link + "\">" + item.title + "</a></b>"
                "<br><small>(" + QLocale().toString(item.pubDate.toLocalTime(), QLocale::ShortFormat) + ") "
                "<p style=\"margin-top: 4px\">" + text + "</p></small>");
        }
        case KisRssReader::RssRoles::TitleRole:
            return item.title;
        case KisRssReader::RssRoles::DescriptionRole:
            return item.description;
        case KisRssReader::RssRoles::PubDateRole:
            return item.pubDate.toString("dd-MM-yyyy hh:mm");
        case KisRssReader::RssRoles::LinkRole:
            return item.link;
        case KisRssReader::RssRoles::CategoryRole:
            return item.category;
        case KisRssReader::RssRoles::BlogNameRole:
            return item.blogName;
        case KisRssReader::RssRoles::BlogIconRole:
            return item.blogIcon;
        }
    }

    return QVariant();
}
