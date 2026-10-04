# Code Review: RetroAchievements Integration

This review focuses on the current RetroAchievements integration against the upstream `melonDS` codebase. The evaluation centers on architecture, code quality, encapsulation, C++/Qt conventions, threading, build integration, and native feel.

## 1. Architecture & Encapsulation

**Violation of Encapsulation in `ReadMemory`:**
`RAClient.cpp` directly accesses raw inner workings of `melonDS::NDS`:
```cpp
memcpy(buffer, ctx->nds->MainRAM + address, count);
memcpy(buffer, ctx->nds->GPU.VRAM_A, 128*1024);
```
**Problem:** This severely breaks encapsulation and assumes memory layout won't change. It requires `RAClient` to know the exact physical memory structure of the console and bypasses MMU hooks.
**Recommendation:** Add a `ReadPhysicalMemory` or `ReadMemoryRA` interface to `melonDS::NDS` (or memory subsystem) and provide a clean callback to `RAContext`. The RA logic should not manually offset into `VRAM_A` or `ARM7WRAM`.

**Tight Coupling in NDSCart:**
In `src/NDSCart.cpp`, `CartCommon` directly includes `RAClient.h` and generates a hash for RA immediately.
```cpp
if (ra) { ra->SetPendingGameHash(this->ra_hash); }
```
**Problem:** `CartCommon` should not be responsible for RA context tracking. It creates unwanted coupling.
**Recommendation:** Compute the hash when requested by the emulator frontend/core or trigger an event `OnCartLoaded(hash)` that the frontend catches to notify `RAContext`.

## 2. Ownership & Lifetime

**RAContext Lifetime (`EmuInstance.h/cpp`):**
```cpp
std::unique_ptr<RAContext> ra;
```
`EmuInstance` owns `RAContext`, but raw pointers are leaked heavily (e.g., `nds->SetRAContext(ra.get())` and frontend components).
**Problem:** `RAContext` might outlive the objects it holds pointers to, such as `nds`, or callback lambdas attached in `RASettingsDialog.cpp` could execute after UI destruction.
**Recommendation:** Use `std::weak_ptr` or carefully manage the nullification of pointers in destructors. Disconnect RA callbacks when `RASettingsDialog` is closed or destroyed to prevent crashes if a network response comes back late.

## 3. C++/Qt Conventions & Frontend Integration

**Raw Network Requests in QGraphicsView (`RAOverlayWidget.cpp`):**
```cpp
netManager = new QNetworkAccessManager(this);
// ...
QNetworkReply* reply = netManager->get(QNetworkRequest(QUrl(qurl)));
```
**Problem:** `RAOverlayWidget` manually performs raw HTTP requests to fetch badge images inside `LoadHeaderImage`. This lacks error checking for timeouts, blocks UI heavily if misconfigured, and caches them manually in a `QHash`.
**Recommendation:** Move network fetching to a unified, asynchronous Image Manager or rely on RA's existing mechanisms if possible. Ensure proper cancellation of network replies when the widget closes.

**Manual Parent Null-Checks & Memory Management:**
In `RAOverlayWidget.cpp`, raw `QPushButton` and layouts are allocated without a parent widget, then manually passed around. Always supply `parent` to `new QWidget(parent)` when possible to rely on Qt's object tree.

**UI Responsiveness (Thread safety):**
`RAContext::ServerCall` offloads work to a custom detached thread queue (`EnsureHTTPThread`).
**Problem:** The custom thread queue (`std::thread`, `std::mutex`, `std::queue`) in `RAClient.cpp` works but bypasses Qt's event loop and `QNetworkAccessManager`, splitting the HTTP stack between core (CURL) and Qt UI (QNetworkAccessManager).
**Recommendation:** If using Qt, unify HTTP fetching using `QNetworkAccessManager` asynchronously, or if `core` must be headless, `core` correctly uses CURL but UI should avoid duplicating HTTP stacks.

## 4. Build System Integration & CMake

**Hardcoded inclusion of C files:**
```cpp
#include "RetroAchievements/cacert.c"
```
**Problem:** Including a `.c` file directly into a `.cpp` file is an anti-pattern.
**Recommendation:** Add `cacert.c` to `CMakeLists.txt` `target_sources(core PRIVATE ...)` and include its header instead. If no header is provided by the external project, declare `extern const unsigned char _accacert[];` in `RAClient.cpp` (which is already done, so the `#include` is even more redundant if compiled).

**Build flags & Conditionals:**
In `CMakeLists.txt`, `ENABLE_RETROACHIEVEMENTS` is present but the UI code doesn't guard its `RAContext` `#include` consistently.

## 5. Naming and Native Feel

- **Naming Convention:** `RAContext` uses mixed C++ naming conventions (`m_logged_in`, `m_IsPaused`, `gameLoaded`, `ShowProgressIndicator`). Follow melonDS conventions (usually PascalCase for methods, camelCase for variables).
- **Namespaces:** `RAContext` and `RAOverlayWidget` are in the global namespace. They should belong to `melonDS` or `melonDS::UI` to match standard practices in this project.

## Summary of Action Items

1. **Refactor Memory Access:** Move VRAM/RAM logic back to `NDS.cpp` and expose a `ReadMemoryRA` function.
2. **Decouple ROM Hash:** Move hashing out of `CartCommon` and handle it externally or via an interface.
3. **Fix Build Anti-Patterns:** Remove `#include "cacert.c"` and compile it natively via CMake.
4. **Cleanup Qt Lifecycle:** Ensure `QNetworkReply` instances are handled properly and lambdas checking `safeItem` handle destruction flawlessly. Put RA components in proper namespaces.
