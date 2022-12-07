/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2021 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not visit scipopt.org.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   pub_activation.h
 * @ingroup PUBLICCOREAPI
 * @brief  public methods for activation handlers
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_PUB_ACTIVATION_H__
#define __SCIP_PUB_ACTIVATION_H__


#include "scip/def.h"
#include "scip/type_activation.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@addtogroup PublicActivationHandlerMethods
 *
 * @{
 */

/** gets user data of activation handler */
SCIP_EXPORT
SCIP_ACTIVATIONHDLRDATA* SCIPactivationhdlrGetData(
   SCIP_ACTIVATIONHDLR*  activationhdlr      /**< activation handler */
   );

/** sets user data of activation handler; user has to free old data in advance! */
SCIP_EXPORT
void SCIPactivationhdlrSetData(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata    /**< new activation handler user data */
   );

/** gets name of activation handler */
SCIP_EXPORT
const char* SCIPactivationhdlrGetName(
   SCIP_ACTIVATIONHDLR*  activationhdlr      /**< activation handler */
   );

/** gets description of activation handler */
SCIP_EXPORT
const char* SCIPactivationhdlrGetDesc(
   SCIP_ACTIVATIONHDLR*  activationhdlr      /**< activation handler */
   );

/** is activation handler initialized? */
SCIP_EXPORT
SCIP_Bool SCIPactivationhdlrIsInitialized(
   SCIP_ACTIVATIONHDLR*  activationhdlr      /**< activation handler */
   );

/** @} */

#ifdef __cplusplus
}
#endif

#endif
