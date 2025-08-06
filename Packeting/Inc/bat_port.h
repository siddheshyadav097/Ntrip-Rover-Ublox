/**
 *  \file bat_port.h
 *  \brief Header file for battery sense port initialization and battery level & charging status query.
 */

#ifndef __BAT_PORT_H
#define __BAT_PORT_H


 /*----includes-----*/
#include "lib_port.h"
//#include "mcu_port.h"

//#define VREFINT_CAL_ADDR    0x1FFFF7BA  // 2 Byte at this address is VRefInt @3.3V/30°C  

#define BATTERY_PGOOD_Pin       GPIO_PIN_12    //PGOOD pin //#define BATTERY_CHG_Pin GPIO_PIN_0
#define BATTERY_CHG_GPIO_Port   GPIOC

#define ADC_CONVERTED_DATA_BUFFER_SIZE   ((uint32_t)  4)

/* Definitions of data related to this example */
  /* Full-scale digital value with a resolution of 12 bits (voltage range     */
  /* determined by analog voltage references Vref+ and Vref-,                 */
  /* refer to reference manual).                                              */
  #define DIGITAL_SCALE_12BITS             ((uint32_t) 0xFFF)

/* Init variable out of ADC expected conversion data range */
  #define VAR_CONVERTED_DATA_INIT_VALUE    (DIGITAL_SCALE_12BITS + 1)
////#define SUPPLY_VOLTAGE_ADC      ADC2

////#define BATSENSE_ADC_CHANNEL    ADC_CHANNEL_11  //PC5
////#define EXTSUPPLY_ADC_CHANNEL   ADC_CHANNEL_5//ADC_CHANNEL_5   //PC4


 /*----typedefs-----*/

 /*----public function declarations-----*/
void MX_DMA_Init(void);
void InitBattChargePin(void);
void InitSupplySenseADCPorts(void);
void TriggerSupplyADC(void);
uint16_t ReadAdcBatValue(void);
uint16_t ReadAdcSupplyValue(void);
uint16_t ReadAdcVrefIntValue(void);
uint16_t ReadTempSensorValue(void);
//uint16_t ReadAnalogIp2Value(void);
BOOL IsChargerConnected(void);
BOOL IsChargingFull(void);
///void ReadSupplyChannel(void);

#endif
