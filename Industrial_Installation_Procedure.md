# Industrial Installation Procedure -- Opthelios Performance Monitoring Node

This document describes the complete procedure for deploying an
Opthelios performance-monitoring node on an industrial site.\
The node is delivered with a minimal pre-configuration and must be
customized on-site and remotely to ensure correct communication with the
central MQTT server.

## 1. Node Preparation

Each node comes preloaded with a minimal configuration.\
This configuration only enables a Wi-Fi access point (hotspot) to allow
initial setup. No networking, DL-Bus, or VBus configuration is active
yet.

-   **Wi-Fi SSID:** `opthelios`
-   **Wi-Fi Password:** `opthelios`

The hotspot becomes visible approximately **2 minutes after powering the
device**.

## 2. Collecting Site Information

Before configuring the node, the technician must obtain all required
network information from the site.

The following parameters must be collected:

  -----------------------------------------------------------------------
  Parameter              Description                  Example
  ---------------------- ---------------------------- -------------------
  **Connection Type**    Wi-Fi or Ethernet            Ethernet

  **IP Mode**            Static or DHCP               Static

  **Static IP Address**  Required if IP mode is       `192.168.1.50`
                         static                       

  **Subnet Mask**        ---                          `255.255.255.0`

  **Gateway**            ---                          `192.168.1.1`

  **DNS Servers**        If not provided, use Google  `8.8.8.8`,
                         DNS                          `8.8.4.4`
  -----------------------------------------------------------------------

### MQTT Connectivity Requirement

Ensure that **port 1883** is open for outbound communication to the
central server.\
This must be verified before installation.

#### Network Communication Topology

    [Node] ⇄ [Local Router] ⇄ [Internet] ⇄ [MQTT Server]
                      (Port 1883 open)

## 3. Generation of the Site-Specific Firmware

The IT center prepares a custom firmware based on the collected site
information:

-   Static network configuration\
-   Node identifier\
-   Type of data bus (DL-Bus or VBus)\
-   Site-specific operational parameters

The generated firmware is then sent to the technician by email.

## 4. On-Site Installation Procedure

### Step 4.1 -- Hardware Connections

1.  Connect the **AC power supply (220 V)** to the node.\
2.  Connect the **data bus** (DL-Bus or VBus) depending on the
    controller model.\
3.  Wait **about 2 minutes** until the Wi-Fi hotspot `opthelios` becomes
    visible.

### Step 4.2 -- Connect to the Local Access Point

1.  On a mobile device or laptop:
    -   Connect to Wi-Fi SSID: `opthelios`\
    -   Password: `opthelios`
2.  Disable mobile data to avoid IP conflicts.\
3.  Open a browser and navigate to:

```{=html}
<!-- -->
```
    http://opthelios.local

### Step 4.3 -- Upload the Site-Specific Firmware (OTA Update)

1.  Go to **OTA Update** in the interface.\
2.  Upload the firmware file received by email.\
3.  The device will install the firmware and reboot automatically.

## 5. Verification and Final Setup

### 5.1 -- Server Visibility Check

The technician must contact the IT center to confirm that the node is
correctly communicating with the central MQTT server.

### 5.2 -- Remote Final Configuration

Once the node is online, the IT center:

1.  Performs the complete configuration of the DL-Bus or VBus
    interface.\
2.  Pushes the final firmware build remotely using MQTT.\
3.  The node automatically downloads and installs the final version.

#### Remote Update Flow

    [IT Center] → (MQTT) → [On-Site Node]
            Automatic download & installation

This completes the installation.
