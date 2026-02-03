/*
 * Sequencer.h
 * Sequencer de 16 steps per Drum Machine (8 tracks)
 * (OPCIONAL - per afegir funcionalitat de sequencer)
 */

#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <Arduino.h>

#define MAX_PATTERNS 16
#define STEPS_PER_PATTERN 16
#define MAX_TRACKS 8

class Sequencer {
public:
  Sequencer();
  ~Sequencer();
  
  // Control
  void start();
  void stop();
  void reset();
  bool isPlaying();
  
  // Timing
  void setTempo(float bpm);
  float getTempo();
  void update(); // Call from loop
  
  // Pattern editing
  void setStep(int track, int step, bool active, uint8_t velocity = 127);
  bool getStep(int track, int step);
  bool getStep(int pattern, int track, int step);  // Get step from specific pattern
  void clearPattern(int pattern);
  void clearPattern(); // Clear current pattern
  void clearTrack(int track);
  
  // Velocity editing per step
  void setStepVelocity(int track, int step, uint8_t velocity);
  void setStepVelocity(int pattern, int track, int step, uint8_t velocity);
  uint8_t getStepVelocity(int track, int step);
  uint8_t getStepVelocity(int pattern, int track, int step);
  
  // Pattern management
  void selectPattern(int pattern);
  int getCurrentPattern();
  void copyPattern(int src, int dst);
  
  // Mute tracks
  void muteTrack(int track, bool muted);
  bool isTrackMuted(int track);
  
  // Track volumes (0-100)
  void setTrackVolume(int track, uint8_t volume);
  uint8_t getTrackVolume(int track);
  
  // Playback
  int getCurrentStep();
  
  // Loop system for live pads
  void toggleLoop(int track);
  void pauseLoop(int track);
  bool isLooping(int track);
  bool isLoopPaused(int track);
  void processLoops(); // Called internally
  
  // Callbacks
  typedef void (*StepCallback)(int track, uint8_t velocity, uint8_t trackVolume);
  typedef void (*StepChangeCallback)(int newStep);
  void setStepCallback(StepCallback callback);
  void setStepChangeCallback(StepChangeCallback callback);
  
private:
  // Pattern data: [pattern][track][step]
  bool steps[MAX_PATTERNS][MAX_TRACKS][STEPS_PER_PATTERN];
  uint8_t velocities[MAX_PATTERNS][MAX_TRACKS][STEPS_PER_PATTERN];
  
  bool playing;
  int currentPattern;
  int currentStep;
  float tempo; // BPM
  uint32_t lastStepTime;
  uint32_t stepInterval; // microseconds
  bool trackMuted[MAX_TRACKS];
  uint8_t trackVolume[MAX_TRACKS]; // Volume per track (0-100)
  
  StepCallback stepCallback;
  StepChangeCallback stepChangeCallback;
  
  // Loop system
  bool loopActive[MAX_TRACKS];
  bool loopPaused[MAX_TRACKS];
  
  void calculateStepInterval();
  void processStep();
};

#endif // SEQUENCER_H
