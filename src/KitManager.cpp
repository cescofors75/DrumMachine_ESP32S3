/*
 * KitManager.cpp
 * Implementació del gestor de kits
 */

#include "KitManager.h"

extern SampleManager sampleManager;

KitManager::KitManager() : kitCount(0), currentKit(-1) {
  for (int i = 0; i < MAX_KITS; i++) {
    memset(kits[i].name, 0, 32);
    kits[i].sampleCount = 0;
  }
}

KitManager::~KitManager() {
}

bool KitManager::begin() {
  Serial.println("Initializing Kit Manager...");
  
  // Scan for available kits
  int count = scanKits();
  
  if (count > 0) {
    Serial.printf("Found %d kits\n", count);
    
    // Load first kit by default
    loadKit(0);
    return true;
  } else {
    Serial.println("No kits found!");
    return false;
  }
}

int KitManager::scanKits() {
  kitCount = 0;
  
  // 16 carpetas de instrumentos
  const char* folders[16] = {
    "/BD", "/SD", "/CH", "/OH", "/CP", "/CB", "/RS", "/CL",
    "/MA", "/CY", "/HT", "/LT", "/MC", "/MT", "/HC", "/LC"
  };
  
  // Kit Único: Todos los instrumentos disponibles
  Kit& kit = kits[0];
  strncpy(kit.name, "RED808 16-Track", 31);
  kit.sampleCount = 0;

  // Cargar primer sample de cada carpeta
  for (int i = 0; i < 16 && kit.sampleCount < MAX_SAMPLES_PER_KIT; i++) {
    File dir = LittleFS.open(folders[i]);
    if (!dir || !dir.isDirectory()) {
      Serial.printf("  ⚠️  Carpeta %s no encontrada\n", folders[i]);
      continue;
    }
    
    // Buscar primer archivo .wav o .WAV
    File file = dir.openNextFile();
    bool found = false;
    while (file && !found) {
      String filename = file.name();
      filename.toUpperCase();
      
      if (!file.isDirectory() && filename.endsWith(".WAV")) {
        // Construir path completo
        char fullPath[128];
        snprintf(fullPath, 127, "%s/%s", folders[i], file.name());
        
        // Agregar al kit
        kit.samples[kit.sampleCount].padIndex = i;
        strncpy(kit.samples[kit.sampleCount].filename, fullPath, 63);
        kit.sampleCount++;
        
        Serial.printf("  ✓ Track %02d: %s\n", i, fullPath);
        found = true;
      }
      
      file = dir.openNextFile();
    }
    
    if (!found) {
      Serial.printf("  ⚠️  Track %02d (%s): sin samples\n", i, folders[i]);
    }
  }
  
  if (kit.sampleCount > 0) {
    kitCount = 1;
    Serial.printf("\n✓ Kit '%s' con %d tracks cargados\n", kit.name, kit.sampleCount);
  } else {
    Serial.println("❌ No se encontraron samples");
  }
  
  return kitCount;
}

bool KitManager::parseKitFile(const char* filename, int kitIndex) {
  fs::File file = LittleFS.open(filename);
  if (!file) {
    Serial.printf("Failed to open: %s\n", filename);
    return false;
  }
  
  Kit& kit = kits[kitIndex];
  kit.sampleCount = 0;
  
  // Extract kit name from filename (kit1.txt -> kit1)
  String name = String(filename);
  int start = name.lastIndexOf('/') + 1;
  int end = name.lastIndexOf('.');
  name = name.substring(start, end);
  name.toCharArray(kit.name, 32);
  
  // Parse file
  while (file.available() && kit.sampleCount < MAX_SAMPLES_PER_KIT) {
    String line = file.readStringUntil('\n');
    line.trim();
    
    // Skip empty lines and comments
    if (line.length() == 0 || line.startsWith("#")) {
      // Try to extract kit name from comment
      if (line.startsWith("# ") && kit.name[0] == 'k') {
        String nameFromComment = line.substring(2);
        nameFromComment.trim();
        if (nameFromComment.length() > 0 && nameFromComment.length() < 32) {
          nameFromComment.toCharArray(kit.name, 32);
        }
      }
      continue;
    }
    
    // Parse line: "pad_index filename.wav"
    int spaceIdx = line.indexOf(' ');
    if (spaceIdx > 0) {
      int pad = line.substring(0, spaceIdx).toInt();
      String sampleFile = line.substring(spaceIdx + 1);
      sampleFile.trim();
      
      if (pad >= 0 && pad < 8 && sampleFile.length() > 0) {
        kit.samples[kit.sampleCount].padIndex = pad;
        sampleFile.toCharArray(kit.samples[kit.sampleCount].filename, 64);
        kit.sampleCount++;
      }
    }
  }
  
  file.close();
  
  Serial.printf("Loaded kit '%s' with %d samples\n", kit.name, kit.sampleCount);
  return kit.sampleCount > 0;
}

bool KitManager::loadKit(int kitIndex) {
  if (kitIndex < 0 || kitIndex >= kitCount) {
    Serial.printf("Error: Kit %d no existe\n", kitIndex);
    return false;
  }
  
  currentKit = kitIndex;
  Kit& kit = kits[kitIndex];
  
  Serial.printf("\n========== CARGANDO KIT %d: %s ==========\n", kitIndex, kit.name);
  
  // Unload current samples
  sampleManager.unloadAll();
  
  // Load all samples from this kit
  int loaded = 0;
  for (int i = 0; i < kit.sampleCount; i++) {
    int padIndex = kit.samples[i].padIndex;
    const char* filename = kit.samples[i].filename;
    
    Serial.printf("  Pad %d -> %s\n", padIndex, filename);
    
    if (sampleManager.loadSample(filename, padIndex)) {
      loaded++;
      Serial.printf("    OK\n");
    } else {
      Serial.printf("    ERROR cargando sample!\n");
    }
  }
  
  Serial.printf("========== KIT CARGADO: %d/%d samples ==========\n\n", loaded, kit.sampleCount);
  
  return loaded > 0;
}

const char* KitManager::getKitName(int kitIndex) {
  if (kitIndex < 0 || kitIndex >= kitCount) {
    return "";
  }
  return kits[kitIndex].name;
}

void KitManager::printKitInfo(int kitIndex) {
  if (kitIndex < 0 || kitIndex >= kitCount) return;
  
  Kit& kit = kits[kitIndex];
  
  Serial.println("========================================");
  Serial.printf("Kit %d: %s\n", kitIndex, kit.name);
  Serial.println("----------------------------------------");
  Serial.printf("Samples: %d\n", kit.sampleCount);
  Serial.println("----------------------------------------");
  
  for (int i = 0; i < kit.sampleCount; i++) {
    Serial.printf("  Pad %2d: %s\n", 
                  kit.samples[i].padIndex, 
                  kit.samples[i].filename);
  }
  
  Serial.println("========================================");
}
