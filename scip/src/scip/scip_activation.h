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

/**@file   scip_activation.h
 * @ingroup PUBLICCOREAPI
 * @brief  public methods for activation handler plugins
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_SCIP_ACTIVATION_H__
#define __SCIP_SCIP_ACTIVATION_H__


#include "scip/def.h"
#include "scip/type_activation.h"
#include "scip/type_retcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@addtogroup PublicActivationHandlerMethods
 *
 * @{
 */

/** creates an activation handler and includes it in SCIP.
 */
SCIP_EXPORT
SCIP_RETCODE SCIPincludeActivationhdlr(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           name,               /**< name of activation handler */
   const char*           desc,               /**< description of activation handler */
   SCIP_DECL_ACTIVATIONCOPY((*activationcopy)),    /**< copy method of activation handler or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_ACTIVATIONFREE((*activationfree)),    /**< destructor of activation handler */
   SCIP_DECL_ACTIVATIONINIT((*activationinit)),    /**< initialize activation handler */
   SCIP_DECL_ACTIVATIONEXIT((*activationexit)),    /**< deinitialize activation handler */
   SCIP_DECL_ACTIVATIONISACTIVE((*activationisactive)),     /**< is active method of activation handler */
   SCIP_DECL_ACTIVATIONFINDDATA((*activationfindactivationdata)),    /**< TODO */
SCIP_ACTIVATIONHDLRDATA* activationhdlrdata    /**< activation handler local data */
   );

/** sets copy method of activation handler */
SCIP_EXPORT
SCIP_RETCODE SCIPsetActivationhdlrCopy(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONCOPY((*activationcopy))    /**< copy method of activation handler or NULL if you don't want to copy your plugin into sub-SCIPs */
   );

/** sets destructor method of activation handler */
SCIP_EXPORT
SCIP_RETCODE SCIPsetActivationhdlrFree(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONFREE((*activationfree))     /**< destructor of activation handler */
   );

/** sets initialization method of activation handler */
SCIP_EXPORT
SCIP_RETCODE SCIPsetActivationhdlrInit(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONINIT((*activationinit))     /**< initialize activation handler */
   );

/** sets deinitialization method of activation handler */
SCIP_EXPORT
SCIP_RETCODE SCIPsetActivationhdlrExit(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONEXIT((*activationexit))     /**< deinitialize activation handler */
   );

/** sets is active method of activation handler */
SCIP_EXPORT
SCIP_RETCODE SCIPsetActivationhdlrIsActive(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONISACTIVE((*activationisactive))     /**< is active method of activation handler */
   );

/** sets TODO */
SCIP_EXPORT
SCIP_RETCODE SCIPsetActivationhdlrFindActivationData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONFINDDATA((*activationfindactivationdata))     /**< TODO */
   );

/** returns the activation handler of the given name, or NULL if not existing */
SCIP_EXPORT
SCIP_ACTIVATIONHDLR* SCIPfindActivationhdlr(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           name                /**< name of activation handler */
   );


/** @} */

#ifdef __cplusplus
}
#endif

#endif
