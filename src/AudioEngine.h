#pragma once

#include "Globals.h"
#include "AudioClip.h"

#include <set>


namespace MINTGGGameEngine
{

class AudioEngine
{
    friend void _AudioEngineTaskMain(void* params);
    
public:
    enum class Priority : uint16_t
    {
        // Lower number means higher priority
        Effect = 1000,
        Background = 2000
    };
    
private:
    struct AudioState
    {
        AudioState(const AudioClip& clip, Priority priority, bool loop, bool advanceInBackground)
                : clip(clip), priority(priority), loop(loop), advanceInBackground(advanceInBackground), playTime(0.0f) {}
        
        bool operator==(const AudioState& other) const { return priority == other.priority; }
        bool operator!=(const AudioState& other) const { return priority != other.priority; }
        bool operator<(const AudioState& other) const { return priority < other.priority; }
        bool operator<=(const AudioState& other) const { return priority <= other.priority; }
        bool operator>(const AudioState& other) const { return priority > other.priority; }
        bool operator>=(const AudioState& other) const { return priority >= other.priority; }
        
        AudioClip clip;
        Priority priority;
        bool loop;
        bool advanceInBackground; // true to advance time even when a higher priority clip is playing
        float playTime; // In base units of the clip
    };
    
public:
    AudioEngine() : speakerPin(-1), curSpeakerFreq(0), mute(false) {}
    
    bool begin(uint8_t speakerPin);
    
    void playClip(const AudioClip& clip, Priority prio = Priority::Effect, bool loop = false, bool advanceInBackground = false);
    bool stopClip(const AudioClip& clip);
    
    void setMute(bool mute);
    bool isMute() const { return mute; }

private:
    bool tick(float deltaTime);
    
    bool advanceAudioState(std::set<AudioState>::iterator stateIt, float deltaTime, AudioClip::Atom** outCurAtom, float* outTimeWithinAtom);
    
    void setTone(uint16_t freq);
    
    void onAudioStateFinished(std::set<AudioState>::iterator stateIt);
    
    void audioTaskMain();

private:
    int16_t speakerPin;
    uint16_t curSpeakerFreq;
    bool mute;
    
    std::set<AudioState> states;
    
    TaskHandle_t audioTask;
};

}
