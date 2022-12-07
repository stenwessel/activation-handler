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

/**@file   struct_activation.h
 * @ingroup INTERNALAPI
 * @brief  datastructures for activation handlers
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_STRUCT_ACTIVATION_H__
#define __SCIP_STRUCT_ACTIVATION_H__


#include "scip/def.h"
#include "scip/type_activation.h"
#include "symmetry/type_symmetry.h"

#ifdef __cplusplus
extern "C" {
#endif

/** activation handler data */
struct SCIP_Activationhdlr
{
   char*                 name;               /**< name of activation handler */
   char*                 desc;               /**< description of activation handler */
   SCIP_DECL_ACTIVATIONCOPY((*activationcopy));    /**< copy method of activation handler or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_ACTIVATIONFREE((*activationfree));    /**< destructor of activation handler */
   SCIP_DECL_ACTIVATIONINIT((*activationinit));    /**< initialize activation handler */
   SCIP_DECL_ACTIVATIONEXIT((*activationexit));    /**< deinitialize activation handler */
   SCIP_DECL_ACTIVATIONISACTIVE((*activationisactive));    /**< is active method of activation handler */
   SCIP_DECL_ACTIVATIONFINDDATA((*activationfindactivationdata));    /**< TODO */
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;       /**< activation handler local data */
   SCIP_Bool             initialized;        /**< is activation handler initialized? */
};

struct SCIP_ActivationSubmatrix
{
   int*                  rows;               /**< indices of the rows selected in this submatrix */
   int*                  cols;               /**< indices of the columns selected in this submatrix */
   int                   nrows;              /**< number of selected rows */
   int                   ncols;              /**< number of selected cols */
   SCIP_ORBITOPETYPE     orbitopetype;       /**< type of orbitope for this submatrix */
   struct SCIP_ActivationSubmatrix* next;    /**< next submatrix in the list */
};

struct Subregion {
   int x;
   int y;
   int width;
   int height;
   struct Subregion* next;
};

#ifdef __cplusplus
}
#endif

#endif
