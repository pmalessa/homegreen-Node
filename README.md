# HomeGreen - self watering system
## Description
A self watering system for outdoor plants. This project contains the hardware and software files to completely build a HomeGreen node on your own.
The system is based on an inexpensive node, which is able to control up to 3 pumps and can be powered by nearly any conventional powerbank. It has a basic user interface using a 6-Digit 7-Segment Display combined with 4 touch buttons. As a pump, you could either use a 4-6V DC pump or a 5V relay to control a higher voltage pump. The node is designed to be water resistant and can be easily mounted using embedded magnets or a 3D printed holder.

## Repository structure
This repo contains everything you need to build a homegreen node on your own:

- hardware: The EAGLE PCB and GERBER files, BOM and models and dimensions for the case
- firmware: The firmware for the microcontroller (ATtimy88) on the node

## Hardware
The HomeGreen node PCB is designed to be enclosed in a 25mm diameter transparent pipe to be water resistant. The touch buttons can be used through the pipe without the need to open the case. The 2 ends need plastic caps, which could be easily 3D printed. one needs to be fully closed, while the other one needs to have an opening the for USB-A connector of the pump as well as the micro USB connector of the powerbank.

## Firmware
The firmware is written in low-level C++ and uses the PlatformIO framework. It is designed to run as energy efficient as possible to be able to run on a powerbank for a long time.

## Detailed description
The HomeGreen node contains the following main components:
- an attiny88 microcontroller as main controller
- a sophisticated power management circuit to be able to sleep and wake up the connected powerbank regularly
- a 6-Digit 7-Segment Display with its controller as user interface
- a 4-Channel touch button controller as user interface
- an I2C temperature sensor to measure the temperature of the environment
- a current limiter, step-up converter and a current measurement circuit to adjust and monitor the output power for the pump
- the ability to control up to 3 pumps sequentially using an additional HomeGreen Pump Hub

### Power management
The main issue of supplying your gadgets using a powerbank is the fact, that nearly all powerbanks shut off after a certain time of inactivity. As the sleep current draw of the HomeGreen node is substantially lower than any charge current the powerbank is designed for, it treats it as inactive and shuts off eventually. 
This led to the necessity to have a supercapacitor as an additional short-time energy storage onboard to be able to power the controller while in sleep and only wake the powerbank once the supercap is drained or it is time to enable the pump.
Additionally, each powerbank is behaving differently when it comes to the sleep and wake behavior. Some powerbanks wake up immediately once you connect the capacitive load of the supercap, while others require an additional substantial resistive load to enable its regulators. Strangely, some powerbanks even change their polarity and apply -2.5V to their output!
To support most of the available powerbanks on the market 2 MOSFETS back to back, combined with a charge pump and an additional controllable resistive load was necessary. Even with that, some powerbanks do not work reliably and should be avoided. A list of tested powerbanks will be added.

### User Interface
The HomeGreen Node consists of a 6-Digit 7-Segment display as well as 4 touch buttons to view its status and adjust all parameters to your needs. The HomeGreen Node can be used to control up to 3 pumps using an additional HoemGreen Pump Hub. Once connected, the user interface switches to the ability to adjust timing of each pump separately. However, if no Pump Hub is connected, it is simplyfied to control just one pump.

#### Sleep State
In its default sleep state, the 7-Segment Display is turned off and only the 2 Status LEDs are displaying its state. 
- If it blinks green once every 8 seconds, everything is fine. 
- If it blinks green twice and in a faster interval, it charges its supercap. This happens every 5-15mins. 
- If it blinks red, an error occured and the Node should be checked by waking it to get more details.
- If it is time to water the plants, it will wake itself and continue with the Pump State.
- To wake the Node, just touch one of its touch buttons. It will then try to wake the powerbank and will turn on its Display, once successful. It may take multiple tries sometimes depending on the model of powerbank. It then moves into Info State. If an error occured, it will switch into Error State after wake up before going to Info State.

#### Info State
In the info state, you see the set interval and duration of the pump cycle as well as the time left till the next pump cycle at one glance. If a Pump Hub is connected, you can switch between the config for each pump with a short press of the SET button.
- If no button is pressed, it shuts off and switches back to Sleep State after some time.
- To adjust the interval or duration of the pump cycle, long press the SET button and wait for the display to fade out and in. It now switched into Config State.
- To start a manual pump cycle, just long press the MANUAL button. It then switches into Pump State.

#### Config State
In the Config State, you can adjust the interval and duration of the pump cycle. The currently selected parameter is displayed by blinking its number.
- To increase the parameter, short press or hold the + button.
- To decrease the parameter, short press of hold the - button.
- To switch the selected parameter, short press the SET button.
- To save the newly configured parameters, long press the SET button. It switches to Info State then.
- If no button have been pressed for a longer period, changes are discarded and it switches to Info State.
- To get more advanced information and configuration options, long press the MANUAL button to switch to Advanced State.

#### Pump State
Once the pump interval elapsed or a manual pump cycle has been started, the Node switches into Pump State. In Pump State, an animation is displayed as well as the time left to pump in the current cycle. If a Pump Hub is connected, it will additionally display which pump is currently enabled.
- To increase the current pump cycle by 1 minute, short press the + button.
- To decrease the current pump cycle by 1 minute, short press the - button.
- To manually switch to the next pump, short press the SET button.
- To stop the current pump cycle, long press the MANUAL button.
- Once the pump duration of all pumps elapsed, the Node will switch to Info State to shortly display its current parameters, and after some time of inactivity it switches back to Sleep State.

#### Error State
If an error ocurred, the red led blinks to signal that something went wrong. To check for details, wake the node and it will switch to this state.
The following errors are possible:
- PB_ERR: Powerbank-Error - The node was not possible to wake the powerbank at its last wake up by a pump cycle or a user action. Check the charge state or cabling of the powerbank. Maybe that model is incompatible with the Node, check the list of tested powerbanks for hints.
- P1_ERR/P2_ERR/P3_ERR: Pump1/2/3-Error - There is an issue with the mentioned pump, the current draw had an anomaly. Maybe the cable is broken, the water tank is empty or the pump is clogged.
- EP_ERR: EEPROM-Error - There is an issue with the save state of the configuration. This rarely happens, but sometimes the config can get corrupted by battery drain. The config will be reset to default state and it has to be reconfigured.

The following interactions are possible:
- To clear the current error, short press any button.
- To ignore the current error permanently (until the next reset to defaults), long press the MANUAL button.

#### Advanced State
The Advanced State contains additional information and configuration options to move through.
The following informations are displayed:
- The build date of the firmware
- The build version of the firmware
- The current runtime in hours
- The total runtime in hours

The following interactions are possible:
- To move to the next information, short press the SET button
- To leave the Advanced State back to Info State, long press the SET button
- To reset all configurations to its default state, long press the MANUAL button. It then switches back to Info State.