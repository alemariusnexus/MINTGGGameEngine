#pragma once

namespace MINTGGGameEngine
{

/**

\page Audio


\section audio_overview Overview

The engine contains a simple \ref MINTGGGameEngine::AudioEngine "AudioEngine". It plays back
\ref MINTGGGameEngine::AudioClip "AudioClips" in real-time, supporting both background music and effect clips. The
audio engine is very basic, outputting only one frequency at a time. It is targeted towards simple piezo speakers as
output devices.


\section audio_clips Audio Clips

Each piece of audio played by the engine is represented by an \ref AudioClip. An AudioClip is mostly a list of atoms,
each of which is either a (musical) note or a pause. Each such atom consists of:

- A timestamp at which it starts
- A duration
- A frequency (which is 0 for pauses)

All time values are measured not in absolute units like milliseconds, but in so-called base units. The real time
duration of such a base unit is calculated from the AudioClip's tempo. This allows changing the tempo of a clip
on-the-fly without having to change every single note or pause duration (e.g. to speed-up or slow-down certain clips
in specific game situations).

Here's an example for manually creating and playing back a simple sound effect by specifying the individual notes and
pauses:

\code{.cpp}
    // On global level
    AudioClip deathClip;

    // Somewhere during game setup
    deathClip.setTempo(200); // Clip tempo: 200 base units per minute
    deathClip.note(NOTE_G4, 1); // Play note G4 for one base unit
    deathClip.note(NOTE_Ds4, 1); // Play note D sharp 4 for one base unit
    deathClip.note(NOTE_C4, 1); // Play note C4 for one base unit
    deathClip.pause(3); // Final pause for 3 base units (before background music resumes, if any)

    // When player is hit by enemy
    game.audio().playClip(deathClip);
\endcode

For each note or pause, only the duration and frequency has to be specified. The timestamp is calculated automatically
based on the duration of previous notes and pauses.

The tempo of 200 means that there will be 200 base units per minute. In other words: The notes in the clip above (which
each have a duration of one base unit) will be 60s/200 = 0.3s long, while the final pause will be three times as long,
since its duration is three base units. Halving the tempo would make all these notes and pauses last twice as long.

The note values above are defined in the \ref Note enum, which define the common frequency values for equal-temperament
tuning (used in many pianos), but any custom integer frequency values can be used instead.

The code above will play the clip exactly once whenever playClip() is called. For background music, you often want a
clip to loop endlessly, which can be done with the following:

\code{.cpp}
    // Assumes an AudioClip object called backgroundAudio was previously created.
    game.audio().playClip(backgroundAudio, AudioEngine::Priority::Background, true, true); // clip, priority, loop, advanceInBackground
\endcode

See the \ref AudioClip and \ref AudioEngine class for more details.


\section audio_midi Loading MIDI Files

Instead of manually specifying note and pause values, the engine supports loading simple MIDI files. Currently, only
type 0 MIDI files are supported, which contain only one single track. Loading a MIDI file is done through the
\ref MIDILoader class.

Here's an example for creating an AudioClip from a MIDI file:

\code{.cpp}
    MIDILoader midiLoader;
    AudioClip bgMusicClip = midiLoader.loadMIDIFile("/storage/background.mid");
    if (!midiLoader.isLoadSuccessful()) {
        LogError("Error loading MIDI file: %s", midiLoader.getErrorMessage().c_str());
    }
\endcode

An error could occur e.g. if the file is not a valid MIDI file, or if it's not a type 0 file.

Please note that MIDI files are **very different** from modern consumer audio file formats like WAV or MP3. WAV or MP3
files contain a digitally sampled version of a very specific audio recording, and encode that very specific audio
recording such that it can later be reproduced almost perfectly on any speaker of sufficient quality. This engine does
**not** support such files, or playing back sampled audio in general.

MIDI files instead contain a sequence of "instructions" for a musical instrument (or several, but this engine can only
handle files for a single instrument) on what notes it should play when for how long. Depending on which instrument
these instructions are given to, the resulting audio will sound very different. In the case of this engine, the
"instrument" used is usually a piezo buzzer, which has no chance of ever sounding like a guitar or a piano. That is one
reason why **converting a WAV or MP3 file to a MIDI file automatically is practically impossible**.

See \ref MIDILoader for more details of what can be done with MIDI files.

*/

}
