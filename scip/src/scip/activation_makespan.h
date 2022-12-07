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

/**@file   activation_makespan.h
 * @ingroup ACTIVATIONHDLR
 * @brief  Makespan activation handler
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_ACTIVATION_MAKESPAN_H__
#define __SCIP_ACTIVATION_MAKESPAN_H__


#include "scip/scip.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@addtogroup ACTIVATIONHDLR
 *
 * @{
 */

/** adds variables to the makespan activation handler */
SCIP_EXPORT
SCIP_RETCODE SCIPregisterConsActivationMakespan(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_VAR***           matrix,             /**< matrix of scheduling variables */
   int*                  jobtimes,           /**< job processing times */
   int                   nmachines,          /**< number of machines (columns in the matrix) */
   int                   njobs               /**< number of jobs (rows in the matrix) */
   );

/** @} */

/** creates the makespan activation handler and includes it in SCIP
 *
 * @ingroup ActivationhdlrIncludes
 */
SCIP_EXPORT
SCIP_RETCODE SCIPincludeActivationMakespan(
   SCIP*                 scip                /**< SCIP data structure */
   );

#ifdef __cplusplus
}
#endif

#endif
