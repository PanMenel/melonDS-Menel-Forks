<p align="center"><img src="https://raw.githubusercontent.com/melonDS-emu/melonDS/master/res/icon/melon_128x128.png"></p>
<h2 align="center"><b>melonDS Menel Forks</b></h2>
<p align="center">
<a href="http://melonds.kuribo64.net/" alt="melonDS website"><img src="https://img.shields.io/badge/website-melonds.kuribo64.net-%2331352e.svg"></a>
<a href="http://melonds.kuribo64.net/downloads.php" alt="Release: 1.1"><img src="https://img.shields.io/badge/release-1.1-%235c913b.svg"></a>
<a href="https://www.gnu.org/licenses/gpl-3.0" alt="License: GPLv3"><img src="https://img.shields.io/badge/License-GPL%20v3-%23ff554d.svg"></a>
<a href="https://kiwiirc.com/client/irc.badnik.net/?nick=IRC-Source_?#melonds" alt="IRC channel: #melonds"><img src="https://img.shields.io/badge/IRC%20chat-%23melonds-%23dd2e44.svg"></a>
<a href="https://discord.gg/pAMAtExcqV" alt="Discord"><img src="https://img.shields.io/badge/Discord-Kuribo64-7289da?logo=discord&logoColor=white"></a>
<br>
<a href="https://github.com/melonDS-emu/melonDS/actions/workflows/build-windows.yml?query=event%3Apush"><img src="https://github.com/melonDS-emu/melonDS/actions/workflows/build-windows.yml/badge.svg" /></a>
<a href="https://github.com/melonDS-emu/melonDS/actions/workflows/build-ubuntu.yml?query=event%3Apush"><img src="https://github.com/melonDS-emu/melonDS/actions/workflows/build-ubuntu.yml/badge.svg" /></a>
<a href="https://github.com/melonDS-emu/melonDS/actions/workflows/build-macos.yml?query=event%3Apush"><img src="https://github.com/melonDS-emu/melonDS/actions/workflows/build-macos.yml/badge.svg" /></a>
<a href="https://github.com/melonDS-emu/melonDS/actions/workflows/build-bsd.yml?query=event%3Apush"><img src="https://github.com/melonDS-emu/melonDS/actions/workflows/build-bsd.yml/badge.svg" /></a>
</p>

These are my forks of melonds adding features I thought should be there already

## 🪚 Ultimate branch [link to branch](https://github.com/PanMenel/melonDS-Menel-Forks/tree/Ultimate-branch)
*A branch containing all my features for easy access to all of them.*

## 📺 WindowBorderToggle Fork [link to branch](https://github.com/PanMenel/melonDS-Menel-Forks/tree/WindowBorderToggle)
*Implemented a hotkey to hide the Windows Border (goes well with menubartoggle).*

## 📺 MenuBarToggle Fork [link to branch](https://github.com/PanMenel/melonDS-Menel-Forks/tree/Menubartoggle)
*Implemented a hotkey to hide the Menu Bar.*


I created this because I recently got a vertical grip for my Nintendo Switch, which is perfect for DS games. When streaming melonDS from my PC via Moonlight, the Menu Bar was the last thing cluttering the screen.

> [!NOTE]
> **Full Disclosure:** I am still learning programming. This feature was developed using "vibecoding" (with AI assistance from Grok). While it might not be a 100% professional implementation, it works exactly as intended. Enjoy!

---

## 🏆 RetroAchievements Implementation [link to branch](https://github.com/PanMenel/melonDS-Menel-Forks/tree/Retro-Achievements-Implementation)
*Bringing RetroAchievements (RA) support to melonDS.*

I saw that the community really wanted RA support, so I decided to take on the challenge. This is a rough, beta implementation that I’ve tested extensively over the last week. I am a huge fan of RetroAchievements and very proud of this progress.

**Important:** This is currently unofficial, so Hardcore mode does not yet provide official Hardcore points.

### ✅ Current Features
* Full backend integration.
* Core frontend implementation.
* Basic connectivity and achievement unlocking.
* Achievements and their info is shown on getting them.
* Basic Login in Config under new tab.
* Multi instance separate logins
* Leaderboards implemented but not submitted yet.
* Fully functional Overlay that shows achievements and their progress
* Progress Indicators and Challenge Indicators fully implemented
* Info on losing an Internet connection and restoring it are implemented
* Fully working Hardcore blocks that stop you from cheating.(may have overlooked some more advanced but main easy access cheats are blocked)

### 🛠️ Known Issues & Planned Features
<ul>
  <li><b>Performance Concerns?, maybe fixed but needs more testing.</li>
  <li><b>Badly Written AI code (Will be improved massively).</li>
  <li><b>Hardcore Approval to get rid of that achievment and allow users to earn hardcore achievements.</li>
</ul>

> [!NOTE]
> **Full Disclosure:** This feature also was developed using "vibecoding" (this time mainly ChatGpt and Gemini). I really put my heart into that implementation, and I hope people will actually think of it as nice thing, so I don't consider my time as wasted.

---

## How to build
See [BUILD.md](./BUILD.md) for build instructions. this works for menubar fork
> [!NOTE]
> **For RA Implementation** you need to install additional dependencies. Also building may not fully work, but feedback is appreciated

* Dependencies
* Curl : pacman -S <prefix>-{toolchain,cmake,SDL2,libarchive,enet,zstd,faad2,curl}
---
* here are intructions for building your own RA build
* git clone -b Retro-Achievements-Implementation https://github.com/PanMenel/melonDS-Menel-Forks.git
* cd melonDS-Menel-Forks
* for static use:  cmake -B build -G Ninja -DENABLE_RETROACHIEVEMENTS=ON -DBUILD_STATIC=ON -DUSE_QT6=OFF -DCMAKE_PREFIX_PATH=$MSYSTEM_PREFIX/qt5-static     /      for dynamic use: cmake -B build -DENABLE_RETROACHIEVEMENTS=ON
* cmake --build build


## 💜 Support My Work

### 🎮 Twitch
Catch me live on **[My Twitch Channel](https://www.twitch.tv/panmenelg)**! I stream my development process and some gameplay. Feel free to drop by, and say hi!

### ☕ Donate
If you find my forks useful and want to support further development, you can buy me a coffee here:
**[There is no better way to thank me for my work ;D](https://ko-fi.com/panmenel)**

Your support helps me keep learning and adding more features!

## How to use

Firmware boot (not direct boot) requires a BIOS/firmware dump from an original DS or DS Lite.
DS firmwares dumped from a DSi or 3DS aren't bootable and only contain configuration data, thus they are only suitable when booting games directly.

### Possible firmware sizes

 * 128KB: DSi/3DS DS-mode firmware (reduced size due to lacking bootcode)
 * 256KB: regular DS firmware
 * 512KB: iQue DS firmware

DS BIOS dumps from a DSi or 3DS can be used with no compatibility issues. DSi BIOS dumps (in DSi mode) are not compatible. Or maybe they are. I don't know.

As for the rest, the interface should be pretty straightforward. If you have a question, don't hesitate to ask, though!

## TODO LIST

 * better DSi emulation
 * better OpenGL rendering
 * netplay
 * the impossible quest of pixel-perfect 3D graphics
 * support for rendering screens to separate windows
 * emulating some fancy addons
 * other non-core shit (debugger, graphics viewers, etc)

### TODO LIST FOR LATER (low priority)

 * big-endian compatibility (Wii, etc)
 * LCD refresh time (used by some games for blending effects)
 * any feature you can eventually ask for that isn't outright stupid

## Credits

 * Martin for GBAtek, a good piece of documentation
 * Cydrak for the extra 3D GPU research
 * limittox for the icon
 * All of you comrades who have been testing melonDS, reporting issues, suggesting shit, etc


## Privacy policy

This emulator does not collect, store, or transmit any personal data on its own servers.

If the RetroAchievements feature is enabled, the emulator communicates directly with the RetroAchievements service.
This includes authentication, achievement unlocks, and related gameplay data.
All such data is processed according to the RetroAchievements privacy policy.

No additional analytics, telemetry, or tracking is performed by this emulator.

All configuration data is stored locally on the user's device and can be removed at any time by deleting the configuration files.

## Licenses

[![GNU GPLv3 Image](https://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl-3.0.en.html)

melonDS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

### External
* Images used in the Input Config Dialog - see `src/frontend/qt_sdl/InputConfig/resources/LICENSE.md`
