# Sensor V2

## Advantages

- Deep sleep (Improved heat generation in the housing)
- Optimized for battery operation
- Temperature average of the last 20 values
- No LoRaWAN required, communication via LoRa to the Gateway-Module

## Disadvantages

- No permanently active display (temperature and humidty)

## Available commands

- `config.muid=LSM2` Set the module name
- `config.tc=-0.2` Set a temperature compensation
- `reset` Restart the module
