/*
 * Sequencer.cpp
 * Implementació del sequencer de 16 steps
 */

#include "Sequencer.h"

Sequencer::Sequencer() : 
  playing(false), 
  currentPattern(0), 
  currentStep(0), 
  tempo(120.0f),
  lastStepTime(0),
  stepCallback(nullptr),
  stepChangeCallback(nullptr) {
  
  // Initialize all patterns
  for (int p = 0; p < MAX_PATTERNS; p++) {
    for (int t = 0; t < MAX_TRACKS; t++) {
      for (int s = 0; s < STEPS_PER_PATTERN; s++) {
        steps[p][t][s] = false;
        velocities[p][t][s] = 127;
      }
      if (p == 0) {
        trackMuted[t] = false;
        loopActive[t] = false;
        loopPaused[t] = false;
      }
    }
  }
  
  // Patrón 0: Ritmo básico 808 con dinámicas variadas
  // BD (0): Kick en 1,5,9,13 - acentos en 1 y 9
  steps[0][0][0] = true; velocities[0][0][0] = 127;  // Acento fuerte
  steps[0][0][4] = true; velocities[0][0][4] = 100;  // Medio
  steps[0][0][8] = true; velocities[0][0][8] = 127;  // Acento fuerte
  steps[0][0][12] = true; velocities[0][0][12] = 90;  // Suave
  
  // SD (1): Snare en 4,12 - ghost notes en 6,14
  steps[0][1][4] = true; velocities[0][1][4] = 127;   // Acento fuerte
  steps[0][1][6] = true; velocities[0][1][6] = 60;    // Ghost note
  steps[0][1][12] = true; velocities[0][1][12] = 127; // Acento fuerte
  steps[0][1][14] = true; velocities[0][1][14] = 70;  // Ghost note suave
  
  // CH (2): Hi-hat 16ths con swing y acentos
  for (int s = 0; s < 16; s++) {
    steps[0][2][s] = true;
    if (s % 4 == 0) velocities[0][2][s] = 110;      // Tiempo fuerte
    else if (s % 2 == 0) velocities[0][2][s] = 85;  // Off-beat
    else velocities[0][2][s] = 65;                  // 16ths suaves
  }
  
  // OH (3): Open hat en off-beats asincopados
  steps[0][3][2] = true; velocities[0][3][2] = 90;
  steps[0][3][6] = true; velocities[0][3][6] = 100;
  steps[0][3][10] = true; velocities[0][3][10] = 80;
  steps[0][3][14] = true; velocities[0][3][14] = 95;
  
  // CP (4): Clap en 4,12 con doble tap
  steps[0][4][4] = true; velocities[0][4][4] = 110;
  steps[0][4][12] = true; velocities[0][4][12] = 110;
  
  // RS (5): Rimshot asincopado
  steps[0][5][3] = true; velocities[0][5][3] = 80;
  steps[0][5][7] = true; velocities[0][5][7] = 70;
  steps[0][5][11] = true; velocities[0][5][11] = 90;
  steps[0][5][15] = true; velocities[0][5][15] = 75;
  
  // Patrón 1: Ritmo Afro-Cuban / Tresillo
  // BD: Patrón 3-3-2
  steps[1][0][0] = true; velocities[1][0][0] = 127;
  steps[1][0][3] = true; velocities[1][0][3] = 100;
  steps[1][0][6] = true; velocities[1][0][6] = 120;
  steps[1][0][8] = true; velocities[1][0][8] = 110;
  steps[1][0][11] = true; velocities[1][0][11] = 95;
  steps[1][0][14] = true; velocities[1][0][14] = 115;
  
  // SD: Clave pattern
  steps[1][1][0] = true; velocities[1][1][0] = 110;
  steps[1][1][3] = true; velocities[1][1][3] = 100;
  steps[1][1][6] = true; velocities[1][1][6] = 90;
  steps[1][1][10] = true; velocities[1][1][10] = 105;
  steps[1][1][12] = true; velocities[1][1][12] = 95;
  
  // CH: Patrón sincopado
  for (int s = 0; s < 16; s++) {
    if (s % 3 == 0) {
      steps[1][2][s] = true;
      velocities[1][2][s] = (s % 6 == 0) ? 100 : 75;
    }
  }
  
  // Patrón 2: Breakbeat con shuffle
  // BD: Groovy pattern
  steps[2][0][0] = true; velocities[2][0][0] = 127;
  steps[2][0][2] = true; velocities[2][0][2] = 70;   // Ghost
  steps[2][0][5] = true; velocities[2][0][5] = 95;
  steps[2][0][7] = true; velocities[2][0][7] = 85;
  steps[2][0][10] = true; velocities[2][0][10] = 100;
  steps[2][0][13] = true; velocities[2][0][13] = 75;
  steps[2][0][15] = true; velocities[2][0][15] = 90;
  
  // SD: Backbeat con flams
  steps[2][1][3] = true; velocities[2][1][3] = 60;   // Flam ghost
  steps[2][1][4] = true; velocities[2][1][4] = 127;  // Backbeat
  steps[2][1][11] = true; velocities[2][1][11] = 65; // Flam ghost
  steps[2][1][12] = true; velocities[2][1][12] = 127;// Backbeat
  
  // CH: Shuffle pattern (swing)
  for (int s = 0; s < 16; s += 2) {
    steps[2][2][s] = true;
    velocities[2][2][s] = (s % 4 == 0) ? 100 : 70;
    if (s < 15) {
      steps[2][2][s+1] = true;
      velocities[2][2][s+1] = 50; // Shuffle note suave
    }
  }
  
  // OH: Acentos en off-beats
  steps[2][3][6] = true; velocities[2][3][6] = 95;
  steps[2][3][14] = true; velocities[2][3][14] = 100;
  
  calculateStepInterval();
}

Sequencer::~Sequencer() {
}

void Sequencer::start() {
  playing = true;
  lastStepTime = micros();
  Serial.println("Sequencer started");
}

void Sequencer::stop() {
  playing = false;
  Serial.println("Sequencer stopped");
}

void Sequencer::reset() {
  currentStep = 0;
  lastStepTime = micros();
}

bool Sequencer::isPlaying() {
  return playing;
}

void Sequencer::setTempo(float bpm) {
  if (bpm < 40.0f) bpm = 40.0f;
  if (bpm > 300.0f) bpm = 300.0f;
  
  tempo = bpm;
  calculateStepInterval();
  
  Serial.printf("Tempo set to %.1f BPM\n", tempo);
}

float Sequencer::getTempo() {
  return tempo;
}

void Sequencer::calculateStepInterval() {
  // 16th notes
  // 1 beat = 60/BPM seconds
  // 1 16th note = (60/BPM) / 4 seconds
  // Convert to microseconds
  stepInterval = (uint32_t)((60.0f / tempo / 4.0f) * 1000000.0f);
}

void Sequencer::update() {
  if (!playing) return;
  
  uint32_t now = micros();
  
  // Check if it's time for next step
  if (now - lastStepTime >= stepInterval) {
    lastStepTime = now;
    
    // PRIMERO: Notificar el step ACTUAL (antes de avanzar)
    // Esto sincroniza la visualización con el audio
    if (stepChangeCallback != nullptr) {
      stepChangeCallback(currentStep);
    }
    
    // SEGUNDO: Procesar el audio del step actual
    processStep();
    
    // TERCERO: Avanzar al siguiente step para la próxima iteración
    currentStep++;
    if (currentStep >= STEPS_PER_PATTERN) {
      currentStep = 0;
    }
  }
}

void Sequencer::processStep() {
  // First: Process looped tracks
  processLoops();
  
  // Trigger all active tracks at current step
  for (int track = 0; track < MAX_TRACKS; track++) {
    // Check sequencer steps
    if (steps[currentPattern][track][currentStep] && !trackMuted[track]) {
      uint8_t velocity = velocities[currentPattern][track][currentStep];
      
      // Call callback if set
      if (stepCallback != nullptr) {
        stepCallback(track, velocity);
      }
    }
  }
}

void Sequencer::setStep(int track, int step, bool active, uint8_t velocity) {
  if (track < 0 || track >= MAX_TRACKS) return;
  if (step < 0 || step >= STEPS_PER_PATTERN) return;
  
  steps[currentPattern][track][step] = active;
  velocities[currentPattern][track][step] = velocity;
}

bool Sequencer::getStep(int track, int step) {
  if (track < 0 || track >= MAX_TRACKS) return false;
  if (step < 0 || step >= STEPS_PER_PATTERN) return false;
  
  return steps[currentPattern][track][step];
}

bool Sequencer::getStep(int pattern, int track, int step) {
  if (pattern < 0 || pattern >= MAX_PATTERNS) return false;
  if (track < 0 || track >= MAX_TRACKS) return false;
  if (step < 0 || step >= STEPS_PER_PATTERN) return false;
  
  return steps[pattern][track][step];
}

void Sequencer::clearPattern(int pattern) {
  if (pattern < 0 || pattern >= MAX_PATTERNS) return;
  
  for (int t = 0; t < MAX_TRACKS; t++) {
    for (int s = 0; s < STEPS_PER_PATTERN; s++) {
      steps[pattern][t][s] = false;
      velocities[pattern][t][s] = 127;
    }
  }
  
  Serial.printf("Pattern %d cleared\n", pattern);
}

void Sequencer::clearPattern() {
  clearPattern(currentPattern);
}

void Sequencer::clearTrack(int track) {
  if (track < 0 || track >= MAX_TRACKS) return;
  
  for (int s = 0; s < STEPS_PER_PATTERN; s++) {
    steps[currentPattern][track][s] = false;
  }
  
  Serial.printf("Track %d cleared\n", track);
}

void Sequencer::selectPattern(int pattern) {
  if (pattern < 0 || pattern >= MAX_PATTERNS) return;
  
  currentPattern = pattern;
  Serial.printf("Pattern %d selected\n", pattern);
}

void Sequencer::muteTrack(int track, bool muted) {
  if (track >= 0 && track < MAX_TRACKS) {
    trackMuted[track] = muted;
    Serial.printf("Track %d %s\n", track, muted ? "MUTED" : "UNMUTED");
  }
}

bool Sequencer::isTrackMuted(int track) {
  if (track >= 0 && track < MAX_TRACKS) {
    return trackMuted[track];
  }
  return false;
}

int Sequencer::getCurrentPattern() {
  return currentPattern;
}

void Sequencer::copyPattern(int src, int dst) {
  if (src < 0 || src >= MAX_PATTERNS) return;
  if (dst < 0 || dst >= MAX_PATTERNS) return;
  
  for (int t = 0; t < MAX_TRACKS; t++) {
    for (int s = 0; s < STEPS_PER_PATTERN; s++) {
      steps[dst][t][s] = steps[src][t][s];
      velocities[dst][t][s] = velocities[src][t][s];
    }
  }
  
  Serial.printf("Pattern %d copied to %d\n", src, dst);
}

int Sequencer::getCurrentStep() {
  return currentStep;
}

void Sequencer::setStepCallback(StepCallback callback) {
  stepCallback = callback;
}

void Sequencer::setStepChangeCallback(StepChangeCallback callback) {
  stepChangeCallback = callback;
}

// ============= LOOP SYSTEM =============

void Sequencer::toggleLoop(int track) {
  if (track >= 0 && track < MAX_TRACKS) {
    loopActive[track] = !loopActive[track];
    loopPaused[track] = false; // Reset pause state
    Serial.printf("[Loop] Track %d: %s\n", track, loopActive[track] ? "ACTIVE" : "INACTIVE");
  }
}

void Sequencer::pauseLoop(int track) {
  if (track >= 0 && track < MAX_TRACKS) {
    if (loopActive[track]) {
      loopPaused[track] = !loopPaused[track];
      Serial.printf("[Loop] Track %d: %s\n", track, loopPaused[track] ? "PAUSED" : "RESUMED");
    }
  }
}

bool Sequencer::isLooping(int track) {
  if (track >= 0 && track < MAX_TRACKS) {
    return loopActive[track];
  }
  return false;
}

bool Sequencer::isLoopPaused(int track) {
  if (track >= 0 && track < MAX_TRACKS) {
    return loopPaused[track];
  }
  return false;
}

void Sequencer::processLoops() {
  // Process looped tracks every step
  for (int track = 0; track < MAX_TRACKS; track++) {
    if (loopActive[track] && !loopPaused[track] && !trackMuted[track]) {
      if (stepCallback != nullptr) {
        stepCallback(track, 100); // Loop triggers at consistent velocity
      }
    }
  }
}

