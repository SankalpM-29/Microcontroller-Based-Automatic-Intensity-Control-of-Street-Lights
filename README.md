# Microcontroller-Based-Automatic-Intensity-Control-of-Street-Lights

The proposed system controls the street lights using light dependent sensor and PIR sensor.

- Street lights are switched on depending on the intensity of sunlight on LDR. If the intensity of sunlight on LDR is low, its resistance value is high. This value increases and becomes high when it is completely in dark. 
- This resistance value decides when the street lights are required to switch ON. As the resistance value is maximum in the midnight, street lights turn themself ON with full intensity.
- Another feature of this project is that street light's intensity vary according to vehicles on the road. RTC comes into the picture now. The controller checks peak time during which there is no traffic and keeps the lights switched OFF. 
- When there is any vehicle on the road, it is detected by the PIR sensor. It just indicates the microcontroller to switch on the street lights. The lights are switched on for 2 to 3 minutes and switched off automatically. 

# Block Diagram

![Block Diagram of Project!](block_diagram.jpg "Block Diagram of Project")
