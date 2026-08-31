//#pragma once
//#include <d3d9.h>
//#include <d3dx9.h>
//#include <string>
//#include <vector>
//#include <map>
//
//// Forward declaration for FMOD
//struct FMOD_SYSTEM;
//struct FMOD_SOUND;
//struct FMOD_CHANNEL;
//
//class SoundManage {
//private:
//    FMOD_SYSTEM* system;
//    std::map<std::string, FMOD_SOUND*> sounds;
//    std::map<std::string, FMOD_CHANNEL*> channels;
//    float masterVolume;
//    float sfxVolume;
//    float musicVolume;
//    bool muted;
//
//public:
//    SoundManage();
//    ~SoundManage();
//
//    bool Initialize();
//    void Shutdown();
//
//    // Load a sound file (supports wav, mp3, ogg, etc.)
//    bool LoadSound(const std::string& name, const std::string& filePath, bool isLooping = false);
//
//    // Play a sound
//    void PlaySound(const std::string& name, float volume = 1.0f);
//
//    // Play background music
//    void PlayMusic(const std::string& name, float volume = 1.0f);
//    void StopMusic();
//    void PauseMusic(bool pause);
//
//    // Volume controls
//    void SetMasterVolume(float volume);
//    void SetSFXVolume(float volume);
//    void SetMusicVolume(float volume);
//    void ToggleMute();
//    void SetMute(bool mute);
//
//    float GetMasterVolume() const { return masterVolume; }
//    float GetSFXVolume() const { return sfxVolume; }
//    float GetMusicVolume() const { return musicVolume; }
//    bool IsMuted() const { return muted; }
//
//    // Update must be called every frame
//    void Update();
//};
