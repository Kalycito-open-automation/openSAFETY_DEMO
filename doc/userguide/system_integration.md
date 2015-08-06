System integration, start up {#page_system_integration}
============

[TOC]

This pages provides information on how to integrate the openSAFETY_DEMO into
a control system and putting the whole system into operation
explained by taking the example of a B&R control system, using openSAFETY.

# Automation Studio and SafeDESIGNER {#sect_as_sd_intro}

In order to integrate the openSAFETY_DEMO in the B&R system and putting the
whole system into operation the tools Automation Studio and SafeDESIGNER are
needed. Automation Studio is an integrated development environment used,
among others, for planning implementing and configuring B&R control systems.
SafeDESIGNER is used to to develop safe logic for the SafePLC and configuring
the openSAFETY safe nodes.

Make sure the tools are installed:
  - Automation Studio 4.2.4 or higher
  - SafeDESIGNER 4.2 or higher


> **Note:** The help pages of Automation Studio and SafeDESIGNER provide
> comprehensive information about the B&R hardware, its configuration
> possibilities and much more. Refer to this help for information not covered
> in this guide.

## Importing the openSAFETY_DEMO {#sect_as_osdd_import}

For importing the openSAFETY_DEMO device, perform the following steps:

1. Start Automation Studio
2. Import the OSDD file from the directory **device_description** via the
    menu `Tools -> Import Fieldbus Device...`, then select the XDD file in the
    window opened afterwards.

## Setting up the hardware {#sect_as_hw_setup}

1. Ensure that the development hardware is set up as described in the sections
  `How to setup the board` of the used targets.
2. Connect the openSAFETY_DEMO device to the B&R system as shown in the picture
  below using Ethernet cables.
3. Plug the SafeKEY into the SafePLC.
4. Set the node number of the SafePLC to 1, set the node number of the
  openSAFETY_DEMO device to 2 (switch SW11 up, all others down).

![Hardware structure](br_hardware.png)
@image latex br_hardware.png "Hardware structure" width=0.75\textwidth

## Downloading the software {#sect_as_sw_setup}

For downloading the software to the B&R hardware, perform the following steps:

1. Open Automation Studio and open the sample project located in the directory
  **automation_studio** via the menu `File -> Open Project`.
2. If there is a Compact Flash card reader available, ensure the PLC is powered
  off and remove the Compact Flash card. Insert the card into the card reader
  and click `Tools -> Create Compact Flash`. Select the proper drive and click
  `Creat Compact Flash`. After the creation has finished, put the card back into
  the PLC.

3. Connect the Ethernet interface of the PLC to the same network as the
   computer or connect it directly to the computer and power up the PLC system.

4. In Automation Studio click `Online -> Settings... ` from the menu and then
  click the refresh button. If the PLC shows up with a exclamation mark,
  right click on the PLC in the list and select `Set IP Parameters`.
  In the opened window, set the IP parameters matching your computer's IP
  settings, but with another IP address or select
  `get IP address from DHCP server`.
  Click `OK`and refresh the device list.
  The exclamation mark should be gone.

5. * Right-Click on the PLC and select
  `Connect`.
6. The status bar at the bottom of the Automation Studio should now show the
  connected PLC and its status.

7. When the connection with the PLC is established, Compile the Automation
  Studio Project and click `Transfer to Target` to download it to the PLC.

## Downloading the SafePLC software {#sect_as_safedesigner}

Make sure all the used hardware is powered up and flashed with the appropriate
software. The status of the application processors can be monitored by opening
a terminal program and then opening the virtual COM port.
The following settings are needed for the COM port:
- Port: Virtual COM port of the Nucleo board
- Baud rate: 115200
- Data: 8bit
- Paraity: none
- Stop: 1 bit
- Flow control: none

To download the software to the SafePLC, perform the following steps:

1. In the System Designer view, open the SafeDESIGNER by right-clicking on the
  SafePLC and selecting `SafeDESIGNER`.

2. When prompted, enter the project password : 123456

3. Click the button `Compile` to build the project.

4. Find the module SL1.M1 in the Safety View located in the left bottom corner
  of the SafeDESIGNER. Right-click on the module and  select `Remote Control`.
  If downloading the software for the first time, a prompt will be shown
  asking to set a new password. Set the password to 123456 and confirm it.
  If an unknown password is pre-set, but unknown, the SafeKEY can be formatted.
  Follow the instructions in the Automation Studio Help in
  `Hardware -> X20 System -> X20 modules -> X20 CPUs -> X20SL80XX -> Control and connection elements -> Safety processor -> Selection switch and confirmation button`

5. In the remote control window, select `Download` to download the software.

6. After the download has finished, wait for the SafePLC to restart.
  Observe the buttons for confirming a new SafeKEY, firmware or module.
  The related button will be coloured, if a confirmation is needed.
  Click the button and wait for the SafeLogic to restart.

7. When the SafePLC is in RUN mode, the status of the variables of the
  openSAFETY_DEMO device can be observed by clicking the button `Variable status`

