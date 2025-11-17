# ESPHome Solar Thermal Panel Integration

This repository provides ESPHome configuration and custom components to integrate a solar-thermal panel (collector) controller, exposing its data via ESPHome using a DL-bus sensor component or VBUS sensor component. It enables reading temperatures, flow, and other relevant values from a solar thermal system, and forwarding them to Home Assistant, MQTT, or other platforms.

**Repository**: https://github.com/barais/esphomesolarthermalpanel

## 🔗 Integration With the rmqttconnector Project

This project can be combined with the open-source stack provided by [rmqttconnector](https://github.com/barais/rmqttconnector), enabling a full end-to-end monitoring and analytics ecosystem. In this setup, the ESPHome solar-thermal component publishes measurements to an MQTT broker, while rmqttconnector reliably streams these MQTT messages into a TimescaleDB time-series database. From there, tools such as Grafana can be used to build advanced dashboards, long-term analytics, performance reports, and alerting rules.
Together, these components form a complete, fully open-source solution for solar-thermal system monitoring, historical data exploration, and automated supervision.

---

## 🌞 Overview

- Use of a **DL-bus (data bus)** or **Vbus (data bus)** protocol to communicate with the solar thermal controller.  
- Custom ESPHome component inside `my_components/sensordlbus` for reading DL-bus frames.  

- Example YAML configurations (`stage1.yaml`, `dlbus.stage3.yaml`, etc.) to bootstrap and deploy the system.  
- Modularity: you can adapt or extend the sensors read by DL-bus.  
- Compatible with ESPHome’s external components mechanism. 

## 🧬 Origin of the DL-bus Component

This custom ESPHome component is partly inspired by the DL-Bus communication work from the [ESPEasy project](https://espeasy.readthedocs.io), specifically the implementation described in [Plugin P092_DLbus](https://espeasy.readthedocs.io/en/latest/Plugin/P092_DLbus.html)
.

While the codebase and integration model were rewritten to fit ESPHome’s architecture and component system, the fundamental understanding of DL-Bus framing, message structure, and parsing logic originates from that pioneering ESPEasy reverse-engineering effort. This project extends that work by providing a clean, maintainable, ESPHome-native implementation that integrates seamlessly with Home Assistant, MQTT, and modern automation systems.

---

## 📁 Repository Structure

```
esphomesolarthermalpanel/
├── my_components/
│ └── sensordlbus/ # Custom ESPHome component for DL-bus
│ ├── sensor.cpp
│ ├── sensor.h
│ └── other sources...
├── dlbus.stage3.yaml # Example YAML (final / stable config)
├── stage1.yaml # Initial ESPHome YAML for setup
├── stage2template.yaml # Template for second-stage config
├── vbus.stage3.yaml # Optional: example with VBus if applicable
├── .gitignore
└── README.md # This file
```


---

## ⚙️ How It Works

1. **VBus Communication**  
   We use directly the [VBUS Component](https://esphome.io/components/vbus/)  from ESPHome 

2. **DL-Bus Communication**  
   The custom component reads DL-bus messages from your solar thermal controller.  

3. **Sensor Mapping**  
   Inside the component you can define which DL-bus registers or frame fields correspond to "sensors" (e.g. collector temperature, return temperature, flow rate, pump state).  

4. **ESPHome Integration**  
   Use the ESPHome YAML examples to declare the custom component, associate sensors, and configure update intervals.  

5. **Data Exposure**  
   Once running, the device exposes sensor values via standard ESPHome mechanisms (MQTT, Home Assistant API, etc.).  

---

## 🔌 Hardware Requirements

To operate this project in a safe and reliable manner, a minimal hardware setup is required. The system is based on an ESP32 (or any other microcontroller fully compatible with ESPHome) that interfaces with the solar-thermal controller’s DL-Bus. For electrical protection and correct signal isolation, a standard 817 optocoupler must be used, following the wiring schematic provided below, ensuring that the controller’s communication bus is safely decoupled from the ESP device. The ESP32 should be powered using a 220V → 3.3V or 220V → 5V power supply, depending on the board variant.
Additionally, a dedicated HAT for the WT32-ETH01 module—which brings hardware Ethernet support for improved reliability—is currently under design and will provide an even cleaner and safer integration in the future.

![](images/vbus_serial_optocoupler.png)


## 📘 Example Configuration if using DL-BUS

Here is a simplified snippet illustrating how to include the custom DL-bus component in an ESPHome YAML:

```yaml
esphome:
  name: solar_thermal
  platform: ESP32
  board: esp32dev


external_components:
  - source: 
      type: local
      path: ./my_components/

....

sensor:
  - platform: sensordlbus
    dl_pin: GPIO5
    id: sensordlbusreader1
    model: "UVR61-3" # possible values: "UVR42", "ESR21", "UVR31", "UVR67", "UVR1611", "UVR61-3-83Plus", "UVR61-3" 
    temperature_1:
      id: temp1
      name: "DLBus Temperature 1"
    temperature_2:
      id: temp2
      name: "DLBus Temperature 2"
      on_value:
          then:
            mqtt.publish_json:
              qos: 1
              retain: true        
              topic: ${mqttstate_topic}
              payload: |-
                root["device_id"] = ${device_id};
                root["temp1"] = id(temp1).state;
                root["temp2"] = id(temp2).state;

```

⚠️ The exact sensor names (collector_temp, return_temp, etc.) depend on how the DL-bus frame parsing is implemented in sensordlbus.

## 🧪 Getting Started

### Clone the repository

```bash
git clone https://github.com/barais/esphomesolarthermalpanel.git
```

### Copy or reference the component

- Option A: Use external_components in your ESPHome YAML to refer to the *my_components/sensordlbus* folder.

- Option B: Copy the sensordlbus folder directly into your ESPHome configuration directory.

### Configure your ESPHome YAML

- Use one of the example YAMLs (stage1.yaml, dlbus.stage3.yaml) as a base.

### Flash the ESP device

Use esphome run (or via the ESPHome dashboard) to compile and upload the firmware.

```bash
 docker run --rm --privileged -v "${PWD}":/config --device=/dev/ttyUSB0 -it ghcr.io/esphome/esphome run  dlbus.stage3.yaml
```


### Verify logs

Check ESPHome logs to ensure DL-bus frames are correctly received and parsed.

### Integrate into your system

- If you use MQTT, configure ESPHome’s mqtt: section.
- If you use Home Assistant, you can leverage the API integration or directly read the sensors there.

## 📊 Use Cases

- Monitor panel collector temperature, return temperature, flow rate, etc.
- Trigger alerts: e.g., when the collector is too hot or flow is abnormal.
- Build dashboards in Home Assistant or Grafana.
- Use values for automation: start/stop pump, adjust valves, etc.

## 🛠️ Customization & Extension

You can extend the DL-bus component to parse additional frame fields.

- Add new templates or sensor definitions in your YAML.
- Tune update_interval depending on how often you want new data.
- If your controller uses VBus instead of DL-bus, you could adapt or use the official ESPHome [VBus component]. 
esphome.io

## 📖 References

- [ESPHome External Components Documentation](https://esphome.io)

## 🚀 Contributing

Contributions are welcome! Whether it’s:

- Adding new DL-bus frame parsing for other devices
- Bug fixes or documentation improvements

Feel free to open an issue or submit a pull request.



## TODOList

- [ ] Manage 50hz frequency for UVR31, UVR42, UVR64
- [ ] Integrate other custom components for existing protocols