#include "SoundManage.h"
#include <fmod.hpp>
#include <Windows.h>   // OutputDebugStringA

// Keep a volume in the 0..1 range
static float ClampVolume(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

SoundManage::SoundManage()
    : system(0), soundCount(0), musicChannel(0), extraDriverData(0),
      masterVolume(1.0f), sfxVolume(1.0f), musicVolume(0.8f), muted(false) {
    for (int i = 0; i < kMaxSounds; ++i) sounds[i] = 0;
}

SoundManage::~SoundManage() {
    Shutdown();
}

bool SoundManage::Initialize() {
    FMOD_RESULT result;
    result = FMOD::System_Create(&system);                          // Create the FMOD system object
    if (result != FMOD_OK) { system = 0; return false; }

    result = system->init(32, FMOD_INIT_NORMAL, extraDriverData);   // 32 channels, default settings
    if (result != FMOD_OK) { system->release(); system = 0; return false; }
    return true;
}

void SoundManage::Shutdown() {
    for (int i = 0; i < soundCount; ++i) {
        if (sounds[i]) sounds[i]->release();
        sounds[i] = 0;
    }
    soundCount = 0;
    musicChannel = 0;

    if (system) {
        system->close();
        system->release();
        system = 0;
    }
}

int SoundManage::FindSound(const std::string& name) const {
    for (int i = 0; i < soundCount; ++i) {
        if (soundNames[i] == name) return i;
    }
    return -1;
}

bool SoundManage::LoadSound(const std::string& name, const std::string& filePath, bool isLooping) {
    if (!system || soundCount >= kMaxSounds) return false;

    FMOD::Sound* sound = 0;
    FMOD_RESULT result;
    // File name, default settings, extra info (none), address of sound
    result = system->createSound(filePath.c_str(), FMOD_DEFAULT, 0, &sound);
    if (result != FMOD_OK) {
        OutputDebugStringA(("SoundManage: could not load " + filePath + "\n").c_str());
        return false;
    }
    result = sound->setMode(isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

    sounds[soundCount] = sound;
    soundNames[soundCount] = name;
    ++soundCount;
    return true;
}

void SoundManage::PlaySfx(const std::string& name, float volume) {
    if (!system || muted) return;
    const int i = FindSound(name);
    if (i < 0) return;

    FMOD::Channel* channel = 0;
    // Sound, channel group (none), start paused?, address of channel
    FMOD_RESULT result = system->playSound(sounds[i], 0, false, &channel);
    if (result == FMOD_OK && channel) {
        channel->setVolume(volume * masterVolume * sfxVolume);
    }
}

void SoundManage::PlayMusic(const std::string& name, float volume) {
    if (!system || muted) return;
    const int i = FindSound(name);
    if (i < 0) return;

    StopMusic();   // Only one track at a time

    FMOD::Channel* channel = 0;
    FMOD_RESULT result = system->playSound(sounds[i], 0, false, &channel);
    if (result == FMOD_OK && channel) {
        channel->setVolume(volume * masterVolume * musicVolume);
        musicChannel = channel;   // Keep the handle so we can stop / adjust it later
    }
}

void SoundManage::StopMusic() {
    if (musicChannel) {
        musicChannel->stop();
        musicChannel = 0;
    }
}

void SoundManage::PauseMusic(bool pause) {
    if (musicChannel) musicChannel->setPaused(pause);
}

void SoundManage::SetMasterVolume(float volume) {
    masterVolume = ClampVolume(volume);
    if (musicChannel) musicChannel->setVolume(masterVolume * musicVolume);
}

void SoundManage::SetSFXVolume(float volume) {
    sfxVolume = ClampVolume(volume);   // Applies to the next SFX played
}

void SoundManage::SetMusicVolume(float volume) {
    musicVolume = ClampVolume(volume);
    if (musicChannel) musicChannel->setVolume(masterVolume * musicVolume);
}

void SoundManage::ToggleMute() {
    SetMute(!muted);
}

void SoundManage::SetMute(bool mute) {
    muted = mute;
    if (musicChannel) musicChannel->setPaused(muted);   // SFX are gated in PlaySfx()
}

void SoundManage::Update() {
    if (system) system->update();   // FMOD needs this once per frame
}
