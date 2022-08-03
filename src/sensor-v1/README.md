# Sensor V1

## Advantages

- Permanently active display (temperature and humidty)
- Temperature average of the last 20 values
- No LoRaWAN required, communication via LoRa to the Gateway-Module

## Disadvantages

- No deep sleep (heat generation in the housing)

## Available commands

- `config.muid=LSM2` Set the module name
- `config.tc=-0.2` Set a temperature compensation
- `reset` Restart the module
