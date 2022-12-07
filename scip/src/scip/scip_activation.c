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

/**@file   scip_activation.c
 * @ingroup OTHER_CFILES
 * @brief  public methods for activation handler plugins
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "scip/debug.h"
#include "scip/activation.h"
#include "scip/scip_activation.h"
#include "scip/set.h"
#include "scip/struct_scip.h"

SCIP_RETCODE SCIPincludeActivationhdlr(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           name,               /**< name of activation handler */
   const char*           desc,               /**< description of activation handler */
   SCIP_DECL_ACTIVATIONCOPY((*activationcopy)),    /**< copy method of activation handler or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_ACTIVATIONFREE((*activationfree)),    /**< destructor of activation handler */
   SCIP_DECL_ACTIVATIONINIT((*activationinit)),    /**< initialize activation handler */
   SCIP_DECL_ACTIVATIONEXIT((*activationexit)),    /**< deinitialize activation handler */
   SCIP_DECL_ACTIVATIONISACTIVE((*activationisactive)),    /**< is active method of activation handler */
   SCIP_DECL_ACTIVATIONFINDDATA((*activationfindactivationdata)),    /**< TODO */
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata    /**< activation handler local data */
   )
{
   SCIP_ACTIVATIONHDLR* activationhdlr;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPincludeActivationhdlr", TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   // Check whether the activation handler is already present
   if( SCIPfindActivationhdlr(scip, name) != NULL )
   {
      SCIPerrorMessage("activation handler <%s> already included.\n", name);
      return SCIP_INVALIDDATA;
   }

   SCIP_CALL( SCIPactivationhdlrCreate(&activationhdlr, scip->set, name, desc, activationcopy, activationfree,
         activationinit, activationexit, activationisactive, activationfindactivationdata, activationhdlrdata) );
   SCIP_CALL( SCIPsetIncludeActivationhdlr(scip->set, activationhdlr) );

   return SCIP_OKAY;
}

/** sets copy method of activation handler */
SCIP_RETCODE SCIPsetActivationhdlrCopy(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONCOPY((*activationcopy))    /**< copy method of activation handler or NULL if you don't want to copy your plugin into sub-SCIPs */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetActivationhdlrCopy", TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert(activationhdlr != NULL);

   SCIPactivationhdlrSetCopy(activationhdlr, activationcopy);

   return SCIP_OKAY;
}

/** sets destructor method of activation handler */
SCIP_RETCODE SCIPsetActivationhdlrFree(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONFREE((*activationfree))     /**< destructor of activation handler */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetActivationhdlrFree", TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert(activationhdlr != NULL);

   SCIPactivationhdlrSetFree(activationhdlr, activationfree);

   return SCIP_OKAY;
}

/** sets initialization method of activation handler */
SCIP_RETCODE SCIPsetActivationhdlrInit(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONINIT((*activationinit))     /**< initialize activation handler */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetActivationhdlrInit", TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert(activationhdlr != NULL);

   SCIPactivationhdlrSetInit(activationhdlr, activationinit);

   return SCIP_OKAY;
}

/** sets deinitialization method of activation handler */
SCIP_RETCODE SCIPsetActivationhdlrExit(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONEXIT((*activationexit))     /**< deinitialize activation handler */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetActivationhdlrExit", TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert(activationhdlr != NULL);

   SCIPactivationhdlrSetExit(activationhdlr, activationexit);

   return SCIP_OKAY;
}

/** sets is active method of activation handler */
SCIP_RETCODE SCIPsetActivationhdlrIsActive(
      SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONISACTIVE((*activationisactive))     /**< is active method of activation handler */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetActivationhdlrIsActive", TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert(activationhdlr != NULL);

   SCIPactivationhdlrSetIsActive(activationhdlr, activationisactive);

   return SCIP_OKAY;
}

/** sets TODO */
SCIP_RETCODE SCIPsetActivationhdlrFindActivationData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_DECL_ACTIVATIONFINDDATA((*activationfindactivationdata))     /**< TODO */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetActivationhdlrFindActivationData", TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert(activationhdlr != NULL);

   SCIPactivationhdlrSetFindActivationData(activationhdlr, activationfindactivationdata);

   return SCIP_OKAY;
}

/** returns the activation handler of the given name, or NULL if not existing */
SCIP_EXPORT
SCIP_ACTIVATIONHDLR* SCIPfindActivationhdlr(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           name                /**< name of activation handler */
   )
{
   assert(scip != NULL);
   assert(scip->set != NULL);
   assert(name != NULL);

   return SCIPsetFindActivationhdlr(scip->set, name);
}