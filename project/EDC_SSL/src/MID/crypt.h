/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2013-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 2012-11-30          xiaowy                    created
 * ============================================================================
 */

#ifndef __CRYPT_H__
#define __CRYPT_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#include "structDefine.h"

/*---------------------------------------------------------------------------*
 * Include Files        					                           					        *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Macros                                       												 *
 *---------------------------------------------------------------------------*/
	

	
//#define MAX_RSA_MODULUS_BITS 2048
//
//#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
//#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
//#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)


/*---------------------------------------------------------------------------*
 * Local Structures and Typedefs                  							 			 *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Constants                                    							 				 *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Local  Variables				                  							  			 *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * External Variables			                							 			 *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Internal Routine Prototypes	                 							 			 *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * External Routine Prototypes	                 										 *
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Routines' Implementations	                							 			 *
 *---------------------------------------------------------------------------*/

//typedef struct {
//  unsigned short int bits;                     /* length in bits of modulus */
//  unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
//  unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* public exponent */
//} R_RSA_PUBLIC_KEY;
//
//typedef struct {
//  unsigned short int bits;                     /* length in bits of modulus */
//  unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
//  unsigned char publicExponent[MAX_RSA_MODULUS_LEN];     /* public exponent */
//  unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* private exponent */
//  unsigned char prime[2][MAX_RSA_PRIME_LEN];   /* prime factors */
//  unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];     /* exponents for CRT */
//  unsigned char coefficient[MAX_RSA_PRIME_LEN];          /* CRT coefficient */
//} R_RSA_PRIVATE_KEY;

void des(unsigned char *input, unsigned char *output, unsigned char *deskey, int mode);
void Hash(unsigned char *DataIn, unsigned char DataInLen, unsigned char *DataOut);
int RSARecover(unsigned char *pbyModule, unsigned int dwModuleLen, unsigned char *pbyExp, unsigned int dwExpLen, unsigned char *pbyDataIn, unsigned char *pbyDataOut);
int RSAKeyPairGen(R_RSA_PUBLIC_KEY *pPublicKey,R_RSA_PRIVATE_KEY *pPrivateKey,int iProtoKeyBit,int iPubEType);
int RSAKeyPairVerify(R_RSA_PUBLIC_KEY puk,R_RSA_PRIVATE_KEY pvk);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __RSAKEY_H__ */


