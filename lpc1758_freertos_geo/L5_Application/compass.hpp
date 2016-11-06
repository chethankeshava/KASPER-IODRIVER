/*
 * compass.hpp
 *
 *  Created on: Oct 26, 2016
 *      Author: ankit
 */

#ifndef L5_APPLICATION_COMPASS_HPP_
#define L5_APPLICATION_COMPASS_HPP_



#include "i2c2_device.hpp"


#define HMC5883L_ADDRESS              (0x3C)
#define HMC5883L_REG_CONFIG_A         (0x00)
#define HMC5883L_REG_CONFIG_B         (0x01)
#define HMC5883L_REG_MODE             (0x02)
#define HMC5883L_REG_OUT_X_M          (0x03)
#define HMC5883L_REG_OUT_X_L          (0x04)
#define HMC5883L_REG_OUT_Z_M          (0x05)
#define HMC5883L_REG_OUT_Z_L          (0x06)
#define HMC5883L_REG_OUT_Y_M          (0x07)
#define HMC5883L_REG_OUT_Y_L          (0x08)
#define HMC5883L_REG_STATUS           (0x09)
#define HMC5883L_REG_IDENT_A          (0x0A)
#define HMC5883L_REG_IDENT_B          (0x0B)
#define HMC5883L_REG_IDENT_C          (0x0C)




//#define compassi2c compass::getInstance()

class compass : public i2c2_device, public SingletonTemplate<compass>
{
private:

        int xOffset, yOffset;
        float mgPerDigit;
        /// Private constructor of this Singleton class
        compass() : i2c2_device(HMC5883L_ADDRESS)
        {
        }
        friend class SingletonTemplate<compass>;  ///< Friend class used for Singleton Template

        typedef enum
        {
            HMC5883L_SAMPLES_8     = 0b11,
            HMC5883L_SAMPLES_4     = 0b10,
            HMC5883L_SAMPLES_2     = 0b01,
            HMC5883L_SAMPLES_1     = 0b00
        } hmc5883l_samples_t;

        typedef enum
        {
            HMC5883L_DATARATE_75HZ       = 0b110,
            HMC5883L_DATARATE_30HZ       = 0b101,
            HMC5883L_DATARATE_15HZ       = 0b100,
            HMC5883L_DATARATE_7_5HZ      = 0b011,
            HMC5883L_DATARATE_3HZ        = 0b010,
            HMC5883L_DATARATE_1_5HZ      = 0b001,
            HMC5883L_DATARATE_0_75_HZ    = 0b000
        } hmc5883l_dataRate_t;

        typedef enum
        {
            HMC5883L_RANGE_8_1GA     = 0b111,
            HMC5883L_RANGE_5_6GA     = 0b110,
            HMC5883L_RANGE_4_7GA     = 0b101,
            HMC5883L_RANGE_4GA       = 0b100,
            HMC5883L_RANGE_2_5GA     = 0b011,
            HMC5883L_RANGE_1_9GA     = 0b010,
            HMC5883L_RANGE_1_3GA     = 0b001,
            HMC5883L_RANGE_0_88GA    = 0b000
        } hmc5883l_range_t;

        typedef enum
        {
            HMC5883L_IDLE          = 0b10,
            HMC5883L_SINGLE        = 0b01,
            HMC5883L_CONTINOUS     = 0b00
        } hmc5883l_mode_t;


        typedef struct
        {
            float XAxis;
            float YAxis;
            float ZAxis;
        }Vector;

        Vector v;
    public:
        bool init(); 			///< Initializes this device, @returns true if successful

        /**
         * Sets the number on LED display
         * @param num   A number less than 100
         */
        //void setNumber(char num);

        void readNormalize(void);
        void readRaw(void);
        void getHeading(void);
        void setMeasurementMode(hmc5883l_mode_t mode);
        void setRange(hmc5883l_range_t range);
        void setDataRate(hmc5883l_dataRate_t dataRate);
        void setOffset(int xo, int yo);
        void setSamples(hmc5883l_samples_t samples);
        //bool run(void *p);                  				///< The main loop
        /** @} */

};


#endif /* L5_APPLICATION_COMPASS_HPP_ */
