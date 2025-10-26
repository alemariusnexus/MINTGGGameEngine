#include "AudioEngine.h"


namespace MINTGGGameEngine
{

void _AudioEngineTaskMain(void* params)
{
    ((AudioEngine*) params)->audioTaskMain();
}


bool AudioEngine::begin(uint8_t speakerPin)
{
    this->speakerPin = speakerPin;
    
    BaseType_t res = xTaskCreate(&_AudioEngineTaskMain, "AudioTask", 4096,
            this, 1, &audioTask);
    if (res != pdPASS) {
        Serial.println("ERROR: Unable to create AudioTask.");
        return false;
    }
    
    return true;
}

void AudioEngine::playClip(const AudioClip& clip, Priority prio, bool loop, bool advanceInBackground)
{
    states.emplace(clip, prio, loop, advanceInBackground);
}

bool AudioEngine::stopClip(const AudioClip& clip)
{
    for (auto it = states.begin() ; it != states.end() ; it++) {
        if (it->clip == clip) {
            states.erase(it);
            return true;
        }
    }
    return false;
}

bool AudioEngine::tick(float deltaTime)
{
    auto stateIt = states.begin();
    
    while (stateIt != states.end()) {
        /*AudioState& astate = const_cast<AudioState&>(*stateIt);
        
        float clipDeltaTime = astate.clip.realTimeToBaseUnits(deltaTime);
        astate.playTime += clipDeltaTime;
        
        AudioClip::Atom* curAtom = nullptr;
        float timeWithinAtom = 0.0f;
        float newPlayTime = astate.clip.getPlaybackPosition(astate.playTime, &curAtom, &timeWithinAtom);
        
        if (!curAtom  ||  (newPlayTime != astate.playTime  &&  !astate.loop)) {
            // Clip finished
            auto finishedIt = stateIt;
            stateIt++;
            onAudioStateFinished(finishedIt);
            continue;
        }
        
        astate.newPlayTime = newPlayTime;*/
        
        AudioState& astate = const_cast<AudioState&>(*stateIt);
        
        AudioClip::Atom* curAtom = nullptr;
        float timeWithinAtom = 0.0f;
        auto curIt = stateIt;
        stateIt++;
        if (advanceAudioState(curIt, deltaTime, &curAtom, &timeWithinAtom)) {
            // Audio finished
            continue;
        }
        
        float releaseT = 1.0f - astate.clip.getNoteEndReleaseDuration();
        float atomT = timeWithinAtom / curAtom->duration;
        
        if (atomT > releaseT) {
            setTone(0);
        } else {
            setTone(curAtom->freq);
        }
        
        // Advance background clips
        while (stateIt != states.end()) {
            auto curIt = stateIt;
            stateIt++;
            if (curIt->advanceInBackground) {
                advanceAudioState(curIt, deltaTime, nullptr, nullptr);
            }
        }
        
        return true;
    }
    
    setTone(0);
    
    return false;
}

bool AudioEngine::advanceAudioState(std::set<AudioState>::iterator stateIt, float deltaTime, AudioClip::Atom** outCurAtom, float* outTimeWithinAtom)
{
    AudioState& astate = const_cast<AudioState&>(*stateIt);
    
    float clipDeltaTime = astate.clip.realTimeToBaseUnits(deltaTime);
    astate.playTime += clipDeltaTime;
    
    AudioClip::Atom* curAtom = nullptr;
    float timeWithinAtom = 0.0f;
    float newPlayTime = astate.clip.getPlaybackPosition(astate.playTime, &curAtom, &timeWithinAtom);
    
    if (outCurAtom) {
        *outCurAtom = curAtom;
    }
    if (outTimeWithinAtom) {
        *outTimeWithinAtom = timeWithinAtom;
    }
    
    if (!curAtom  ||  (newPlayTime != astate.playTime  &&  !astate.loop)) {
        // Clip finished
        onAudioStateFinished(stateIt);
        return true;
    }
    
    astate.playTime = newPlayTime;
    
    return false;
}

void AudioEngine::audioTaskMain()
{
    unsigned long lastTimeUs = 0;
    while (true) {
        unsigned long now = micros();
        float deltaTime = 0.0f;
        
        if (lastTimeUs != 0  &&  now > lastTimeUs) {
            deltaTime = (now-lastTimeUs) / 1e3f;
        }
        lastTimeUs = now;
        
        tick(deltaTime);
        
        vTaskDelay(1);
    }
}

void AudioEngine::onAudioStateFinished(std::set<AudioState>::iterator stateIt)
{
    states.erase(stateIt);
}

void AudioEngine::setTone(uint16_t freq)
{
    if (speakerPin < 0) {
        return;
    }
    if (curSpeakerFreq == freq) {
        return;
    }
    
    curSpeakerFreq = freq;
    if (freq == 0) {
        noTone(speakerPin);
        Serial.printf("noTone()\r\n");
    } else {
        tone(speakerPin, freq);
        Serial.printf("tone(%u)\r\n", freq);
    }
}

}
