#pragma once

#include "Sound.h"

class MidiPlayer
{
public:
    MidiPlayer();
    ~MidiPlayer();
    DWORD SetSound(const SoundComponent &sound, uint16_t wTempo);
    void Play();
    void Pause();
    void Stop();
    bool CanPlay();
    bool CanPause();
    bool CanStop();
    DWORD QueryPosition(DWORD scope);
    DWORD QueryPosition();
    void SetTempo(uint16_t wTempo) { _wTempo = wTempo; _SetTempoAndDivision(); }
    void SetDevice(DeviceType device) { _device = device; }
    void CueTickPosition(DWORD dwTicks);
    void CuePosition(DWORD dwTicks, DWORD scope);

private:
    bool _Init();
    void _SetTempoAndDivision();
    void _ClearHeaders();
    void static CALLBACK s_MidiOutProc(HMIDIOUT hmo, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    void _OnStreamDone();
    void _CuePosition(DWORD dwEventIndex);

    HMIDISTRM _handle;
    MIDIHDR _midiHdr;
    DWORD _cRemainingStreamEvents; // In case it didn't fit into a 64k chunk
    DWORD _cTotalStreamEvents;
    DWORD *_pRealData; // Full data for the 64k chunk in _midiHdr.
    std::vector<DWORD> _accumulatedStreamTicks;   // Corresponds to _midiHdr.lpData / 3
    DWORD _dwCurrentChunkTickStart;
    DWORD _dwCurrentTickPos;
    DeviceType _device;
    bool _fPlaying;
    bool _fQueuedUp;
    bool _fStoppingStream;
    DWORD _wTotalTime;
    uint16_t _wTempo;
    uint16_t _wTimeDivision;
    DWORD _dwLoopPoint;
    DWORD _dwCookie;
};

extern MidiPlayer g_midiPlayer;