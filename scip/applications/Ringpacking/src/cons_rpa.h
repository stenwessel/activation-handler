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

/**@file   cons_rpa.h
 * @ingroup CONSHDLRS
 * @brief  constraint handler for ringpacking
 * @author Benjamin Mueller
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_CONS_RPA_H__
#define __SCIP_CONS_RPA_H__


#include "scip/scip.h"

#ifdef __cplusplus
extern "C" {
#endif

/** creates the handler for ringpacking
 *
 * @ingroup ConshdlrIncludes
 * */
SCIP_RETCODE SCIPincludeConshdlrRpa(
   SCIP*                 scip                /**< SCIP data structure */
   );

#ifdef __cplusplus
}
#endif

#endif
