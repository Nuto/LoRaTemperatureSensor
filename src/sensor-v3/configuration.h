//Libraries for Permanent memory
#include <Preferences.h>

//Prepare Permanent memory (EEPROM alternative)
Preferences preferences;

//Set config keys
char* moduleUniqueidentifierKey = "muid";
char* temperatureCompensationKey = "tc";

uint8_t getModuleUniqueidentifier() {
  uint8_t value = 0;

  if (!preferences.begin("config", true)) {
    return value;
  }

  if (!preferences.isKey(moduleUniqueidentifierKey)) {
    preferences.end();

    return value;
  }

  value = preferences.getUChar(moduleUniqueidentifierKey, 0);
  preferences.end();

  return value;
}

void setModuleUniqueidentifier(uint8_t uniqueIdentifier) {
  if (!preferences.begin("config", false)) {
    return;
  }
  preferences.putUChar(moduleUniqueidentifierKey, uniqueIdentifier);
  preferences.end();
}

float getTemperatureCompensation() {
  float value = 0;
  if (!preferences.begin("config", true)) {
    return value;
  }
  value = preferences.getFloat(temperatureCompensationKey, 0);
  preferences.end();

  return value;
}

void setTemperatureCompensation(float temperatureCompensation) {
  if (!preferences.begin("config", false)) {
    return;
  }
  preferences.putFloat(temperatureCompensationKey, temperatureCompensation);
  preferences.end();
}
