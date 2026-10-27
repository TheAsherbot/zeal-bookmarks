// Copyright (C) Oleg Shparber, et al. <https://zealdocs.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmarkmanager.h"
#include "application.h"
#include "httpserver.h"

#include <registry/docset.h>
#include <registry/docsetregistry.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace Zeal::Core {

BookmarkManager *BookmarkManager::s_instance = nullptr;

BookmarkManager::BookmarkManager(QObject *parent)
    : QObject(parent)
{
    if (!s_instance) {
        s_instance = this;
    }
    m_filePath = Application::configLocation() + QStringLiteral("/bookmarks.json");
    load();
}

BookmarkManager::~BookmarkManager()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

BookmarkManager *BookmarkManager::instance()
{
    return s_instance;
}

QUrl BookmarkManager::resolveUrl(const QUrl &url)
{
    if (url.isEmpty()) {
        return url;
    }

    if ((url.host() == QStringLiteral("127.0.0.1") || url.host() == QStringLiteral("localhost"))
        && url.scheme() == QStringLiteral("http")) {
        auto *app = Application::instance();
        if (app && app->httpServer() && app->httpServer()->isListening()) {
            const QUrl base = app->httpServer()->baseUrl();
            QUrl resolved = url;
            resolved.setHost(base.host());
            resolved.setPort(base.port());
            return resolved;
        }
    }

    return url;
}

bool BookmarkManager::areUrlsEqual(const QUrl &u1, const QUrl &u2)
{
    if (u1 == u2) {
        return true;
    }

    const bool isLocal1 = (u1.host() == QStringLiteral("127.0.0.1") || u1.host() == QStringLiteral("localhost"));
    const bool isLocal2 = (u2.host() == QStringLiteral("127.0.0.1") || u2.host() == QStringLiteral("localhost"));

    if (isLocal1 && isLocal2) {
        return u1.path() == u2.path() && u1.query() == u2.query() && u1.fragment() == u2.fragment();
    }

    return false;
}

const QList<BookmarkItem> &BookmarkManager::bookmarks() const
{
    return m_bookmarks;
}

bool BookmarkManager::isBookmarked(const QUrl &url) const
{
    for (const auto &item : m_bookmarks) {
        if (areUrlsEqual(item.url, url)) {
            return true;
        }
    }
    return false;
}

bool BookmarkManager::addBookmark(const QString &title, const QUrl &url, const QString &docsetName)
{
    if (url.isEmpty()) {
        return false;
    }

    if (isBookmarked(url)) {
        return false;
    }

    QString docset = docsetName;
    if (docset.isEmpty()) {
        auto *app = Application::instance();
        if (app && app->docsetRegistry()) {
            if (auto *ds = app->docsetRegistry()->docsetForUrl(url)) {
                docset = ds->title();
                if (docset.isEmpty()) {
                    docset = ds->name();
                }
            }
        }
    }

    if (docset.isEmpty()) {
        if (url.host() == QStringLiteral("127.0.0.1") || url.host() == QStringLiteral("localhost")) {
            const QString p = url.path();
            const QStringList parts = p.split(QLatin1Char('/'), Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                docset = parts.first();
                docset.replace(QLatin1Char('_'), QLatin1Char(' '));
            }
        }
    }

    if (docset.isEmpty()) {
        static const QRegularExpression docsetRegex(QStringLiteral("([^/\\\\]+)\\.docset"), QRegularExpression::CaseInsensitiveOption);
        const auto match = docsetRegex.match(url.toString());
        if (match.hasMatch()) {
            docset = match.captured(1);
        } else {
            docset = QStringLiteral("General");
        }
    }

    BookmarkItem item;
    item.title = title.trimmed().isEmpty() ? url.fileName() : title.trimmed();
    item.url = url;
    item.docsetName = docset;
    item.dateAdded = QDateTime::currentDateTime();

    m_bookmarks.append(item);
    save();

    emit bookmarkAdded(item);
    emit bookmarksChanged();
    return true;
}

bool BookmarkManager::removeBookmark(const QUrl &url)
{
    for (int i = 0; i < m_bookmarks.size(); ++i) {
        if (areUrlsEqual(m_bookmarks[i].url, url)) {
            m_bookmarks.removeAt(i);
            save();
            emit bookmarkRemoved(url);
            emit bookmarksChanged();
            return true;
        }
    }
    return false;
}

bool BookmarkManager::removeBookmarkAt(int index)
{
    if (index >= 0 && index < m_bookmarks.size()) {
        const QUrl url = m_bookmarks[index].url;
        m_bookmarks.removeAt(index);
        save();
        emit bookmarkRemoved(url);
        emit bookmarksChanged();
        return true;
    }
    return false;
}

void BookmarkManager::load()
{
    m_bookmarks.clear();

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return;
    }

    const QJsonArray array = doc.array();
    for (const auto &val : array) {
        if (!val.isObject()) {
            continue;
        }
        const QJsonObject obj = val.toObject();

        BookmarkItem item;
        item.title = obj.value(QStringLiteral("title")).toString();
        item.url = QUrl(obj.value(QStringLiteral("url")).toString());
        item.docsetName = obj.value(QStringLiteral("docset")).toString();
        item.dateAdded = QDateTime::fromString(obj.value(QStringLiteral("dateAdded")).toString(), Qt::ISODate);

        if (!item.url.isEmpty()) {
            m_bookmarks.append(item);
        }
    }
}

void BookmarkManager::save()
{
    QFileInfo fi(m_filePath);
    QDir().mkpath(fi.absolutePath());

    QJsonArray array;
    for (const auto &item : m_bookmarks) {
        QJsonObject obj;
        obj[QStringLiteral("title")] = item.title;
        obj[QStringLiteral("url")] = item.url.toString();
        obj[QStringLiteral("docset")] = item.docsetName;
        obj[QStringLiteral("dateAdded")] = item.dateAdded.toString(Qt::ISODate);
        array.append(obj);
    }

    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    }
}

} // namespace Zeal::Core
