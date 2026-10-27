// Copyright (C) Oleg Shparber, et al. <https://zealdocs.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZEAL_WIDGETUI_BOOKMARKSIDEBAR_H
#define ZEAL_WIDGETUI_BOOKMARKSIDEBAR_H

#include <sidebar/view.h>

#include <QUrl>
#include <QWidget>

class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;

namespace Zeal::WidgetUi {

class BookmarkSidebar final : public Sidebar::View
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(BookmarkSidebar)
public:
    explicit BookmarkSidebar(QWidget *parent = nullptr);
    ~BookmarkSidebar() override = default;

public slots:
    void reloadBookmarks();

signals:
    void navigationRequested(const QUrl &url);
    void openInNewTabRequested(const QUrl &url, bool activate = false);

private slots:
    void onItemActivated(QTreeWidgetItem *item, int column);
    void onFilterTextChanged(const QString &text);
    void showContextMenu(const QPoint &pos);

private:
    void setupUi();

    QLineEdit *m_filterEdit = nullptr;
    QTreeWidget *m_treeWidget = nullptr;
    QLabel *m_emptyStateLabel = nullptr;
};

} // namespace Zeal::WidgetUi

#endif // ZEAL_WIDGETUI_BOOKMARKSIDEBAR_H
