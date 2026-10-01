#include "AudioEngine.h"

#include "../util/Log.h"
#include "../util/Util.h"

#include <cmath>

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
#   include <QAudioSink>
#   include <QByteArray>
#   include <QCoreApplication>
#   include <QMediaDevices>
#   include <QThread>
#endif


LOG_USE_TAG("AudioEngine")


namespace MINTGGGameEngine
{


// TODO: Wow. Real-time audio using Qt sure sucks. The current code kinda sorta works, but there are noticeable
//  artifacts, and the whole code is less robust than I'd like. Consider using either the new Qt 6.11
//  callback-based API for QAudioSink, or using a completely different audio library altogether. Right now I'm too
//  lazy to do this.


#ifdef MINTGGGAMEENGINE_PORT_DESKTOP

ToneGeneratorDevice::ToneGeneratorDevice(const QAudioFormat& format)
    : format(format), frequency(0), phase(0), dataReadSinceLastTick(false)
{
}

qint64 ToneGeneratorDevice::bytesAvailable() const
{
    // Generate about 5ms of audio at a time
    return static_cast<qint64>(QIODevice::bytesAvailable()
        + (format.sampleRate() * calcSampleSize()) / 200);
}

void ToneGeneratorDevice::setFrequency(float frequency)
{
    this->frequency = frequency;
}

qint64 ToneGeneratorDevice::readData(char *data, qint64 maxlen)
{
    const auto sampleSize = static_cast<qint64>(calcSampleSize());
    const size_t numSamples = maxlen / sampleSize;
    if (!generatePCMData(data, numSamples)) {
        return -1;
    }
    QMetaObject::invokeMethod(this, [this] {
        emit readyRead();
    }, Qt::QueuedConnection);
    dataReadSinceLastTick = true;
    return static_cast<qint64>(numSamples*sampleSize);
}

qint64 ToneGeneratorDevice::writeData(const char *data, qint64 len)
{
    return -1;
}

bool ToneGeneratorDevice::generatePCMData(void* data, size_t numSamples)
{
    if (frequency == 0.0f) {
        memset(data, 0, numSamples*calcSampleSize());
        return true;
    }

    const auto sampleFormat = format.sampleFormat();
    const auto channelCount = format.channelCount();

    const float phaseDelta = frequency / static_cast<float>(format.sampleRate());
    const float amplitude = 1.0f / sqrtf(2.0f);

    if (sampleFormat == QAudioFormat::UInt8) {
        auto d = reinterpret_cast<uint8_t*>(data);
        while (numSamples != 0) {
            const auto sample = (phase < 0.5f) ? amplitude : -amplitude;
            const auto rawSample = static_cast<uint8_t>((sample+1)*0.5f*INT8_MAX);
            for (size_t j = 0 ; j < channelCount ; j++) {
                *d++ = rawSample;
            }
            phase += phaseDelta;
            if (phase >= 1.0f) {
                phase -= 1.0f;
            }
            numSamples--;
        }
        return true;
    } else if (sampleFormat == QAudioFormat::Int16) {
        auto d = reinterpret_cast<int16_t*>(data);
        while (numSamples != 0) {
            const auto sample = (phase < 0.5f) ? amplitude : -amplitude;
            const auto rawSample = static_cast<int16_t>(sample*INT16_MAX);
            for (size_t j = 0 ; j < channelCount ; j++) {
                *d++ = rawSample;
            }
            phase += phaseDelta;
            if (phase >= 1.0f) {
                phase -= 1.0f;
            }
            numSamples--;
        }
        return true;
    } else if (sampleFormat == QAudioFormat::Int32) {
        auto d = reinterpret_cast<int32_t*>(data);
        while (numSamples != 0) {
            const auto sample = (phase < 0.5f) ? amplitude : -amplitude;
            const auto rawSample = static_cast<int32_t>(sample*INT32_MAX);
            for (size_t j = 0 ; j < channelCount ; j++) {
                *d++ = rawSample;
            }
            phase += phaseDelta;
            if (phase >= 1.0f) {
                phase -= 1.0f;
            }
            numSamples--;
        }
        return true;
    } else if (sampleFormat == QAudioFormat::Float) {
        auto d = reinterpret_cast<float*>(data);
        while (numSamples != 0) {
            const auto sample = (phase < 0.5f) ? amplitude : -amplitude;
            for (size_t j = 0 ; j < channelCount ; j++) {
                *d++ = sample;
            }
            phase += phaseDelta;
            if (phase >= 1.0f) {
                phase -= 1.0f;
            }
            numSamples--;
        }
        return true;
    }

    return false;
}

size_t ToneGeneratorDevice::calcSampleSize() const
{
    switch (format.sampleFormat()) {
    case QAudioFormat::UInt8:   return format.channelCount() * sizeof(uint8_t);
    case QAudioFormat::Int16:   return format.channelCount() * sizeof(int16_t);
    case QAudioFormat::Int32:   return format.channelCount() * sizeof(int32_t);
    case QAudioFormat::Float:   return format.channelCount() * sizeof(float);
    default:                    return 0;
    }
}

#endif



AudioEngine::AudioEngine()
    : speakerPin(-1), curSpeakerFreq(0), mute(false), audioThread("AudioTask")
{
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    evtLoop = nullptr;
    audioSink = nullptr;
    toneGen = nullptr;
#endif
}

bool AudioEngine::begin(gpionum_t speakerPin)
{
    this->speakerPin = speakerPin;

#if defined(ESP_PLATFORM)  &&  !defined(ARDUINO)
    // Configure LEDC for the audio PWM signal

    if (speakerPin >= 0) {
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
    }
#elif defined(ARDUINO)
    // Nothing to do
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    QAudioDevice audioDev(QMediaDevices::defaultAudioOutput());
    audioFormat = audioDev.preferredFormat();

    const char* formatStr;
    switch (audioFormat.sampleFormat()) {
    case QAudioFormat::UInt8:       formatStr = "uint8";        break;
    case QAudioFormat::Int16:       formatStr = "int16";        break;
    case QAudioFormat::Int32:       formatStr = "int32";        break;
    case QAudioFormat::Float:       formatStr = "float";        break;
    default:                        formatStr = "[unknown]";    break;
    }
    LogInfo("Using audio device \"%s\": %d Hz, %s, %d channels", audioDev.description().toUtf8().constData(),
        audioFormat.sampleRate(), formatStr, audioFormat.channelCount());
#else
    LogWarning("Audio is currently not supported on this platform!");
#endif

    bool ok = audioThread.start([this] { audioTaskMain(); }, 4096, 1);
    if (!ok) {
        LogError("ERROR: Unable to create AudioTask.");
        return false;
    }
    
    return true;
}

void AudioEngine::shutdown()
{
    audioThread.stopAndWait();
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

        if (atomT < 0) {
            setTone(0);
        } else if (atomT > releaseT) {
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
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    evtLoop = new QEventLoop;

    toneGen = new ToneGeneratorDevice(audioFormat);
    toneGen->open(QIODevice::ReadOnly);

    audioSink = new QAudioSink(audioFormat);
    audioSink->moveToThread(QThread::currentThread());
    audioSink->setBufferSize(audioFormat.sampleRate() / 200);
    audioSink->start(toneGen);
#endif

    timer_ustick_t lastTimeUs = 0;
    while (!audioThread.isStopRequested()) {
#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
        evtLoop->processEvents();
#endif

        timer_ustick_t now = TimerGetTickcountUs();
        float deltaTime = 0.0f;
        
        if (lastTimeUs != 0  &&  now > lastTimeUs) {
            deltaTime = (now-lastTimeUs) / 1e3f;
        }
        lastTimeUs = now;
        
        tick(deltaTime);

        DelayTaskMs(1);
    }

#ifdef MINTGGGAMEENGINE_PORT_DESKTOP
    delete audioSink;
    delete toneGen;
    delete evtLoop;
#endif
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
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    ::tone(speakerPin, freq);
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
    ledc_set_freq(LEDC_LOW_SPEED_MODE, ledcTimer, freq);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledcChannel, 511);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ledcChannel);
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    audioSink->reset();
    toneGen->setFrequency(freq);
    audioSink->start(toneGen);
#else
    // TODO: Implement
#endif
}

void AudioEngine::noTone()
{
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    ::noTone(speakerPin);
#elif defined(MINTGGGAMEENGINE_PORT_ESPIDF)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledcChannel, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ledcChannel);
#elif defined(MINTGGGAMEENGINE_PORT_DESKTOP)
    audioSink->reset();
    toneGen->setFrequency(0);
#else
    // TODO: Implement
#endif
}

}
