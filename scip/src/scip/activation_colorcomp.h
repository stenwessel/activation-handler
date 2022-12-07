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

/**@file   activation_suborbitope.h
 * @ingroup ACTIVATIONHDLR
 * @brief  Sub-orbitope activation handler
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_ACTIVATION_COLORCOMP_H__
#define __SCIP_ACTIVATION_COLORCOMP_H__


#include "scip/scip.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@addtogroup ACTIVATIONHDLR
 *
 * @{
 */

/** registers orbitope constraint to use this activation handler
 */
SCIP_EXPORT
SCIP_RETCODE SCIPregisterConsActivationColorComp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_VAR***           matrix,             /**< matrix of color variables (nvertices x ncolors) */
   int                   nvertices,          /**< number of rows in the matrix (number of vertices in the graph) */
   int                   ncolors,            /**< number of columns in the matrix (number of colors) */
   int**                 adjacencies,        /**< array that contains for every vertex an array of adjacent vertices */
   int*                  nadjacencies,       /**< array that contains for every vertex the number of adjacent vertices */
   SCIP_Bool             allcolorpairs,       /**< whether to consider all color pairs or only consecutive-color pairs */
   int                   strategy             /**<  */
   );

/** @} */

/** creates the sub-orbitope activation handler and includes it in SCIP
 *
 * @ingroup ActivationhdlrIncludes
 */
SCIP_EXPORT
SCIP_RETCODE SCIPincludeActivationColorComp(
   SCIP*                 scip                /**< SCIP data structure */
   );

#ifdef __cplusplus
}
#endif

#endif
