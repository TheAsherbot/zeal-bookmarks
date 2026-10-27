// Copyright (C) Oleg Shparber, et al. <https://zealdocs.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZEAL_CORE_BOOKMARKMANAGER_H
#define ZEAL_CORE_BOOKMARKMANAGER_H

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>

namespace Zeal::Core {

struct BookmarkItem
{
    QString title;
    QUrl url;
    QString docsetName;
    QDateTime dateAdded;

    bool operator==(const BookmarkItem &other) const { return url == other.url; }
};

class BookmarkManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(BookmarkManager)
public:
    explicit BookmarkManager(QObject *parent = nullptr);
    ~BookmarkManager() override;

    static BookmarkManager *instance();
    static QUrl resolveUrl(const QUrl &url);
    static bool areUrlsEqual(const QUrl &u1, const QUrl &u2);

    const QList<BookmarkItem> &bookmarks() const;
    bool isBookmarked(const QUrl &url) const;

    bool addBookmark(const QString &title, const QUrl &url, const QString &docsetName = QString());
    bool removeBookmark(const QUrl &url);
    bool removeBookmarkAt(int index);

    void load();
    void save();

signals:
    void bookmarksChanged();
    void bookmarkAdded(const BookmarkItem &item);
    void bookmarkRemoved(const QUrl &url);

private:
    static BookmarkManager *s_instance;
    QList<BookmarkItem> m_bookmarks;
    QString m_filePath;
};

} // namespace Zeal::Core

#endif // ZEAL_CORE_BOOKMARKMANAGER_H
