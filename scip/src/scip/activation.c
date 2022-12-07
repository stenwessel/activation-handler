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

/**@file   activation.c
 * @ingroup OTHER_CFILES
 * @brief  methods and datastructures for activation handlers
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "scip/set.h"
#include "scip/activation.h"

#include "scip/struct_activation.h"

/** copies the given activation handler to a new scip */
SCIP_RETCODE SCIPactivationhdlrCopyInclude(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_SET*             set                 /**< SCIP_SET of SCIP to copy to */
   )
{
   assert(activationhdlr != NULL);
   assert(set != NULL);
   assert(set->scip != NULL);

   if( activationhdlr->activationcopy != NULL )
   {
      SCIPsetDebugMsg(set, "including activation handler %s in subscip %p\n", SCIPactivationhdlrGetName(activationhdlr),
            (void*)set->scip);
      SCIP_CALL( activationhdlr->activationcopy(set->scip, activationhdlr) );
   }
   return SCIP_OKAY;
}

/** internal method for creating an activation handler */
static
SCIP_RETCODE doActivationHdlrCreate(
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
)
{
   assert(activationhdlr != NULL);
   assert(name != NULL);
   assert(desc != NULL);

   SCIP_ALLOC( BMSallocMemory(activationhdlr) );
   BMSclearMemory(*activationhdlr);

   SCIP_ALLOC( BMSduplicateMemoryArray(&(*activationhdlr)->name, name, strlen(name) + 1) );
   SCIP_ALLOC( BMSduplicateMemoryArray(&(*activationhdlr)->desc, desc, strlen(desc) + 1) );
   (*activationhdlr)->activationcopy = activationcopy;
   (*activationhdlr)->activationfree = activationfree;
   (*activationhdlr)->activationinit = activationinit;
   (*activationhdlr)->activationexit = activationexit;
   (*activationhdlr)->activationisactive = activationisactive;
   (*activationhdlr)->activationfindactivationdata = activationfindactivationdata;
   (*activationhdlr)->activationhdlrdata = activationhdlrdata;
   (*activationhdlr)->initialized = FALSE;

   return SCIP_OKAY;
}

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
   )
{
   assert(activationhdlr != NULL);
   assert(name != NULL);
   assert(desc != NULL);

   SCIP_CALL_FINALLY( doActivationHdlrCreate(activationhdlr, set, name, desc, activationcopy, activationfree,
      activationinit, activationexit, activationisactive, activationfindactivationdata, activationhdlrdata),
      (void) SCIPactivationhdlrFree(activationhdlr, set) );

   return SCIP_OKAY;
}

/** calls destructor and frees memory of activation handler */
SCIP_RETCODE SCIPactivationhdlrFree(
   SCIP_ACTIVATIONHDLR** activationhdlr,     /**< pointer to activation handler data structure */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(activationhdlr != NULL);
   if( *activationhdlr == NULL )
      return SCIP_OKAY;
   assert(!(*activationhdlr)->initialized);
   assert(set != NULL);

   /* call destructor of propagator */
   if( (*activationhdlr)->activationfree != NULL )
   {
      SCIP_CALL( (*activationhdlr)->activationfree(set->scip, *activationhdlr) );
   }

   BMSfreeMemoryArrayNull(&(*activationhdlr)->desc);
   BMSfreeMemoryArrayNull(&(*activationhdlr)->name);
   BMSfreeMemory(activationhdlr);

   return SCIP_OKAY;
}

/** initializes activation handler */
SCIP_RETCODE SCIPactivationhdlrInit(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(activationhdlr != NULL);
   assert(set != NULL);

   if( activationhdlr->initialized )
   {
      SCIPerrorMessage("activation handler <%s> already initialized\n", activationhdlr->name);
      return SCIP_INVALIDCALL;
   }

   if( activationhdlr->activationinit != NULL )
   {
      SCIP_CALL( activationhdlr->activationinit(set->scip, activationhdlr) );
   }
   activationhdlr->initialized = TRUE;

   return SCIP_OKAY;
}

/** calls exit method of activation handler */
SCIP_RETCODE SCIPactivationhdlrExit(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(activationhdlr != NULL);
   assert(set != NULL);

   if( !activationhdlr->initialized )
   {
      SCIPerrorMessage("activation handler <%s> not initialized\n", activationhdlr->name);
      return SCIP_INVALIDCALL;
   }

   if( activationhdlr->activationexit != NULL )
   {
      SCIP_CALL( activationhdlr->activationexit(set->scip, activationhdlr) );
   }
   activationhdlr->initialized = FALSE;

   return SCIP_OKAY;
}

/** calls execution method of activation handler */
SCIP_RETCODE SCIPactivationhdlrIsActive(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_CONS*            cons,               /**< constraint to test activation for */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_Bool*            result              /**< pointer to store the result to */
   )
{
   assert(activationhdlr != NULL);
   assert(activationhdlr->activationisactive != NULL);
   assert(set != NULL);

   /* call external activation method */
   SCIP_CALL( activationhdlr->activationisactive(set->scip, activationhdlr, cons, result) );

   return SCIP_OKAY;
}

/** calls TODO */
SCIP_RETCODE SCIPactivationhdlrFindActivationData(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_CONS*            cons,               /**< constraint to test activation for */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_Bool             forparentnode,
   void*                 activationdata      /**< pointer to store the result to */
)
{
   assert(activationhdlr != NULL);
   assert(activationhdlr->activationfindactivationdata != NULL);
   assert(set != NULL);

   SCIP_CALL( activationhdlr->activationfindactivationdata(set->scip, activationhdlr, cons, forparentnode, activationdata) );

   return SCIP_OKAY;
}

/** gets user data of activation handler */
SCIP_ACTIVATIONHDLRDATA* SCIPactivationhdlrGetData(
   SCIP_ACTIVATIONHDLR*  activationhdlr      /**< activation handler */
   )
{
   assert(activationhdlr != NULL);

   return activationhdlr->activationhdlrdata;
}

/** sets user data of activation handler; user has to free old data in advance! */
void SCIPactivationhdlrSetData(
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata    /**< new activation handler user data */
   )
{
   assert(activationhdlr != NULL);

   activationhdlr->activationhdlrdata = activationhdlrdata;
}

/** sets copy method of activation handler */
void SCIPactivationhdlrSetCopy(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONCOPY((*activationcopy))     /**< copy method of activation handler or NULL if you don't want to copy your plugin into sub-SCIPs */
   )
{
   assert(activationhdlr != NULL);

   activationhdlr->activationcopy = activationcopy;
}

/** sets destructor method of activation handler */
void SCIPactivationhdlrSetFree(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONFREE((*activationfree))     /**< destructor of activation handler */
   )
{
   assert(activationhdlr != NULL);

   activationhdlr->activationfree = activationfree;
}

/** sets initialization method of activation handler */
void SCIPactivationhdlrSetInit(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONINIT((*activationinit))     /**< initialize activation handler */
   )
{
   assert(activationhdlr != NULL);

   activationhdlr->activationinit = activationinit;
}

/** sets deinitialization method of activation handler */
void SCIPactivationhdlrSetExit(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONEXIT((*activationexit))     /**< deinitialize activation handler */
   )
{
   assert(activationhdlr != NULL);

   activationhdlr->activationexit = activationexit;
}

/** sets execution method of activation handler */
void SCIPactivationhdlrSetIsActive(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONISACTIVE((*activationisactive))     /**< is active method of activation handler */
   )
{
   assert(activationhdlr != NULL);

   activationhdlr->activationisactive = activationisactive;
}

/** sets execution method of activation handler */
void SCIPactivationhdlrSetFindActivationData(
   SCIP_ACTIVATIONHDLR*  activationhdlr,        /**< activation handler */
   SCIP_DECL_ACTIVATIONFINDDATA((*activationfindactivationdata))     /**< TODO */
   )
{
   assert(activationhdlr != NULL);

   activationhdlr->activationfindactivationdata = activationfindactivationdata;
}

/** gets name of activation handler */
const char* SCIPactivationhdlrGetName(
      SCIP_ACTIVATIONHDLR*  activationhdlr      /**< activation handler */
   )
{
   assert(activationhdlr != NULL);

   return activationhdlr->name;
}

/** gets description of activation handler */
const char* SCIPactivationhdlrGetDesc(
      SCIP_ACTIVATIONHDLR*  activationhdlr      /**< activation handler */
   )
{
   assert(activationhdlr != NULL);

   return activationhdlr->desc;
}

/** is activation handler initialized? */
SCIP_Bool SCIPactivationhdlrIsInitialized(
   SCIP_ACTIVATIONHDLR*  activationhdlr      /**< activation handler */
   )
{
   assert(activationhdlr != NULL);

   return activationhdlr->initialized;
}