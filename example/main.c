#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "VL53L1X_api.h"

#define SCL_LINE                LINE_ARD_D5
#define SDA_LINE                LINE_ARD_D4
#define INT_LINE                LINE_ARD_D9
#define XSHUT_LINE              LINE_ARD_D10

#define SCL_LINE_MODE           PAL_MODE_ALTERNATE(4) | \
                                PAL_STM32_OSPEED_HIGHEST | \
                                PAL_STM32_OTYPE_OPENDRAIN
#define SDA_LINE_MODE           PAL_MODE_ALTERNATE(4) | \
                                PAL_STM32_OSPEED_HIGHEST | \
                                PAL_STM32_OTYPE_OPENDRAIN

//#define VL52L1X_I2C_ADDRESS     (0x52) // VL52L1X/VL52L0X I2C Address
#define VL52L1X_I2C_ADDRESS     (0x29) // VL52L1X/VL52L0X I2C Address

#define VIRTUAL_COM_TX_LINE         LINE_VCP_TX
#define VIRTUAL_COM_RX_LINE         LINE_VCP_RX
#define VIRTUAL_COM_TX_LINE_MODE    PAL_MODE_ALTERNATE(7) | \
                                    PAL_STM32_OSPEED_MID2 | \
                                    PAL_STM32_PUPDR_FLOATING | \
                                    PAL_STM32_OTYPE_PUSHPULL
#define VIRTUAL_COM_RX_LINE_MODE    PAL_MODE_ALTERNATE(3) | \
                                    PAL_STM32_OSPEED_MID2 | \
                                    PAL_STM32_PUPDR_FLOATING | \
                                    PAL_STM32_OTYPE_PUSHPULL


/* I2C Configuration (ChibiOS HAL I2Cv2)*/
static const I2CConfig i2ccfg = {
    STM32_TIMINGR_PRESC(0x7U) | \
    STM32_TIMINGR_SCLDEL(0x2U) | \
    STM32_TIMINGR_SDADEL(0x4U) | \
    STM32_TIMINGR_SCLH(0x30U) | \
    STM32_TIMINGR_SCLL(0x30U), // Timing Register // System clock for STM32L432KC is 80MHz (from PLL)
    0, // CR1
    0, // CR2
};

/* Serial Configuration for Virtual COM Port */
static SerialConfig serial_cfg = {
    .speed  = 115200,           // Baud rate
    .cr1    = 0,                // No parity, 8-bit data (default)
    .cr2    = 0,                // No specific control settings
    .cr3    = 0                 // No hardware flow control
};


/* Simplified State Machine (Should be replaced with ChibiOS threads/Scheduling) */
void simplified_state_machine(void);


int main(void) {

    /* System initializations. */
    halInit();
    chSysInit();

    /* Configuring I2C SCL and I2C SDA related GPIOs */
    palSetLineMode(SCL_LINE, SCL_LINE_MODE);
    palSetLineMode(SDA_LINE, SDA_LINE_MODE);
    palSetLineMode(INT_LINE, PAL_MODE_OUTPUT_PUSHPULL);
    palSetLineMode(XSHUT_LINE, PAL_MODE_OUTPUT_PUSHPULL);
    
    /* Configure Serial Driver SD2 (USART2) for Virtual COM Port */
    palSetLineMode(VIRTUAL_COM_TX_LINE, VIRTUAL_COM_TX_LINE_MODE);
    palSetLineMode(VIRTUAL_COM_RX_LINE, VIRTUAL_COM_RX_LINE_MODE);
    
    /* Start Serial Driver 2 */
    sdStart(&SD2, &serial_cfg);

    /* Print initial message */
    chprintf((BaseSequentialStream*)&SD2, "Starting VL53L1X I2C Test...\r\n");

    /* Set Interrupt pin LOW, to keep it active" */
    palClearLine(INT_LINE);
    /* Set Shutdown pins HIGH, to Enable VL53L1X */
    palSetLine(XSHUT_LINE);

    // Prepare vl53l1 driver
    VL53L1_SetI2CDriver(&I2CD1);
    VL53L1_SetSerialDriver(&SD2);

    // Start I2C
    i2cStart(&I2CD1, &i2ccfg);

    // Main Thread
    while (true) {

        simplified_state_machine();
        
    }

}


/* Simplified State Machine (Should be replaced with ChibiOS threads/Scheduling) */

enum Simple_State {
    STARTUP,
    MEASURING,
    STOPPING
};

enum Simple_State test_state = STARTUP;
const uint16_t max_measures = 5;
uint16_t measures = 0;

void simplified_state_machine(void){
    
    volatile VL53L1X_ERROR status = -128;

    switch (test_state) {
        case STARTUP:
            
            // Wait for device booted (1)
            uint8_t boot_state = 0;
            status = -128;
            while(boot_state == 0){
                chprintf((BaseSequentialStream*)&SD2, "Checking Boot State...\r\n");
                status = VL53L1X_BootState(VL52L1X_I2C_ADDRESS, &boot_state);
                //chprintf((BaseSequentialStream*)&SD2, "I2C Status: %u\r\n", status);
                chprintf((BaseSequentialStream*)&SD2, "Boot State: %u\r\n", boot_state);
                chThdSleepMilliseconds(200);
            }

            // Read sensor ID (extra step)
            volatile uint16_t sensorId = 0;
            status = -128;
            chprintf((BaseSequentialStream*)&SD2, "Reading Sensor ID...\r\n");
            status = VL53L1X_GetSensorId(VL52L1X_I2C_ADDRESS, &sensorId);
            chprintf((BaseSequentialStream*)&SD2, "Sensor ID: 0x%04x\r\n", sensorId);
            //chprintf((BaseSequentialStream*)&SD2, "I2C Status: %u\r\n", status);
            chThdSleepMilliseconds(200);

            // Sensor init (2)
            status = -128;
            chprintf((BaseSequentialStream*)&SD2, "Initializing Sensor...\r\n");
            status = VL53L1X_SensorInit(VL52L1X_I2C_ADDRESS);
            //chprintf((BaseSequentialStream*)&SD2, "I2C Status: %u\r\n", status);
            chThdSleepMilliseconds(200);

            // Optional functions call (3)
            // none..

            // Start ranging (4)
            status = -128;
            chprintf((BaseSequentialStream*)&SD2, "Start Ranging...\r\n");
            status = VL53L1X_StartRanging(VL52L1X_I2C_ADDRESS);
            //chprintf((BaseSequentialStream*)&SD2, "I2C Status: %u\r\n", status);
            chThdSleepMilliseconds(200);

            test_state = MEASURING;

            break;

        case MEASURING:
            
            // Wait for data ready (5) (Polling) // Optional: Wait for interrupt (6)
            uint8_t is_data_ready = 0;
            status = -128;
            while(is_data_ready == 0){
                chprintf((BaseSequentialStream*)&SD2, "Checking if data is ready...\r\n");
                status = VL53L1X_CheckForDataReady(VL52L1X_I2C_ADDRESS, &is_data_ready);
                //chprintf((BaseSequentialStream*)&SD2, "I2C Status: %u\r\n", status);
                chprintf((BaseSequentialStream*)&SD2, "Is Data ready: %u\r\n", is_data_ready);
                chThdSleepMilliseconds(200);
            }

            // Get ranging data (7)
            uint16_t distance_mm = 0;
            status = -128;
            chprintf((BaseSequentialStream*)&SD2, "Checking if data is ready...\r\n");
            status = VL53L1X_GetDistance(VL52L1X_I2C_ADDRESS, &distance_mm);
            //chprintf((BaseSequentialStream*)&SD2, "I2C Status: %u\r\n", status);
            chprintf((BaseSequentialStream*)&SD2, "Distance: %u [mm]\r\n", distance_mm);
            chThdSleepMilliseconds(200);

            // Clear interrupt (8) (optional when polling???)
            status = -128;
            chprintf((BaseSequentialStream*)&SD2, "Clear interrupt...\r\n");
            status = VL53L1X_ClearInterrupt(VL52L1X_I2C_ADDRESS);
            //chprintf((BaseSequentialStream*)&SD2, "I2C Status: %u\r\n", status);
            chThdSleepMilliseconds(200);

            measures = measures + 1;

            if(measures >= max_measures){
                test_state = STOPPING;
                measures = 0;
            }

            break;

        case STOPPING:
            
            // Start ranging (4)
            status = -128;
            chprintf((BaseSequentialStream*)&SD2, "Stop Ranging...\r\n");
            status = VL53L1X_StopRanging(VL52L1X_I2C_ADDRESS);
            //chprintf((BaseSequentialStream*)&SD2, "I2C Status: %u\r\n", status);
            chThdSleepMilliseconds(200);

            // Starting over again
            test_state = STARTUP;
            chprintf((BaseSequentialStream*)&SD2, "Starting over again...\r\n");

            break;

        default:

            chprintf((BaseSequentialStream*)&SD2, "Invalid Simple_State for State Machine\r\n");
            break;
    }

}
