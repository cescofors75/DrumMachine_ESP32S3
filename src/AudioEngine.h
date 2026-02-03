/*
 * AudioEngine.h
 * Motor d'àudio per ESP32-S3 Drum Machine
 * Gestiona I2S, samples i mixing de múltiples veus
 */

#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <cmath>

#define MAX_VOICES 8
#define SAMPLE_RATE 44100
#define DMA_BUF_COUNT 4
#define DMA_BUF_LEN 128

// Constants for filter management
static constexpr int MAX_AUDIO_TRACKS = 8;  // For per-track filters
static constexpr int MAX_PADS = 8;           // For per-pad filters



// Filter types (10 classic types)
enum FilterType {
  FILTER_NONE = 0,
  FILTER_LOWPASS = 1,      // Low Pass Filter
  FILTER_HIGHPASS = 2,     // High Pass Filter
  FILTER_BANDPASS = 3,     // Band Pass Filter
  FILTER_NOTCH = 4,        // Notch/Band Reject
  FILTER_ALLPASS = 5,      // All Pass (phase shift)
  FILTER_PEAKING = 6,      // Peaking EQ
  FILTER_LOWSHELF = 7,     // Low Shelf EQ
  FILTER_HIGHSHELF = 8,    // High Shelf EQ
  FILTER_RESONANT = 9      // Resonant Filter (high Q)
};

// Filter preset structure
struct FilterPreset {
  FilterType type;
  float cutoff;       // Hz
  float resonance;    // Q factor
  float gain;         // dB (for EQ filters)
  const char* name;
};

// Biquad filter coefficients
struct BiquadCoeffs {
  float b0, b1, b2;  // Numerator coefficients
  float a1, a2;      // Denominator coefficients (a0 normalized to 1)
};

// Filter state (for stereo)
struct FilterState {
  float x1, x2;  // Input history
  float y1, y2;  // Output history
};

// FX parameters
struct FXParams {
  FilterType filterType;
  float cutoff;          // Hz
  float resonance;       // Q factor
  float gain;            // dB (for EQ filters)
  uint8_t bitDepth;      // 4-16 bits
  float distortion;      // 0-100
  uint32_t sampleRate;   // Hz (for decimation)
  
  BiquadCoeffs coeffs;
  FilterState state;
  
  // Sample rate reducer state
  int32_t srHold;
  uint32_t srCounter;
};

// Voice structure
struct Voice {
  int16_t* buffer;        // Pointer to sample data in PSRAM
  uint32_t position;      // Current playback position
  uint32_t length;        // Sample length in samples
  bool active;            // Is voice playing?
  uint8_t velocity;       // MIDI velocity (0-127)
  uint8_t volume;         // Volume scale (0-100)
  float pitchShift;       // Pitch shift multiplier
  bool loop;              // Loop sample?
  uint32_t loopStart;     // Loop start point
  uint32_t loopEnd;       // Loop end point
  int padIndex;           // Which pad is playing (-1 if none)
  bool isLivePad;         // True if triggered from live pad, false if from sequencer
};

class AudioEngine {
public:
  AudioEngine();
  ~AudioEngine();
  
  // Initialization
  bool begin(int bckPin, int wsPin, int dataPin);
  
  // Sample management
  bool setSampleBuffer(int padIndex, int16_t* buffer, uint32_t length);
  
  // Playback control
  void triggerSample(int padIndex, uint8_t velocity);
  void triggerSampleSequencer(int padIndex, uint8_t velocity, uint8_t trackVolume = 100);
  void triggerSampleLive(int padIndex, uint8_t velocity);
  void stopSample(int padIndex);
  void stopAll();
  
  // Voice parameters
  void setPitch(int voiceIndex, float pitch);
  void setLoop(int voiceIndex, bool loop, uint32_t start = 0, uint32_t end = 0);
  
  // FX Control (Global)
  void setFilterType(FilterType type);
  void setFilterCutoff(float cutoff);
  void setFilterResonance(float resonance);
  void setBitDepth(uint8_t bits);
  void setDistortion(float amount);
  void setSampleRateReduction(uint32_t rate);
  
  // Per-Track Filter Management
  bool setTrackFilter(int track, FilterType type, float cutoff = 1000.0f, float resonance = 1.0f, float gain = 0.0f);
  void clearTrackFilter(int track);
  FilterType getTrackFilter(int track);
  int getActiveTrackFiltersCount();
  
  // Per-Pad (Live) Filter Management
  bool setPadFilter(int pad, FilterType type, float cutoff = 1000.0f, float resonance = 1.0f, float gain = 0.0f);
  void clearPadFilter(int pad);
  FilterType getPadFilter(int pad);
  int getActivePadFiltersCount();
  
  // Filter Presets (10 classic types)
  static const FilterPreset* getFilterPreset(FilterType type);
  static const char* getFilterName(FilterType type);
  
  // Volume Control
  void setMasterVolume(uint8_t volume); // 0-150
  uint8_t getMasterVolume();
  void setSequencerVolume(uint8_t volume); // 0-150
  uint8_t getSequencerVolume();
  void setLiveVolume(uint8_t volume); // 0-150
  uint8_t getLiveVolume();
  
  // Processing
  void process();
  
  // Statistics
  int getActiveVoices();
  float getCpuLoad();
  
  // Audio data capture for visualization
  void captureAudioData(uint8_t* spectrum, uint8_t* waveform);
  
private:
  Voice voices[MAX_VOICES];
  int16_t* sampleBuffers[16];  // Pointers to PSRAM sample data
  uint32_t sampleLengths[16];
  
  i2s_port_t i2sPort;
  int16_t mixBuffer[DMA_BUF_LEN * 2]; // Stereo buffer
  
  uint32_t processCount;
  uint32_t lastCpuCheck;
  float cpuLoad;
  
  FXParams fx;
  uint8_t masterVolume; // 0-100
  uint8_t sequencerVolume; // 0-100
  uint8_t liveVolume; // 0-100
  
  // Per-track and per-pad filters (max 8 active each)
  FXParams trackFilters[MAX_AUDIO_TRACKS];  // Filters for sequencer tracks
  bool trackFilterActive[MAX_AUDIO_TRACKS];
  FXParams padFilters[MAX_PADS];            // Filters for live pads
  bool padFilterActive[MAX_PADS];
  
  // Visualization buffers
  int16_t captureBuffer[256];
  uint8_t captureIndex;
  
  void fillBuffer(int16_t* buffer, size_t samples);
  int findFreeVoice();
  void resetVoice(int voiceIndex);
  
  // FX processing functions (optimized)
  void calculateBiquadCoeffs();
  void calculateBiquadCoeffs(FXParams& fx);  // Calculate for specific filter
  inline int16_t applyFilter(int16_t input);
  inline int16_t applyFilter(int16_t input, FXParams& fx);  // Apply specific filter
  inline int16_t applyBitCrush(int16_t input);
  inline int16_t applyDistortion(int16_t input);
  inline int16_t processFX(int16_t input);
};

#endif // AUDIOENGINE_H
