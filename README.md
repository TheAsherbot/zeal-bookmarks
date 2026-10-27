# Zeal (with Native Bookmarks & Favorites ⭐)

> [!NOTE]
> **Upstream Project Notice**:
> This repository is a community fork of the official **[Zeal](https://github.com/zealdocs/zeal)** offline documentation browser created by Oleg Shparber and contributors.
> - **Official Upstream Repository**: [https://github.com/zealdocs/zeal](https://github.com/zealdocs/zeal)
> - **Official Website & Docs**: [https://zealdocs.org](https://zealdocs.org)
>
> If you are looking for the original, unmodified Zeal application without custom bookmarks, please visit the upstream links above.

---

[![GitHub Releases](https://img.shields.io/github/v/release/TheAsherbot/zeal-bookmarks?include_prereleases&style=flat-square&color=success)](https://github.com/TheAsherbot/zeal-bookmarks/releases)
[![Build Check](https://img.shields.io/github/actions/workflow/status/TheAsherbot/zeal-bookmarks/build-check.yaml?style=flat-square)](https://github.com/TheAsherbot/zeal-bookmarks/actions/workflows/build-check.yaml)
[![License](https://img.shields.io/github/license/TheAsherbot/zeal-bookmarks?style=flat-square)](COPYING)

Zeal is an offline documentation browser: your personal reference library, searchable in an instant and available without an internet connection. Originally inspired by [Dash](https://kapeli.com/dash), it supports the standard Apple docset format.

---

## 🌟 What's New in This Fork

This fork implements one of the community's longest-standing feature requests: **Native Bookmarks & Favorites**, built with persistent local storage and dynamic internal port translation.

### Key Features:
- **Instant Bookmark Shortcut (`Ctrl + D`)**: Bookmark whatever documentation page you are reading with a single keystroke.
- **Dedicated Bookmarks Sidebar Tab (`⭐ Bookmarks`)**: A clean tabbed panel in the left sidebar alongside the standard Docset index.
  - **Auto-grouped by Docset**: Bookmarks are automatically organized into folders by docset name (e.g. `DirectX 12`, `Win32`, `Python`, `C++`).
  - **Live Search / Filter**: Instant real-time filtering across bookmark titles and docset categories.
  - **Full Context Menu**: Right-click any bookmark to **Open**, **Open in New Tab**, **Copy URL**, or **Delete**.
- **Top Menu Bar (`&Bookmarks`)**: Access, jump to, and browse your bookmarks from the main application menu bar anytime (`Ctrl + Shift + B` toggles the tab).
- **Dynamic Port Translation**: Zeal's internal documentation HTTP server binds to a random port on each launch (e.g. `:62944` on run 1, `:51234` on run 2). This fork automatically resolves stored bookmark URLs to the live server port so saved bookmarks **never 404 on app restarts**.
- **Universal Compatibility**: Works seamlessly with both prebuilt official docsets and custom local docsets (such as Win32, DirectX 12, or custom engine documentation).

---

## 🚀 Download & Installation

Precompiled binaries for Windows and Linux are available under **[GitHub Releases](https://github.com/TheAsherbot/zeal-bookmarks/releases)**:

- **Windows Installer (`.msi`)**: Full Windows setup that registers Zeal in your Start Menu and Programs list.
- **Windows Portable (`.zip`)**: Standalone archive—extract anywhere and run `zeal.exe`.
- **Linux AppImage (`.AppImage`)**: Self-contained executable for Ubuntu, Debian, Fedora, Arch, and other distributions.

### Quick Install from Source (Windows):
If building locally from source:
1. Run `install_to_program_files.bat` as Administrator.
2. The script will automatically uninstall any previous stock Zeal installation, deploy the custom release into `C:\Program Files\Zeal\`, create Start Menu and Desktop shortcuts, and register Zeal in Windows App Paths so you can run `zeal` from `Win + R` or PowerShell.

---

## 📖 How to Use Bookmarks

1. **Add a Bookmark**: Open any documentation article and press **`Ctrl + D`** (or click `Bookmarks → Bookmark Current Page`).
2. **Switch to Bookmarks**: Click the **Bookmarks** tab in the sidebar or press **`Ctrl + Shift + B`**.
3. **Filter**: Type in the search box at the top of the sidebar to filter your bookmarks in real time.
4. **Open in New Tab**: Right-click any bookmark and choose **Open in New Tab** (or middle-click).
5. **Delete**: Right-click any bookmark and choose **Delete Bookmark**.

---

## 🔍 How to Use Docsets (Standard Zeal Features)

After installing Zeal, go to `File → Docset Library`, select the docsets you want, and click the `Download` button.

### Query and filter docsets
Limit the search scope by prefixing your query with a docset name and a colon:
```text
cpp:vector
```
To search multiple docsets, separate them with a comma:
```text
python,django:string
```

### Command line
You can also launch Zeal with a query directly from the terminal or Run prompt:
```bash
zeal python:pprint
zeal win32:CreateFile
```

---

## 🛠️ How to Compile

### Build dependencies
* [CMake](https://cmake.org/) and [Ninja](https://ninja-build.org/).
* [Qt](https://www.qt.io/) version 6.4.2 or later (modules: Core, Gui, Widgets, Network, Svg, WebEngineCore, WebEngineWidgets, WebChannel).
* [libarchive](https://libarchive.org/).
* [sqlite3](https://sqlite.org/).
* [tomlplusplus](https://marzer.github.io/tomlplusplus/).

### Windows Build (MSVC + Ninja + vcpkg):
```powershell
# 1. Configure
cmake -B build -S . -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="<vcpkg_root>/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="<qt6_install_prefix>"

# 2. Build
cmake --build build --config Release

# 3. Deploy Qt runtime DLLs
windeployqt --release build/zeal.exe
```

---

## 📜 Credits & License

- **Original Zeal Author**: [Oleg Shparber](https://github.com/trollixx) and the [Zeal Contributors](https://github.com/zealdocs/zeal/graphs/contributors).
- **Bookmarks & Favorites Implementation**: [Asher Jordan](https://github.com/TheAsherbot).
- **License**: GNU General Public License v3.0 or later ([COPYING](COPYING)).
