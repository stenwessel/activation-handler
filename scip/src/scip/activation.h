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

/**@file   activation.h
 * @ingroup INTERNALAPI
 * @brief  internal methods for activation handlers
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_ACTIVATION_H__
#define __SCIP_ACTIVATION_H__


#include "scip/def.h"
#include "blockmemshell/memory.h"
#include "scip/type_retcode.h"
#include "scip/type_set.h"
#include "scip/type_activation.h"
#include "scip/pub_activation.h"

#ifdef __cplusplus
extern "C" {
#endif

/** copies the given activation handler to a new scip */
SCIP_RETCODE SCIPactivationhdlrCopyInclude(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_SET*             set                 /**< SCIP_SET of SCIP to copy to */
   );

/** creates an activation handler */
SCIP_RETCODE SCIPactivationhdlrCreate(
   SCIP_ACTIVATIONHDLR** activationhdlr,     /**< pointer to activation handler data structure */
   SCIP_SET*             set,                /**< global SCIP settings */
   const char*           name,               /**< name of activation handler */
   const char*           desc,               /**< description of activation handler */
   SCIP_DECL_ACTIVATIONCOPY((*activationcopy)),    /**< copy method of activation handler or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_ACTIVATIONFREE((*activationfree)),    /**< destructor of activation handler */
   SCIP_DECL_ACTIVATIONINIT((*activationinit)),    /**< initialize activation handler */
   SCIP_DECL_ACTIVATIONEXIT((*activationexit)),    /**< deinitialize activation handler */
   SCIP_DECL_ACTIVATIONISACTIVE((*activationisactive)),    /**< is active method of activation handler */
   SCIP_DECL_ACTIVATIONFINDDATA((*activationfindactivationdata)),    /**< TODO */
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata    /**< activation handler local data */
   );

/** calls destructor and frees memory of activation handler */
SCIP_RETCODE SCIPactivationhdlrFree(
   SCIP_ACTIVATIONHDLR** activationhdlr,     /**< pointer to activation handler data structure */
   SCIP_SET*             set                 /**< global SCIP settings */
   );

/** initializes activation handler */
SCIP_RETCODE SCIPactivationhdlrInit(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_SET*             set                 /**< global SCIP settings */
   );

/** calls exit method of activation handler */
SCIP_RETCODE SCIPactivationhdlrExit(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_SET*             set                 /**< global SCIP settings */
   );

/** calls is active method of activation handler */
SCIP_RETCODE SCIPactivationhdlrIsActive(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_CONS*            cons,               /**< constraint to test activation for */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_Bool*            result              /**< pointer to store the result to */
);

/** calls TODO */
SCIP_RETCODE SCIPactivationhdlrFindActivationData(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_CONS*            cons,               /**< constraint to test activation for */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_Bool             forparentnode,
   void*                 activationdata      /**< pointer to store the result to */
);

/** sets copy method of activation handler */
void SCIPactivationhdlrSetCopy(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONCOPY((*activationcopy))     /**< copy method of activation handler or NULL if you don't want to copy your plugin into sub-SCIPs */
   );

/** sets destructor method of activation handler */
void SCIPactivationhdlrSetFree(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONFREE((*activationfree))     /**< destructor of activation handler */
   );

/** sets initialization method of activation handler */
void SCIPactivationhdlrSetInit(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONINIT((*activationinit))     /**< initialize activation handler */
   );

/** sets deinitialization method of activation handler */
void SCIPactivationhdlrSetExit(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONEXIT((*activationexit))     /**< deinitialize activation handler */
   );

/** sets execution method of activation handler */
void SCIPactivationhdlrSetIsActive(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONISACTIVE((*activationisactive))     /**< is active method of activation handler */
   );

/** sets execution method of activation handler */
void SCIPactivationhdlrSetFindActivationData(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONFINDDATA((*activationfindactivationdata))     /**< TODO */
   );

#ifdef __cplusplus
}
#endif

#endif
