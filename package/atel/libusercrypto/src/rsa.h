/* Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __CROS_EC_RSA_H
#define __CROS_EC_RSA_H

//#define CONFIG_RSA_KEY_SIZE 2048 /* default to 2048-bit key length */
#define CONFIG_RSA_KEY_SIZE 4096 /* default to 2048-bit key length */
// Define for 3 otherwise it's 65537
// #define CONFIG_RSA_EXPONENT_3
#include <linux/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <wait.h>
#include "debug.h"
#define RSANUMBYTES ((CONFIG_RSA_KEY_SIZE) / 8)
#define RSANUMWORDS (RSANUMBYTES / sizeof(uint32_t))

/* RSA public key definition */
struct rsa_public_key {
	uint32_t size;
	uint32_t n0inv;           /* -1 / n[0] mod 2^32 */
	uint32_t n[RSANUMWORDS];  /* modulus as little endian array */
	uint32_t rr[RSANUMWORDS]; /* R^2 as little endian array */
};

int rsa_verify(const struct rsa_public_key *key,
	       const uint8_t *signature,
	       const uint8_t *sha,
	       uint32_t *workbuf32);

#endif /* __CROS_EC_RSA_H */
