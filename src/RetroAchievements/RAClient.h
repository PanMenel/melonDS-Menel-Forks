#pragma once

#include <cstdint>
#include <string>
#include <rcheevos/include/rc_client.h>
#include <optional>
#include <functional>
#include <vector>
#include "../rcheevos/include/rc_runtime.h"


using AchievementUnlockedCallback =
    std::function<void(const char* title,
                       const char* desc,
                       const char* badge_url)>;
using MeasuredProgressCallback =
    std::function<void(unsigned id,
                       unsigned value,
                       unsigned target,
                       const char* text)>;

struct TrackedAchievement
{
    uint32_t id = 0;
    unsigned prev_value = 0;
};
namespace melonDS { class NDS; }
class RAContext {
public:
    friend uint32_t RC_CCONV RuntimePeek(uint32_t, uint32_t, void*);
    struct FullAchievement
    {
        uint32_t id = 0;
        std::string title;
        std::string description;
        uint32_t points = 0;
        bool unlocked = false;

        bool measured = false;
        int value = 0;
        int target = 0;
        std::string progressText;

        char badge_name[8] = {};
        float measured_percent = 0.0f;
        time_t unlock_time = 0;
        uint8_t state = 0;
        uint8_t category = 0;
        uint8_t bucket = 0;
        float rarity = 0.0f;
        float rarity_hardcore = 0.0f;
        uint8_t type = 0;
        const char* badge_url = nullptr;
        const char* badge_locked_url = nullptr;
        const rc_client_achievement_t** achievements = nullptr;
        uint32_t num_achievements;

        const char* label;
        uint32_t subset_id;
        uint8_t bucket_type;
    };


    const std::vector<FullAchievement>& GetAllAchievements() const {
        return allAchievements;
    }
    void ResetGameState();
    RAContext();
    ~RAContext();
    bool m_shuttingDown = false;
    bool m_IsPaused = false;
    bool CanPause(uint32_t* frames_remaining);
    void SetPaused(bool paused);
    std::function<void(bool success, const std::string& message)> onLoginResponse;
    bool pendingLoginResult = false;
    bool loginSuccess = false;
    std::string loginErrorMessage;
    bool isLoading = false;
    std::string m_lastHash;
    std::string pendingLoadError;
    bool pendingLoadFailed = false;
    void SetOnGameLoadedCallback(std::function<void()> cb);
    std::function<void()> onGameLoaded;
    const rc_client_game_t* GetCurrentGameInfo() const { return currentGameInfo; }
    bool IsGameLoaded() const { return gameLoaded; }
    void SetOnAchievementUnlocked(AchievementUnlockedCallback cb);
    void SetOnMeasuredProgress(MeasuredProgressCallback cb);
    void UpdateMeasuredAchievements();
    void LoginNow();
    void Enable();
    void Disable();
    bool IsEnabled() const { return m_enabled; }
    bool IsHardcoreEnabled() const { return m_hardcore; }
    using LoginCallback = std::function<void()>;
    void SetOnLogin(LoginCallback cb)
    {
        m_onLogin = std::move(cb);
    }
    void AttachNDS(melonDS::NDS* nds_);
    void SetPendingGameHash(const char* hash);
    void SetToken(const std::string& token) { m_token = token; }
    void SetLoggedIn(bool v);
    void Init(melonDS::NDS* nds);
    void Shutdown();
    void DoFrame();
    void LoginWithPassword(const char* username, const char* password, bool hardcore);
    void SetCredentials(const char* user, const char* password, bool hardcore);
    void LoadGame(const char* hash);
    void Reset();
    bool IsLoggedIn() const { return m_logged_in; }
    const std::string& GetUser() const { return m_user; }
    const std::string& GetToken() const { return m_token; }
    const std::string& GetDisplayName() const;
    melonDS::NDS* nds = nullptr;
    rc_client_t* client = nullptr;
    bool m_logged_in = false;
private:
    rc_runtime_t m_runtime;
    std::vector<TrackedAchievement> trackedAchievements;
    std::vector<FullAchievement> allAchievements;
    void SetDisplayName(const char* name);
    AchievementUnlockedCallback m_onAchievementUnlocked;
    MeasuredProgressCallback m_onMeasuredProgress;
    std::string m_displayName;
    const rc_client_game_t* currentGameInfo = nullptr;
    bool m_enabled = false;
    LoginCallback m_onLogin;
    std::optional<std::string> pendingGameHash;
    bool gameLoaded = false;
    std::string m_user;
    std::string m_token;
    std::string m_password;
    bool m_hardcore = false;
    static uint32_t ReadMemory(
        uint32_t address,
        uint8_t* buffer,
        uint32_t size,
        rc_client_t* client
    );
    static void ServerCall(
        const rc_api_request_t* request,
        rc_client_server_callback_t callback,
        void* userdata,
        rc_client_t* client
    );
};