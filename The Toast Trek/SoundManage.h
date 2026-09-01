#pragma once
#include <string>
#include <map>

// FMOD Core (C++ API) forward declarations - the real headers are only
// pulled into SoundManage.cpp.
namespace FMOD {
    class System;
    class Sound;
    class Channel;
}

// Thin wrapper over FMOD Core. Every call is safe to make even if FMOD
// failed to initialise or a sound file is missing - it just no-ops.
class SoundManage {
private:
    FMOD::System* system;
    std::map<std::string, FMOD::Sound*> sounds;
    std::map<std::string, FMOD::Channel*> channels;
    float masterVolume;
    float sfxVolume;
    float musicVolume;
    bool muted;

public:
    SoundManage();
    ~SoundManage();

    bool Initialize();
    void Shutdown();

    // Load a sound file (wav/mp3/ogg...). Returns false if FMOD is down or
    // the file can't be opened - callers can ignore the result.
    bool LoadSound(const std::string& name, const std::string& filePath, bool isLooping = false);

    // Named PlaySfx (not PlaySound) so it can't collide with the winmm
    // PlaySound macro from <Windows.h>.
    void PlaySfx(const std::string& name, float volume = 1.0f);

    void PlayMusic(const std::string& name, float volume = 1.0f);
    void StopMusic();
    void PauseMusic(bool pause);

    void SetMasterVolume(float volume);
    void SetSFXVolume(float volume);
    void SetMusicVolume(float volume);
    void ToggleMute();
    void SetMute(bool mute);

    float GetMasterVolume() const { return masterVolume; }
    float GetSFXVolume() const { return sfxVolume; }
    float GetMusicVolume() const { return musicVolume; }
    bool IsMuted() const { return muted; }

    // Call once per frame.
    void Update();
};
