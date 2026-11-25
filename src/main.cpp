#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace {
    struct ButtonState {
        bool held = false;
        bool deferredRelease = false;
        int heldSteps = 0;
        int deferredAge = 0;
    };

    ButtonState g_state[2];

    void resetState() {
        g_state[0] = {};
        g_state[1] = {};
    }

    int window() {
        return static_cast<int>(Mod::get()->getSettingValue<int64_t>("buffer-frames"));
    }

    bool isCubeLike(PlayerObject* p) {
        return !(p->m_isShip || p->m_isBird || p->m_isDart ||
                 p->m_isRobot || p->m_isSpider || p->m_isSwing);
    }
}

class $modify(PIBGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (button != 1 || !Mod::get()->getSettingValue<bool>("enabled")) {
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }

        auto& s = g_state[isPlayer1 ? 0 : 1];
        auto* p = isPlayer1 ? m_player1 : m_player2;
        if (!p) return GJBaseGameLayer::handleButton(down, button, isPlayer1);

        if (down) {
            if (s.deferredRelease) {
                s.deferredRelease = false;
                GJBaseGameLayer::handleButton(false, 1, isPlayer1);
            }
            s.held = true;
            s.heldSteps = 0;
            return GJBaseGameLayer::handleButton(true, 1, isPlayer1);
        }

        s.held = false;
        bool defer = false;

        if (p->m_isDart) {
            defer = Mod::get()->getSettingValue<bool>("wave-buffer") && s.heldSteps < window();
        }
        else if (isCubeLike(p)) {
            defer = Mod::get()->getSettingValue<bool>("jump-buffer")
                && !p->m_isOnGround
                && p->m_yVelocity < 0;
        }

        if (defer) {
            s.deferredRelease = true;
            s.deferredAge = 0;
            return;
        }

        GJBaseGameLayer::handleButton(false, 1, isPlayer1);
    }

    void processCommands(float dt) {
        for (int i = 0; i < 2; i++) {
            auto& s = g_state[i];
            auto* p = i == 0 ? m_player1 : m_player2;

            if (s.held || s.deferredRelease) s.heldSteps++;
            if (!s.deferredRelease || !p) continue;

            s.deferredAge++;

            bool fire;
            if (p->m_isDart) {
                fire = s.heldSteps >= window();
            } else {
                fire = (p->m_isOnGround && s.deferredAge >= 1) || s.deferredAge >= window();
            }

            if (fire) {
                s.deferredRelease = false;
                GJBaseGameLayer::handleButton(false, 1, i == 0);
            }
        }

        GJBaseGameLayer::processCommands(dt);
    }
};

class $modify(PIBPlayLayer, PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        resetState();
    }
};
