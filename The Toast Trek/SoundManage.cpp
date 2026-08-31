//#include "SoundManage.h"
//#include <fmod.hpp>
//#include <Windows.h>
//#include <iostream>
//
//SoundManage::SoundManage()
//    : system(nullptr)
//    , masterVolume(1.0f)
//    , sfxVolume(1.0f)
//    , musicVolume(0.8f)
//    , muted(false) {
//}
//
//SoundManage::~SoundManage() {
//    Shutdown();
//}
//
//bool SoundManage::Initialize() {
//    if (FMOD::System_Create(&system) != FMOD_OK) {
//        MessageBoxA(NULL, "Failed to create FMOD system", "Sound Error", MB_OK | MB_ICONERROR);
//        return false;
//    }
//
//    if (system->init(32, FMOD_INIT_NORMAL, nullptr) != FMOD_OK) {
//        MessageBoxA(NULL, "Failed to initialize FMOD system", "Sound Error", MB_OK | MB_ICONERROR);
//        return false;
//    }
//
//    return true;
//}
//
//void SoundManage::Shutdown() {
//    // Release all sounds
//    for (auto& pair : sounds) {
//        if (pair.second) {
//            pair.second->release();
//        }
//    }
//    sounds.clear();
//    channels.clear();
//
//    if (system) {
//        system->close();
//        system->release();
//        system = nullptr;
//    }
//}
//
//bool SoundManage::LoadSound(const std::string& name, const std::string& filePath, bool isLooping) {
//    if (!system) return false;
//
//    FMOD_SOUND* sound = nullptr;
//    FMOD_MODE mode = FMOD_DEFAULT;
//
//    if (isLooping) {
//        mode |= FMOD_LOOP_NORMAL;
//    }
//
//    if (system->createSound(filePath.c_str(), mode, nullptr, &sound) != FMOD_OK) {
//        std::string msg = "Failed to load sound: " + filePath;
//        OutputDebugStringA(msg.c_str());
//        return false;
//    }
//
//    sounds[name] = sound;
//    return true;
//}
//
//void SoundManage::PlaySound(const std::string& name, float volume) {
//    if (muted) return;
//    if (sounds.find(name) == sounds.end()) return;
//
//    FMOD_SOUND* sound = sounds[name];
//    FMOD_CHANNEL* channel = nullptr;
//
//    if (system->playSound(sound, nullptr, false, &channel) == FMOD_OK) {
//        float finalVolume = volume * masterVolume * sfxVolume;
//        channel->setVolume(finalVolume);
//        channels[name] = channel;
//    }
//}
//
//void SoundManage::PlayMusic(const std::string& name, float volume) {
//    if (muted) return;
//    if (sounds.find(name) == sounds.end()) return;
//
//    // Stop current music
//    StopMusic();
//
//    FMOD_SOUND* sound = sounds[name];
//    FMOD_CHANNEL* channel = nullptr;
//
//    if (system->playSound(sound, nullptr, false, &channel) == FMOD_OK) {
//        float finalVolume = volume * masterVolume * musicVolume;
//        channel->setVolume(finalVolume);
//        channels["_music"] = channel;
//    }
//}
//
//void SoundManage::StopMusic() {
//    if (channels.find("_music") != channels.end()) {
//        channels["_music"]->stop();
//        channels.erase("_music");
//    }
//}
//
//void SoundManage::PauseMusic(bool pause) {
//    if (channels.find("_music") != channels.end()) {
//        channels["_music"]->setPaused(pause);
//    }
//}
//
//void SoundManage::SetMasterVolume(float volume) {
//    masterVolume = max(0.0f, min(1.0f, volume));
//    // Update all active channels
//    for (auto& pair : channels) {
//        if (pair.second) {
//            bool isMusic = (pair.first == "_music");
//            float baseVol = isMusic ? musicVolume : sfxVolume;
//            pair.second->setVolume(masterVolume * baseVol);
//        }
//    }
//}
//
//void SoundManage::SetSFXVolume(float volume) {
//    sfxVolume = max(0.0f, min(1.0f, volume));
//    for (auto& pair : channels) {
//        if (pair.second && pair.first != "_music") {
//            pair.second->setVolume(masterVolume * sfxVolume);
//        }
//    }
//}
//
//void SoundManage::SetMusicVolume(float volume) {
//    musicVolume = max(0.0f, min(1.0f, volume));
//    if (channels.find("_music") != channels.end()) {
//        channels["_music"]->setVolume(masterVolume * musicVolume);
//    }
//}
//
//void SoundManage::ToggleMute() {
//    SetMute(!muted);
//}
//
//void SoundManage::SetMute(bool mute) {
//    muted = mute;
//    if (muted) {
//        // Pause all channels
//        for (auto& pair : channels) {
//            if (pair.second) {
//                pair.second->setPaused(true);
//            }
//        }
//    }
//    else {
//        // Resume all channels
//        for (auto& pair : channels) {
//            if (pair.second) {
//                pair.second->setPaused(false);
//                // Restore volumes
//                bool isMusic = (pair.first == "_music");
//                float baseVol = isMusic ? musicVolume : sfxVolume;
//                pair.second->setVolume(masterVolume * baseVol);
//            }
//        }
//    }
//}
//
//void SoundManage::Update() {
//    if (system) {
//        system->update();
//    }
//}
//
