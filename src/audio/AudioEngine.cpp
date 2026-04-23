#include "AudioEngine.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../util/Log.h"
#include "../util/Util.h"


LOG_USE_TAG("AudioEngine")


namespace MINTGGGameEngine
{

void _AudioEngineTaskMain(void* params)
{
    ((AudioEngine*) params)->audioTaskMain();
}


bool AudioEngine::begin(uint8_t speakerPin)
{
    this->speakerPin = speakerPin;

#ifdef ESP_PLATFORM
    // Configure LEDC for the audio PWM signal

    ledcTimer = LEDC_TIMER_0;
    ledcChannel = LEDC_CHANNEL_0;

    ledc_timer_config_t timerCfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = ledcTimer,
        .freq_hz = 440,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timerCfg));

    ledc_channel_config_t channelCfg = {
        .gpio_num = speakerPin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = ledcChannel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = ledcTimer,
        .duty = 0,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = {}
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channelCfg));
#endif
    
    BaseType_t bres = xTaskCreate(&_AudioEngineTaskMain, "AudioTask", 4096,
            this, 1, &audioTask);
    if (bres != pdPASS) {
        LogError(TAG, "ERROR: Unable to create AudioTask.");
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

void AudioEngine::setMute(bool mute)
{
	if (mute == this->mute) {
		return;
	}
    this->mute = mute;
    curSpeakerFreq = 0;
}

bool AudioEngine::tick(float deltaTime)
{
    auto stateIt = states.begin();
    
    while (stateIt != states.end()) {
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
    timer_ustick_t lastTimeUs = 0;
    while (true) {
        timer_ustick_t now = TimerGetTickcountUs();
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
    if (mute) {
        this->noTone();
        return;
    }
    if (curSpeakerFreq == freq) {
        return;
    }
    
    curSpeakerFreq = freq;
    if (freq == 0) {
        this->noTone();
    } else {
        this->tone(freq);
    }
}

void AudioEngine::tone(uint16_t freq)
{
#ifdef ARDUINO
    ::tone(speakerPin, freq);
#elif defined(ESP_PLATFORM)
    ledc_set_freq(LEDC_LOW_SPEED_MODE, ledcTimer, freq);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledcChannel, 511);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ledcChannel);
#endif
}

void AudioEngine::noTone()
{
#ifdef ARDUINO
    ::noTone(speakerPin);
#elif defined(ESP_PLATFORM)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledcChannel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ledcChannel);
#endif
}

}
