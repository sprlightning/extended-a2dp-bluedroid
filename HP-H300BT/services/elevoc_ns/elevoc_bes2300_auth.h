/*
 *  elevoc_auth.h
 *
 *  Created on: 2021年2月20日
 *      Author: dehong.liu
 */

#ifndef ELEVOC_BES2300_AUTH_H_
#define ELEVOC_BES2300_AUTH_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    ELEVOC_KEY_LEN_ERR = -7,        /**< The key len is invalid. */
    ELEVOC_NULL_PTR = -6,           /**< The key ptr is null. */
    ELEVOC_INVALID_PARAMETER = -5,  /**< The user parameter is invalid. */
    ELEVOC_ITEM_NOT_FOUND = -4,     /**< The data item wasn't found by the NVDM. */
    ELEVOC_INSUFFICIENT_SPACE = -3, /**< No space is available in the flash. */
    ELEVOC_INCORRECT_CHECKSUM = -2, /**< The NVDM found a checksum error when reading the data item or the buffer has changed when writing the data item. */
    ELEVOC_ERROR = -1,              /**< An unknown error occurred. */
    ELEVOC_STATUS_OK = 0,           /**< The operation was successful. */
}ELEVOC_ADDR_AUTH_STATE_E;

typedef enum
{
    AUTH_VERIFY_OK = 0,     // Auth verify ok
    AUTH_PACK_ERROR = 1,    // Package len/mem/read is wrong
    AUTH_BT_READ_ERROR = 2, // BT address read is wrong
    AUTH_TAG_ERROR = 3,     // TAG is not match
    AUTH_WRONG = 4,         // BT address beyond limited address
    AUTH_CRC_ERROR = 5,     // Package crc is wrong
}ELEVOC_ADDR_VERIFY_STATE_E;

/** elevoc authorized initialization function
 *
 * @param none
*/
extern ELEVOC_ADDR_VERIFY_STATE_E elevoc_auth_init(void);

/** elevoc authorized key write in OTP section
 *
 * @param key  	  	authorized key data (128 bytes)
 * @param len		authorized key data length (128)
*/
extern ELEVOC_ADDR_AUTH_STATE_E elevoc_encryption_key_write(uint8_t *key, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* ELEVOC_AUTH_H_ */
