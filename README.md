Universal cruise control for cars

# What it is
This project is about to bring cruise control to my Fiat 126p, but basically it should be possible to adapt it to any car.

# Sensors used
- hall sensor as a speed sensor
- brake light switch (already in place)
- clutch switch (has to be added)
- Three buttons (+, -, cancel)

# What is driving what
Based on the speed sensor, the device regulates PWM signal with PI loop. It tries to keep the vehicle speed as close as possible to the one set with the buttons. What controls the engine is a simple, but "strong" servomotor. In my case it is TiankongRC TD-6620MG (they say it has metal gears, 180 degrees of movement and 20 kg of torque). The servo is attached to gas pedal linkage as it is a simple connection and there are no vibrations on the floor compared to the engine.

# Where is the firmware
Coming<sup>TM</sup>

It's bare metal and I started writing it with HW v1, so I have to make some changes (not speaking about other projects).

# Changes in HW between versions
v1:
- controls DC motor with H bridge. It came out as not so simple due to lack of motor position sensing.
- buttons and switches had to be debounced in code, this takes time and control over cruise control is somehow critical on the road.
- buttons were driven from the VCC.
- I misplaced pins in AMSR20783.3JZ footprint.

v2:
- improved misconceptions from above.
- JST connectors instead of goldpins.
- improved routing and footprints (for example SOT123 in place of SOT323 for improved solderability).
