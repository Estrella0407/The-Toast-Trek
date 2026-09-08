#pragma once
#include <string>
#include <map>

// pulled into SoundManage.cpp.
namespace FMOD {
    class System;
    class Sound;
    class Channel;
}

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

    // Load a sound file. Returns false if FMOD is down or the file can't be opened - callers can ignore the result.
    bool LoadSound(const std::string& name, const std::string& filePath, bool isLooping = false);

    void PlaySfx(const std::string& name, float volume = 1.0f, float pitch = 1.0f); 
	void PlayHitSfx(float volume = 1.0f, float pitch = 1.0f);


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

    void Update();
};
