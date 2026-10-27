// Copyright (C) Oleg Shparber, et al. <https://zealdocs.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmarksidebar.h"

#include <core/bookmarkmanager.h>
#include <ui/widgets/layouthelper.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFont>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QMouseEvent>
#include <QStyle>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace Zeal::WidgetUi {

BookmarkSidebar::BookmarkSidebar(QWidget *parent)
    : Sidebar::View(parent)
{
    setupUi();

    if (auto *bm = Core::BookmarkManager::instance()) {
        connect(bm, &Core::BookmarkManager::bookmarksChanged, this, &BookmarkSidebar::reloadBookmarks);
    }

    reloadBookmarks();
}

void BookmarkSidebar::setupUi()
{
    auto *layout = LayoutHelper::createBorderlessLayout<QVBoxLayout>();
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText(tr("Search bookmarks..."));
    m_filterEdit->setClearButtonEnabled(true);
    connect(m_filterEdit, &QLineEdit::textChanged, this, &BookmarkSidebar::onFilterTextChanged);
    layout->addWidget(m_filterEdit);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setIndentation(16);
    m_treeWidget->setAnimated(true);
    m_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeWidget->setStyleSheet(QStringLiteral("QTreeWidget::item:hover:!selected { background: palette(midlight); }"));

    connect(m_treeWidget, &QTreeWidget::itemActivated, this, &BookmarkSidebar::onItemActivated);
    connect(m_treeWidget, &QTreeWidget::itemClicked, this, &BookmarkSidebar::onItemActivated);
    connect(m_treeWidget, &QTreeWidget::customContextMenuRequested, this, &BookmarkSidebar::showContextMenu);
    layout->addWidget(m_treeWidget);

    m_emptyStateLabel = new QLabel(tr("No bookmarks yet.\nPress Ctrl+D on any page to bookmark it!"), this);
    m_emptyStateLabel->setAlignment(Qt::AlignCenter);
    m_emptyStateLabel->setWordWrap(true);
    m_emptyStateLabel->setStyleSheet(QStringLiteral("color: palette(disabled); padding: 20px; font-style: italic;"));
    m_emptyStateLabel->hide();
    layout->addWidget(m_emptyStateLabel);

    setLayout(layout);
}

void BookmarkSidebar::reloadBookmarks()
{
    auto *bm = Core::BookmarkManager::instance();
    if (!bm) {
        return;
    }

    m_treeWidget->clear();

    const auto &bookmarks = bm->bookmarks();
    if (bookmarks.isEmpty()) {
        m_treeWidget->hide();
        m_emptyStateLabel->show();
        return;
    }

    m_emptyStateLabel->hide();
    m_treeWidget->show();

    // Group bookmarks by docset name
    QMap<QString, QList<Core::BookmarkItem>> groups;
    for (const auto &item : bookmarks) {
        groups[item.docsetName].append(item);
    }

    const QString filter = m_filterEdit->text().trimmed();

    for (auto it = groups.cbegin(); it != groups.cend(); ++it) {
        const QString &docsetName = it.key();
        const QList<Core::BookmarkItem> &items = it.value();

        auto *groupItem = new QTreeWidgetItem(m_treeWidget);
        groupItem->setText(0, QStringLiteral("%1 (%2)").arg(docsetName).arg(items.size()));
        QFont f = groupItem->font(0);
        f.setBold(true);
        groupItem->setFont(0, f);
        groupItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));

        bool hasVisibleChild = false;

        for (const auto &bItem : items) {
            auto *childItem = new QTreeWidgetItem(groupItem);
            childItem->setText(0, bItem.title);
            childItem->setData(0, Qt::UserRole, bItem.url.toString());
            childItem->setToolTip(0, QStringLiteral("%1\n%2\nAdded: %3")
                                         .arg(bItem.title,
                                              bItem.url.toString(),
                                              bItem.dateAdded.isValid() ? bItem.dateAdded.toString(QStringLiteral("yyyy-MM-dd HH:mm")) : QString()));
            childItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_FileIcon));

            if (!filter.isEmpty()) {
                const bool matches = bItem.title.contains(filter, Qt::CaseInsensitive)
                                     || docsetName.contains(filter, Qt::CaseInsensitive);
                childItem->setHidden(!matches);
                if (matches) {
                    hasVisibleChild = true;
                }
            } else {
                hasVisibleChild = true;
            }
        }

        if (!filter.isEmpty() && !hasVisibleChild) {
            groupItem->setHidden(true);
        } else {
            groupItem->setHidden(false);
            groupItem->setExpanded(true);
        }
    }
}

void BookmarkSidebar::onFilterTextChanged(const QString &text)
{
    const QString filter = text.trimmed();
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        auto *group = m_treeWidget->topLevelItem(i);
        bool hasVisible = false;

        for (int j = 0; j < group->childCount(); ++j) {
            auto *child = group->child(j);
            const QString title = child->text(0);
            const bool matches = filter.isEmpty()
                                 || title.contains(filter, Qt::CaseInsensitive)
                                 || group->text(0).contains(filter, Qt::CaseInsensitive);
            child->setHidden(!matches);
            if (matches) {
                hasVisible = true;
            }
        }

        group->setHidden(!hasVisible);
        if (hasVisible) {
            group->setExpanded(true);
        }
    }
}

void BookmarkSidebar::onItemActivated(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item) {
        return;
    }

    const QString urlStr = item->data(0, Qt::UserRole).toString();
    if (!urlStr.isEmpty()) {
        emit navigationRequested(Core::BookmarkManager::resolveUrl(QUrl(urlStr)));
    }
}

void BookmarkSidebar::showContextMenu(const QPoint &pos)
{
    auto *item = m_treeWidget->itemAt(pos);
    if (!item) {
        return;
    }

    const QString urlStr = item->data(0, Qt::UserRole).toString();
    if (urlStr.isEmpty()) {
        return; // Group header clicked
    }

    const QUrl url(urlStr);

    QMenu menu(this);
    auto *openAction = menu.addAction(QApplication::style()->standardIcon(QStyle::SP_ArrowRight), tr("&Open"));
    auto *openNewTabAction = menu.addAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder), tr("Open in &New Tab"));
    menu.addSeparator();
    auto *copyLinkAction = menu.addAction(tr("&Copy URL"));
    menu.addSeparator();
    auto *deleteAction = menu.addAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon), tr("&Delete Bookmark"));

    auto *selected = menu.exec(m_treeWidget->viewport()->mapToGlobal(pos));
    if (selected == openAction) {
        emit navigationRequested(Core::BookmarkManager::resolveUrl(url));
    } else if (selected == openNewTabAction) {
        emit openInNewTabRequested(Core::BookmarkManager::resolveUrl(url), true);
    } else if (selected == copyLinkAction) {
        QApplication::clipboard()->setText(Core::BookmarkManager::resolveUrl(url).toString());
    } else if (selected == deleteAction) {
        if (auto *bm = Core::BookmarkManager::instance()) {
            bm->removeBookmark(url);
        }
    }
}

} // namespace Zeal::WidgetUi
