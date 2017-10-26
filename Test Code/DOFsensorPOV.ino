/*
Attempt to incorporate LSM9DS1 into POV code using 
magetometer interrupt

Hardware setup: This library supports communicating with the
LSM9DS1 over either I2C or SPI. This example demonstrates how
to use I2C. The pin-out is as follows:
  LSM9DS1 --------- Arduino
   SCL ---------- SCL (A5 on older 'Duinos')
   SDA ---------- SDA (A4 on older 'Duinos')
   VDD ------------- 3.3V
   GND ------------- GND
   INT2 ------------- D4
   INT1 ------------- D3
   INTM ------------- D5
(CSG, CSXM, SDOG, and SDOXM should all be pulled high. 
Jumpers on the breakout board will do this for you.)

*/
// Include the SparkFunLSM9DS1 library and its dependencies.
#include <Wire.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>

#include "FastLED.h" //Include library for LED strip
#define NUM_LEDS 100 //number of leds in strip length on one side
#define DATA_PIN 2//7 = second hardware spi data
#define CLOCK_PIN 3//14 = second hardware spi clock
CRGB leds[NUM_LEDS];
int numberOfSlices = 150;

 

LSM9DS1 imu; // Create an LSM9DS1 object to use from here on.

///////////////////////////////
// Interrupt Pin Definitions //
///////////////////////////////
// These can be swapped to any available digital pin:
const int INT1_PIN_THS = 3; // INT1 pin to D3 - will be attached to gyro
const int INT2_PIN_DRDY = 4; // INT2 pin to D4 - attached to accel
const int INTM_PIN_THS = 5;  // INTM_PIN_THS pin to D5
const int RDYM_PIN = 6;  // RDY pin to D6

// Variable to keep track of when we print sensor readings:
unsigned long lastPrint = 0;

// configureIMU sets up our LSM9DS1 interface, sensor scales
// and sample rates.
uint16_t configureIMU()
{
  // Set up Device Mode (I2C) and I2C addresses:
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.agAddress = LSM9DS1_AG_ADDR(1);
  imu.settings.device.mAddress = LSM9DS1_M_ADDR(1);

  // gyro.latchInterrupt controls the latching of the
  // gyro and accelerometer interrupts (INT1 and INT2).
  // false = no latching
  imu.settings.gyro.latchInterrupt = false;

  // Set gyroscope scale to +/-245 dps:
  imu.settings.gyro.scale = 245;
  // Set gyroscope (and accel) sample rate to 14.9 Hz
  imu.settings.gyro.sampleRate = 1;
  // Set accelerometer scale to +/-2g
  imu.settings.accel.scale = 2;
  // Set magnetometer scale to +/- 4g
  imu.settings.mag.scale = 4;
  // Set magnetometer sample rate to 0.625 Hz
  imu.settings.mag.sampleRate = 0;

  // Call imu.begin() to initialize the sensor and instill
  // it with our new settings.
  return imu.begin();
}

void configureLSM9DS1Interrupts()
{
  /////////////////////////////////////////////
  // Configure INT1 - Gyro & Accel Threshold //
  /////////////////////////////////////////////
  // For more information on setting gyro interrupt, threshold,
  // and configuring the intterup, see the datasheet.
  // We'll configure INT_GEN_CFG_G, INT_GEN_THS_??_G, 
  // INT_GEN_DUR_G, and INT1_CTRL.
  // 1. Configure the gyro interrupt generator:
  //  - ZHIE_G: Z-axis high event (more can be or'd together)
  //  - false: and/or (false = OR) (not applicable)
  //  - false: latch interrupt (false = not latched)
  imu.configGyroInt(ZHIE_G, false, false);
  // 2. Configure the gyro threshold
  //   - 500: Threshold (raw value from gyro)
  //   - Z_AXIS: Z-axis threshold
  //   - 10: duration (based on ODR)
  //   - true: wait (wait duration before interrupt goes low)
  imu.configGyroThs(500, Z_AXIS, 10, true);
  // 3. Configure accelerometer interrupt generator:
  //   - XHIE_XL: x-axis high event
  //     More axis events can be or'd together
  //   - false: OR interrupts (N/A, since we only have 1)
  imu.configAccelInt(XHIE_XL, false);
  // 4. Configure accelerometer threshold:
  //   - 20: Threshold (raw value from accel)
  //     Multiply this value by 128 to get threshold value.
  //     (20 = 2600 raw accel value)
  //   - X_AXIS: Write to X-axis threshold
  //   - 10: duration (based on ODR)
  //   - false: wait (wait [duration] before interrupt goes low)
  imu.configAccelThs(20, X_AXIS, 1, false);
  // 5. Configure INT1 - assign it to gyro interrupt
  //   - XG_INT1: Says we're configuring INT1
  //   - INT1_IG_G | INT1_IG_XL: Sets interrupt source to 
  //     both gyro interrupt and accel
  //   - INT_ACTIVE_LOW: Sets interrupt to active low.
  //         (Can otherwise be set to INT_ACTIVE_HIGH.)
  //   - INT_PUSH_PULL: Sets interrupt to a push-pull.
  //         (Can otherwise be set to INT_OPEN_DRAIN.)
  imu.configInt(XG_INT1, INT1_IG_G | INT_IG_XL, INT_ACTIVE_LOW, INT_PUSH_PULL);

  ////////////////////////////////////////////////
  // Configure INT2 - Gyro and Accel Data Ready //
  ////////////////////////////////////////////////
  // Configure interrupt 2 to fire whenever new accelerometer
  // or gyroscope data is available.
  // Note XG_INT2 means configuring interrupt 2.
  // INT_DRDY_XL is OR'd with INT_DRDY_G
  imu.configInt(XG_INT2, INT_DRDY_XL | INT_DRDY_G, INT_ACTIVE_LOW, INT_PUSH_PULL);

  //////////////////////////////////////
  // Configure Magnetometer Interrupt //
  //////////////////////////////////////
  // 1. Configure magnetometer interrupt:
  //   - XIEN: axis to be monitored. Can be an or'd combination
  //     of XIEN, YIEN, or ZIEN.
  //   - INT_ACTIVE_LOW: Interrupt goes low when active.
  //   - true: Latch interrupt
  imu.configMagInt(XIEN, INT_ACTIVE_LOW, true);
  // 2. Configure magnetometer threshold.
  //   There's only one threshold value for all 3 mag axes.
  //   This is the raw mag value that must be exceeded to
  //   generate an interrupt.
  imu.configMagThs(10000);
  
}

//Stopping point

void setup() {

  delay(200);
    FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);
 }

const unsigned int array0[] = {0x80d06, 0x100,  0x0, 0x0, 0x10000, 0x20000, 0x10000, 0x4, 0x102, 0x100, 0x10001, 0x20801, 0x15370b, 0x468e31, 0x408a27, 0xd2405, 0x300, 0x0, 0x0, 0x0, 0x0
};//holly
const unsigned int array0[] = {0x0, 0x0, 0x0, 0x0, , 0x0, 0x0, 0x10100, 0x40000, 0x20002, 0x4, 0…….
};//snow
