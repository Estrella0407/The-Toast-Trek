#include "SoundManage.h"
#include <fmod.hpp>
#include <fmod_errors.h>
#include <Windows.h>

static float ClampVol(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

SoundManage::SoundManage()
    : system(nullptr)
    , masterVolume(1.0f)
    , sfxVolume(1.0f)
    , musicVolume(0.8f)
    , muted(false) {
}

SoundManage::~SoundManage() {
    Shutdown();
}

bool SoundManage::Initialize() {
    if (FMOD::System_Create(&system) != FMOD_OK) {
        system = nullptr;
        return false;
    }
    if (system->init(32, FMOD_INIT_NORMAL, nullptr) != FMOD_OK) {
        system->release();
        system = nullptr;
        return false;
    }
    return true;
}

void SoundManage::Shutdown() {
    for (auto& pair : sounds) {
        if (pair.second) pair.second->release();
    }
    sounds.clear();
    channels.clear();

    if (system) {
        system->close();
        system->release();
        system = nullptr;
    }
}

bool SoundManage::LoadSound(const std::string& name, const std::string& filePath, bool isLooping) {
    if (!system) return false;

    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = FMOD_DEFAULT;
    if (isLooping) mode |= FMOD_LOOP_NORMAL;

    if (system->createSound(filePath.c_str(), mode, nullptr, &sound) != FMOD_OK) {
        OutputDebugStringA(("SoundManage: could not load " + filePath + "\n").c_str());
        return false;
    }
    sounds[name] = sound;
    return true;
}

void SoundManage::PlaySfx(const std::string& name, float volume) {
    if (!system || muted) return;
    auto it = sounds.find(name);
    if (it == sounds.end()) return;

    FMOD::Channel* channel = nullptr;
    if (system->playSound(it->second, nullptr, false, &channel) == FMOD_OK && channel) {
        channel->setVolume(volume * masterVolume * sfxVolume);
        channels[name] = channel;
    }
}

void SoundManage::PlayMusic(const std::string& name, float volume) {
    if (!system || muted) return;
    auto it = sounds.find(name);
    if (it == sounds.end()) return;

    StopMusic();

    FMOD::Channel* channel = nullptr;
    if (system->playSound(it->second, nullptr, false, &channel) == FMOD_OK && channel) {
        channel->setVolume(volume * masterVolume * musicVolume);
        channels["_music"] = channel;
    }
}

void SoundManage::StopMusic() {
    auto it = channels.find("_music");
    if (it != channels.end()) {
        if (it->second) it->second->stop();
        channels.erase(it);
    }
}

void SoundManage::PauseMusic(bool pause) {
    auto it = channels.find("_music");
    if (it != channels.end() && it->second) it->second->setPaused(pause);
}

void SoundManage::SetMasterVolume(float volume) {
    masterVolume = ClampVol(volume);
    for (auto& pair : channels) {
        if (!pair.second) continue;
        const bool isMusic = (pair.first == "_music");
        pair.second->setVolume(masterVolume * (isMusic ? musicVolume : sfxVolume));
    }
}

void SoundManage::SetSFXVolume(float volume) {
    sfxVolume = ClampVol(volume);
    for (auto& pair : channels) {
        if (pair.second && pair.first != "_music")
            pair.second->setVolume(masterVolume * sfxVolume);
    }
}

void SoundManage::SetMusicVolume(float volume) {
    musicVolume = ClampVol(volume);
    auto it = channels.find("_music");
    if (it != channels.end() && it->second)
        it->second->setVolume(masterVolume * musicVolume);
}

void SoundManage::ToggleMute() {
    SetMute(!muted);
}

void SoundManage::SetMute(bool mute) {
    muted = mute;
    for (auto& pair : channels) {
        if (!pair.second) continue;
        if (muted) {
            pair.second->setPaused(true);
        }
        else {
            pair.second->setPaused(false);
            const bool isMusic = (pair.first == "_music");
            pair.second->setVolume(masterVolume * (isMusic ? musicVolume : sfxVolume));
        }
    }
}

void SoundManage::Update() {
    if (system) system->update();
}
