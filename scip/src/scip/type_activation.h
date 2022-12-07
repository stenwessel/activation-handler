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

/**@file   type_activation.h
 * @ingroup TYPEDEFINITIONS
 * @brief  type definitions for activation handlers
 * @author Sten Wessel
 */

/** @defgroup DEFPLUGINS_ACTIVATION Default activation handlers
 *  @ingroup DEFPLUGINS
 *  @brief implementation files (.c files) of the default activation handlers of SCIP
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_TYPE_ACTIVATION_H__
#define __SCIP_TYPE_ACTIVATION_H__


#include "scip/type_scip.h"
#include "scip/type_cons.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SCIP_Activationhdlr SCIP_ACTIVATIONHDLR; /**< activation handler */
typedef struct SCIP_ActivationhdlrData SCIP_ACTIVATIONHDLRDATA;    /**< locally defined activation handler data */
typedef struct SCIP_ActivationSubmatrix SCIP_ACTIVATIONSUBMATRIX;


/** copy method for activation handler plugins (called when SCIP copies plugins)
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - activationhdlr  : the activation handler itself
 */
#define SCIP_DECL_ACTIVATIONCOPY(x) SCIP_RETCODE x (SCIP* scip, SCIP_ACTIVATIONHDLR* activationhdlr)

/** destructor of activation handler to free user data (called when SCIP is exiting)
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - activationhdlr  : the activation handler itself
 */
#define SCIP_DECL_ACTIVATIONFREE(x) SCIP_RETCODE x (SCIP* scip, SCIP_ACTIVATIONHDLR* activationhdlr)

/** initialization method of activation handler (called after problem was transformed)
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - activationhdlr  : the activation handler itself
 */
#define SCIP_DECL_ACTIVATIONINIT(x) SCIP_RETCODE x (SCIP* scip, SCIP_ACTIVATIONHDLR* activationhdlr)

/** deinitialization method of activation handler (called before transformed problem is freed)
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - activationhdlr  : the activation handler itself
 */
#define SCIP_DECL_ACTIVATIONEXIT(x) SCIP_RETCODE x (SCIP* scip, SCIP_ACTIVATIONHDLR* activationhdlr)

/** is active method of activation handler
 *
 *  input:
 *  - scip            : SCIP main data structure
 *  - activationhdlr  : the activation handler itself
 *  - cons            : the constraint to test activation for
 *
 *  output:
 *  - result  : store whether this activation handler is active
 */
#define SCIP_DECL_ACTIVATIONISACTIVE(x) SCIP_RETCODE x (SCIP* scip, SCIP_ACTIVATIONHDLR* activationhdlr, SCIP_CONS* cons, SCIP_Bool* result)

#define SCIP_DECL_ACTIVATIONFINDDATA(x) SCIP_RETCODE x (SCIP* scip, SCIP_ACTIVATIONHDLR* activationhdlr, SCIP_CONS* cons, SCIP_Bool forparentnode, void** activationdata)

#ifdef __cplusplus
}
#endif

#endif
