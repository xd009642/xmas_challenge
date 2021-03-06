/* SD/MMC File System Library
 * Copyright (c) 2016 Neil Thiessen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SD_FILE_SYSTEM_H
#define SD_FILE_SYSTEM_H

#include "mbed.h"
#include "FATFileSystem.h"

#include "SD_DISCO_F469NI.h"
/** SDFileSystem class.
 *  Used for creating a virtual file system for accessing SD/MMC cards via SPI.
 *
 * Example:
 * @code
 * #include "mbed.h"
 * #include "SDFileSystem.h"
 *
 * //Create an SDFileSystem object
 * SDFileSystem sd(p5, p6, p7, p20, "sd");
 *
 * int main()
 * {
 *     //Mount the filesystem
 *     sd.mount();
 *
 *     //Perform a write test
 *     printf("\nWriting to SD card...");
 *     FILE *fp = fopen("/sd/sdtest.txt", "w");
 *     if (fp != NULL) {
 *         fprintf(fp, "We're writing to an SD card!");
 *         fclose(fp);
 *         printf("success!\n");
 *     } else {
 *         printf("failed!\n");
 *     }
 *
 *     //Perform a read test
 *     printf("Reading from SD card...");
 *     fp = fopen("/sd/sdtest.txt", "r");
 *     if (fp != NULL) {
 *         char c = fgetc(fp);
 *         if (c == 'W')
 *             printf("success!\n");
 *         else
 *             printf("incorrect char (%c)!\n", c);
 *         fclose(fp);
 *     } else {
 *         printf("failed!\n");
 *     }
 *
 *     //Unmount the filesystem
 *     sd.unmount();
 * }
 * @endcode
 */
class SDFileSystem : public FATFileSystem
{
public:
    /** Represents the different card detect switch types
     */
    enum SwitchType {
        SWITCH_NONE,    /**< No card detect switch (assumes socket is always occupied) */
        SWITCH_POS_NO,  /**< Switch shorts to VDD when the socket is occupied (positive logic, normally open) */
        SWITCH_POS_NC,  /**< Switch shorts to VDD when the socket is empty (positive logic, normally closed) */
        SWITCH_NEG_NO,  /**< Switch shorts to GND when the socket is occupied (negative logic, normally open) */
        SWITCH_NEG_NC   /**< Switch shorts to GND when the socket is empty (negative logic, normally closed) */
    };

    /** Represents the different SD/MMC card types
     */
    enum CardType {
        CARD_SD = 1,        /**< Standard capacity SD card */
        CARD_SDHC= 2,      /**< High capacity SD card */
        CARD_MMC = 3,       /**< MMC card */
        CARD_NONE = 8,      /**< No card is present */
        CARD_UNKNOWN = 9    /**< Unknown or unsupported card */
    };

    /** Create a virtual file system for accessing SD/MMC cards via SPI
     *
     * @param mosi The SPI data out pin.
     * @param miso The SPI data in pin.
     * @param sclk The SPI clock pin.
     * @param cs The SPI chip select pin.
     * @param name The name used to access the virtual filesystem.
     * @param cd The card detect pin.
     * @param cdtype The type of card detect switch.
     * @param hz The SPI bus frequency (defaults to 1MHz).
     */
    SDFileSystem(const char* name);

    /** Determine whether or not a card is present
     *
     * @returns
     *   'true' if a card is present,
     *   'false' if no card is present.
     */
    bool card_present();

    /** Get the detected SD/MMC card type
     *
     * @returns The detected card type as a CardType enum.
     *
     * @note Valid after the filesystem has been mounted.
     */
    SDFileSystem::CardType card_type();

    /** Get whether or not 16-bit frames are enabled for data read/write operations
     *
     * @returns
     *   'true' if 16-bit frames will be used during data read/write operations,
     *   'false' if 8-bit frames will be used during data read/write operations.
     */
    bool large_frames();

    /** Set whether or not 16-bit frames are enabled for data read/write operations
     *
     * @param enabled Whether or not 16-bit frames are enabled for data read/write operations.
     */
    void large_frames(bool enabled);

    /** Get whether or not write validation is enabled for data write operations
     *
     * @returns
     *   'true' if data writes will be verified using CMD13,
     *   'false' if data writes will not be verified.
     */
    bool write_validation();

    /** Set whether or not write validation is enabled for data write operations
     *
     * @param enabled Whether or not write validation is enabled for data write operations.
     */
    void write_validation(bool enabled);

    virtual int unmount();
    virtual int disk_initialize();
    virtual int disk_status();
    virtual int disk_read(uint8_t* buffer, uint32_t sector, uint32_t count);
    virtual int disk_write(const uint8_t* buffer, uint32_t sector, uint32_t count);
    virtual int disk_sync();
    virtual uint32_t disk_sectors();

private:
   
    //Member variables
    Timer m_Timer;
    SD_DISCO_F469NI sd;
    SDFileSystem::CardType m_CardType;
    bool m_LargeFrames;
    bool m_WriteValidation;
    int m_Status;

    //Internal methods
    void onCardRemoval();
    void checkSocket();
    bool waitReady(int timeout);
    bool readData(char* buffer, int length);
    char writeData(const char* buffer, char token);
    bool readBlock(char* buffer, unsigned int lba);
    bool readBlocks(char* buffer, unsigned int lba, unsigned int count);
    bool writeBlock(const char* buffer, unsigned int lba);
    bool writeBlocks(const char* buffer, unsigned int lba, unsigned int count);
};

#endif
