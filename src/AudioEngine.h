#pragma once

#include "Globals.h"
#include "AudioClip.h"

#include <set>


namespace MINTGGGameEngine
{

/**
 * \brief The audio portion of the game engine.
 *
 * This engine works on objects of the AudioClip class. One or more AudioClips
 * can be active, but at most one may actually play at any given time. If
 * multiple clips are active at the same time, only the one with the highest
 * priority will be actively playing.
 *
 * This engine only supports playing a single note at a time (i.e. no harmony).
 * It is optimized for a simple piezo speaker as an output.
 *
 * Each AudioClip can either be played in single-shot mode (e.g. for sound
 * effects), or in endless loop mode (e.g. for background music). Clips can
 * also be stopped at any time.
 *
 * For details of how clips are handled, see the AudioClip class.
 *
 * \see AudioClip
 */
class AudioEngine
{
    friend void _AudioEngineTaskMain(void* params);
    
public:
    /**
     * \brief Determines which clip is actually played when multiple are active.
     *
     * Lower numbers mean higher priority. Only the clip with the highest
     * priority is played at any given point in time.
     */
    enum class Priority : uint16_t
    {
        Effect = 1000,      ///< For effects that mute background music while playing.
        Background = 2000   ///< For background music, muted by effects when they occur.
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
    
    /**
     * \brief Start the audio engine, playing a PWM output at the given pin.
     *
     * The device behind the pin is currently assumed to be a piezo speaker.
     *
     * \param speakerPin The audio output pin.
     * \return true if successful, false otherwise
     */
    bool begin(uint8_t speakerPin);
    
    /**
     * \brief Play the given audio clip.
     *
     * \param clip The clip to play.
     * \param prio The playback priority.
     * \param loop true to keep restarting the clip whenever it ends, until it
     *      is explicitly stopped; false otherwise
     * \param advanceInBackground true if playback time should advance for this
     *      clip even if it is not actually playing (due to other clips with
     *      higher priority). If false, the clip is paused while higher-priority
     *      clips are playing. This is usually true for background music, but
     *      false for most other clips (which results in "queueing" of clips).
     */
    void playClip(const AudioClip& clip, Priority prio = Priority::Effect, bool loop = false, bool advanceInBackground = false);
    
    /**
     * \brief Stop playback of the given clip.
     */
    bool stopClip(const AudioClip& clip);
    
    /**
     * \brief Mute or unmute all sound.
     *
     * Playback will still resume normally when muted, but no signals will be
     * delivered to the speaker.
     */
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
