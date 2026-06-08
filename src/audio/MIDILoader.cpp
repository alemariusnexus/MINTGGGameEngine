#include "MIDILoader.h"

#include "util/Log.h"

extern "C" {
#include "../3rdparty/eMIDI/src/helpers.h"
#include "../3rdparty/eMIDI/src/midifile.h"
}


LOG_USE_TAG("MIDILoader")


namespace MINTGGGameEngine
{


// Generated with the following command:
//
//      for m in $(seq 0 127); do freq=$(python -c "print(round((440 * 2**(($m-69)/12)) * 1000), end='')");
//          printf "%8u," "$freq"; if [ $((m%8)) -eq 7 ]; then echo; else echo -n ' '; fi; done
//
// Using the formula for 12-ET from: https://newt.phys.unsw.edu.au/jw/notes.html
// Units: mHz
static const uint32_t AudioFreqByMidiNoteNum[] = {
    8176,     8662,     9177,     9723,    10301,    10913,    11562,    12250,
   12978,    13750,    14568,    15434,    16352,    17324,    18354,    19445,
   20602,    21827,    23125,    24500,    25957,    27500,    29135,    30868,
   32703,    34648,    36708,    38891,    41203,    43654,    46249,    48999,
   51913,    55000,    58270,    61735,    65406,    69296,    73416,    77782,
   82407,    87307,    92499,    97999,   103826,   110000,   116541,   123471,
  130813,   138591,   146832,   155563,   164814,   174614,   184997,   195998,
  207652,   220000,   233082,   246942,   261626,   277183,   293665,   311127,
  329628,   349228,   369994,   391995,   415305,   440000,   466164,   493883,
  523251,   554365,   587330,   622254,   659255,   698456,   739989,   783991,
  830609,   880000,   932328,   987767,  1046502,  1108731,  1174659,  1244508,
 1318510,  1396913,  1479978,  1567982,  1661219,  1760000,  1864655,  1975533,
 2093005,  2217461,  2349318,  2489016,  2637020,  2793826,  2959955,  3135963,
 3322438,  3520000,  3729310,  3951066,  4186009,  4434922,  4698636,  4978032,
 5274041,  5587652,  5919911,  6271927,  6644875,  7040000,  7458620,  7902133,
 8372018,  8869844,  9397273,  9956063, 10548082, 11175303, 11839822, 12543854,
};



MIDILoader::MIDILoader()
    : loadSuccessful(false), transposeHalftones(0)
{
}

AudioClip MIDILoader::loadMIDIFile(const std::string_view& path)
{
    loadSuccessful = false;

    MidiFile midi;
    Error err = eMidi_open(&midi, path.data());
    if (err != EMIDI_OK) {
        setError(eMidi_errorToStr(err));
        return {};
    }

    AudioClip clip;

    // Default MIDI tempo is 120 quarter notes per minute
    uint32_t uspqn = 60000000 / 120; // us per quarter note
    uint32_t timeOffsetUs = 0;

    bool haveLastMidiNote = false;
    uint8_t lastMidiNote = 0;
    uint32_t lastMidiNoteStartTime = 0;
    unsigned int ignoreNoteOffCount = 0;

    clip.setTempo(60000.0f); // ms per minute

    MidiEvent evt;
    do {
        err = eMidi_readEvent(&midi, &evt);
        if (err != EMIDI_OK) {
            setError(eMidi_errorToStr(err));
            return {};
        }

        timeOffsetUs += (evt.deltaTime * uspqn) / midi.header.division.tpqn.TPQN;

        uint8_t evtType = evt.eventId & 0xF0;
        if (evtType == MIDI_EVENT_NOTE_ON) {
                if (haveLastMidiNote) {
                    // Note NoteOn before NoteOff. Since we can only play one tone at
                    // a time, we'll implicitly turn the previous note off now, and
                    // ignore the next explicit NoteOff
                    ignoreNoteOffCount++;

                    clip.noteAt (
                        midiNoteToFrequency(lastMidiNote),
                        static_cast<uint16_t>(midiTimeToClipTime(timeOffsetUs - lastMidiNoteStartTime)),
                        midiTimeToClipTime(lastMidiNoteStartTime)
                        );
                }
                haveLastMidiNote = true;
                lastMidiNote = evt.params.msg.noteOn.note;
                lastMidiNoteStartTime = timeOffsetUs;
        } else if (evtType == MIDI_EVENT_NOTE_OFF) {
            if (ignoreNoteOffCount != 0) {
                ignoreNoteOffCount--;
            } else {
                clip.noteAt (
                        midiNoteToFrequency(lastMidiNote),
                        static_cast<uint16_t>(midiTimeToClipTime(timeOffsetUs - lastMidiNoteStartTime)),
                        midiTimeToClipTime(lastMidiNoteStartTime)
                        );
                haveLastMidiNote = false;
            }
        } else if (evtType == MIDI_EVENT_META) {
            if (evt.metaEventId == MIDI_SET_TEMPO) {
                uspqn = evt.params.msg.meta.setTempo.usPerQuarterNote;
            }
        }
    } while (!(evt.eventId == MIDI_EVENT_META  &&  evt.metaEventId == MIDI_END_OF_TRACK));

    err = eMidi_close(&midi);
    if (err != EMIDI_OK) {
        setError(eMidi_errorToStr(err));
        return {};
    }

    loadSuccessful = true;
    errmsg = {};
    return clip;
}

void MIDILoader::setError(const std::string_view &errmsg)
{
    loadSuccessful = false;
    this->errmsg = errmsg;
}

uint16_t MIDILoader::midiNoteToFrequency(uint8_t note) const
{
    note += transposeHalftones;
    return note < sizeof(AudioFreqByMidiNoteNum)/sizeof(AudioFreqByMidiNoteNum[0])
            ? (AudioFreqByMidiNoteNum[note] + 500) / 1000 : 0;
}

uint32_t MIDILoader::midiTimeToClipTime(uint32_t time) const
{
    return time / 1000; // us -> ms
}


}
