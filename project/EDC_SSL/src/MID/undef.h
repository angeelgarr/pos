/*
 * ============================================================================
 * COPYRIGHT
 *               PAX TECHNOLOGY, Inc. PROPRIETARY INFORMATION
 *   This software is supplied under the terms of a license agreement or
 *   nondisclosure agreement with PAX  Technology, Inc. and may not be copied
 *   or disclosed except in accordance with the terms in that agreement.
 *      Copyright (C) 2012-? PAX Technology, Inc. All rights reserved.
 * Description:
 * Revision History:
 * Date                  Author                 Action
 * 2012-12-4          wenjf                     created
 * ============================================================================
 */

#ifndef UNDEF_FUNC_H_
#define UNDEF_FUNC_H_


#ifdef  ScrInit
#undef  ScrInit
#endif

#ifdef  Lcdprintf
#undef  Lcdprintf
#endif

#ifdef open
#undef open
#endif

#ifdef truncate
#undef truncate
#endif

#ifdef read
#undef read
#endif

#ifdef write
#undef write
#endif

#ifdef close
#undef close
#endif

#ifdef remove
#undef remove
#endif

#endif
