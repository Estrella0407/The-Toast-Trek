#pragma once
#include <string>

// FMOD Core forward declarations - keeps <fmod.hpp> out of every file that
// only needs to trigger a sound
namespace FMOD {
    class System;
    class Sound;
    class Channel;
}

// Thin wrapper over FMOD. Every call is safe even if FMOD failed to start
// or a sound file is missing - it just does nothing
class SoundManage {
private:
    static const int kMaxSounds = 16;

    FMOD::System* system;
    FMOD::Sound* sounds[kMaxSounds];        // Loaded sound data
    std::string  soundNames[kMaxSounds];    // Name -> slot, parallel to sounds[]
    int soundCount;

    FMOD::Channel* musicChannel;            // The looping-music channel we keep a handle to
    void* extraDriverData;                  // extra FMOD init data - none, so 0

    float masterVolume;
    float sfxVolume;
    float musicVolume;
    bool  muted;

    int FindSound(const std::string& name) const;   // Slot index, or -1

public:
    SoundManage();
    ~SoundManage();

    bool Initialize();
    void Shutdown();

    // Load a sound file under a name. false if FMOD is down / the file is missing
    bool LoadSound(const std::string& name, const std::string& filePath, bool isLooping = false);

    // Named PlaySfx (not PlaySound) so it can't clash with the PlaySound macro from <Windows.h>
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
    bool  IsMuted() const { return muted; }

    void Update();   // Call once per frame
};
