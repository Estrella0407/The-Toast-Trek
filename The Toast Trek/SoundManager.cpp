#include "SoundManager.h"
#include "SaveGame.h"
#include <fmod.hpp>
#include <Windows.h>   // OutputDebugStringA

// Every sound the game uses, plus the saved volume settings and the
// looping background track. Loading a missing file is a safe no-op.
void SoundManager::LoadGameSounds() {
    LoadSound("click", "Assets/Sounds/click.wav");
    LoadSound("gameover", "Assets/Sounds/gameover.wav");
    LoadSound("levelcomplete", "Assets/Sounds/levelcomplete.wav");
    LoadSound("background", "Assets/Sounds/background.wav", true);
    LoadSound("battle", "Assets/Sounds/battle.wav", true);
    LoadSound("attack", "Assets/Sounds/attack.wav");   // Pochi's FIGHT swing
    LoadSound("hurt", "Assets/Sounds/hurt.ogg");       // Pochi takes damage

    save::Settings st = save::LoadSettings();
    SetMasterVolume(st.master);
    SetMusicVolume(st.music);
    SetSFXVolume(st.sfx);
    SetMute(st.muted);

    PlayMusic("background", 0.6f);
}

// Keep a volume in the 0..1 range
static float ClampVolume(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

SoundManager::SoundManager()
    : system(0), soundCount(0), musicChannel(0), extraDriverData(0),
      masterVolume(1.0f), sfxVolume(1.0f), musicVolume(0.8f), muted(false) {
    for (int i = 0; i < kMaxSounds; ++i) sounds[i] = 0;
}

SoundManager::~SoundManager() {
    Shutdown();
}

bool SoundManager::Initialize() {
    FMOD_RESULT result;
    result = FMOD::System_Create(&system);                          // Create the FMOD system object
    if (result != FMOD_OK) { system = 0; return false; }

    result = system->init(32, FMOD_INIT_NORMAL, extraDriverData);   // 32 channels, default settings
    if (result != FMOD_OK) { system->release(); system = 0; return false; }
    return true;
}

void SoundManager::Shutdown() {
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

int SoundManager::FindSound(const std::string& name) const {
    for (int i = 0; i < soundCount; ++i) {
        if (soundNames[i] == name) return i;
    }
    return -1;
}

bool SoundManager::LoadSound(const std::string& name, const std::string& filePath, bool isLooping) {
    if (!system || soundCount >= kMaxSounds) return false;

    FMOD::Sound* sound = 0;
    FMOD_RESULT result;
    // File name, default settings, extra info (none), address of sound
    result = system->createSound(filePath.c_str(), FMOD_DEFAULT, 0, &sound);
    if (result != FMOD_OK) {
        OutputDebugStringA(("SoundManager: could not load " + filePath + "\n").c_str());
        return false;
    }
    result = sound->setMode(isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

    sounds[soundCount] = sound;
    soundNames[soundCount] = name;
    ++soundCount;
    return true;
}

void SoundManager::PlaySfx(const std::string& name, float volume) {
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

void SoundManager::PlayMusic(const std::string& name, float volume) {
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

void SoundManager::StopMusic() {
    if (musicChannel) {
        musicChannel->stop();
        musicChannel = 0;
    }
}

void SoundManager::PauseMusic(bool pause) {
    if (musicChannel) musicChannel->setPaused(pause);
}

void SoundManager::SetMasterVolume(float volume) {
    masterVolume = ClampVolume(volume);
    if (musicChannel) musicChannel->setVolume(masterVolume * musicVolume);
}

void SoundManager::SetSFXVolume(float volume) {
    sfxVolume = ClampVolume(volume);   // Applies to the next SFX played
}

void SoundManager::SetMusicVolume(float volume) {
    musicVolume = ClampVolume(volume);
    if (musicChannel) musicChannel->setVolume(masterVolume * musicVolume);
}

void SoundManager::ToggleMute() {
    SetMute(!muted);
}

void SoundManager::SetMute(bool mute) {
    muted = mute;
    if (musicChannel) musicChannel->setPaused(muted);   // SFX are gated in PlaySfx()
}

void SoundManager::Update() {
    if (system) system->update();   // FMOD needs this once per frame
}
