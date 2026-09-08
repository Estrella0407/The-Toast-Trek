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
    static const int kMaxSounds = 16;

    FMOD::System* system;
    FMOD::Sound* sounds[kMaxSounds];        // loaded sound data
    std::string  soundNames[kMaxSounds];    // name -> slot, parallel to sounds[]
    int soundCount;

    FMOD::Channel* musicChannel;            // keep a handle to looping-music channel
    void* extraDriverData;                  // extra FMOD init data - none, so 0 

    std::map<std::string, FMOD::Channel*> channels; // for sfx channel with pitch

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
    bool  IsMuted() const { return muted; }
    void Update();

};
