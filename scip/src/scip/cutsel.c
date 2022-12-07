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

/**@file   cutsel.c
 * @ingroup OTHER_CFILES
 * @brief  methods for cut selectors
 * @author Felipe Serrano
 * @author Mark Turner
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "scip/set.h"
#include "scip/clock.h"
#include "scip/paramset.h"
#include "scip/scip.h"
#include "scip/cutsel.h"

#include "scip/struct_cutsel.h"


/** method to call, when the priority of a cut selector was changed */
static
SCIP_DECL_PARAMCHGD(paramChgdCutselPriority)
{  /*lint --e{715}*/
   SCIP_PARAMDATA* paramdata;

   paramdata = SCIPparamGetData(param);
   assert(paramdata != NULL);

   /* use SCIPsetCutselPriority() to mark the cutsels unsorted */
   SCIP_CALL( SCIPsetCutselPriority(scip, (SCIP_CUTSEL*)paramdata, SCIPparamGetInt(param)) ); /*lint !e740*/

   return SCIP_OKAY;
}

/** internal method for creating a cut selector */
static
SCIP_RETCODE doCutselCreate(
   SCIP_CUTSEL**         cutsel,             /**< pointer to store cut selector */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_MESSAGEHDLR*     messagehdlr,        /**< message handler */
   BMS_BLKMEM*           blkmem,             /**< block memory for parameter settings */
   const char*           name,               /**< name of cut selector */
   const char*           desc,               /**< description of cut selector */
   int                   priority,           /**< priority of the cut selector */
   SCIP_DECL_CUTSELCOPY ((*cutselcopy)),     /**< copy method of cut selector or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_CUTSELFREE ((*cutselfree)),     /**< destructor of cut selector */
   SCIP_DECL_CUTSELINIT ((*cutselinit)),     /**< initialize cut selector */
   SCIP_DECL_CUTSELEXIT ((*cutselexit)),     /**< deinitialize cut selector */
   SCIP_DECL_CUTSELINITSOL((*cutselinitsol)),/**< solving process initialization method of cut selector */
   SCIP_DECL_CUTSELEXITSOL((*cutselexitsol)),/**< solving process deinitialization method of cut selector */
   SCIP_DECL_CUTSELSELECT((*cutselselect)),  /**< cut selection method */
   SCIP_CUTSELDATA*      cutseldata          /**< cut selector data */
   )
{
   char paramname[SCIP_MAXSTRLEN];
   char paramdesc[SCIP_MAXSTRLEN];

   assert(cutsel != NULL);
   assert(name != NULL);
   assert(desc != NULL);
   assert(cutselselect != NULL);

   SCIP_ALLOC( BMSallocMemory(cutsel) );
   BMSclearMemory(*cutsel);

   SCIP_ALLOC( BMSduplicateMemoryArray(&(*cutsel)->name, name, strlen(name)+1) );
   SCIP_ALLOC( BMSduplicateMemoryArray(&(*cutsel)->desc, desc, strlen(desc)+1) );
   (*cutsel)->priority = priority;
   (*cutsel)->cutselcopy = cutselcopy;
   (*cutsel)->cutselfree = cutselfree;
   (*cutsel)->cutselinit = cutselinit;
   (*cutsel)->cutselexit = cutselexit;
   (*cutsel)->cutselinitsol = cutselinitsol;
   (*cutsel)->cutselexitsol = cutselexitsol;
   (*cutsel)->cutselselect = cutselselect;
   (*cutsel)->cutseldata = cutseldata;
   (*cutsel)->initialized = FALSE;

   /* create clocks */
   SCIP_CALL( SCIPclockCreate(&(*cutsel)->setuptime, SCIP_CLOCKTYPE_DEFAULT) );
   SCIP_CALL( SCIPclockCreate(&(*cutsel)->cutseltime, SCIP_CLOCKTYPE_DEFAULT) );

   /* add parameters */
   (void) SCIPsnprintf(paramname, SCIP_MAXSTRLEN, "cutselection/%s/priority", name);
   (void) SCIPsnprintf(paramdesc, SCIP_MAXSTRLEN, "priority of cut selection rule <%s>", name);
   SCIP_CALL( SCIPsetAddIntParam(set, messagehdlr, blkmem, paramname, paramdesc,
         &(*cutsel)->priority, FALSE, priority, INT_MIN/4, INT_MAX/2,
         paramChgdCutselPriority, (SCIP_PARAMDATA*)(*cutsel)) ); /*lint !e740*/

   return SCIP_OKAY;
}


/** creates a cut selector */
SCIP_RETCODE SCIPcutselCreate(
   SCIP_CUTSEL**         cutsel,             /**< pointer to store cut selector */
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_MESSAGEHDLR*     messagehdlr,        /**< message handler */
   BMS_BLKMEM*           blkmem,             /**< block memory for parameter settings */
   const char*           name,               /**< name of cut selector */
   const char*           desc,               /**< description of cut selector */
   int                   priority,           /**< priority of the cut selector in standard mode */
   SCIP_DECL_CUTSELCOPY ((*cutselcopy)),     /**< copy method of cut selector or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_CUTSELFREE ((*cutselfree)),     /**< destructor of cut selector */
   SCIP_DECL_CUTSELINIT ((*cutselinit)),     /**< initialize cut selector */
   SCIP_DECL_CUTSELEXIT ((*cutselexit)),     /**< deinitialize cut selector */
   SCIP_DECL_CUTSELINITSOL((*cutselinitsol)),/**< solving process initialization method of cut selector */
   SCIP_DECL_CUTSELEXITSOL((*cutselexitsol)),/**< solving process deinitialization method of cut selector */
   SCIP_DECL_CUTSELSELECT((*cutselselect)),  /**< cut selection method */
   SCIP_CUTSELDATA*      cutseldata          /**< cut selector data */
   )
{
   assert(cutsel != NULL);
   assert(name != NULL);
   assert(desc != NULL);
   assert(cutselselect != NULL);

   SCIP_CALL_FINALLY( doCutselCreate(cutsel, set, messagehdlr, blkmem, name, desc, priority,
         cutselcopy, cutselfree, cutselinit, cutselexit, cutselinitsol, cutselexitsol, cutselselect,
         cutseldata), (void) SCIPcutselFree(cutsel, set) );

   return SCIP_OKAY;
}

/** gets name of cut selector */
const char* SCIPcutselGetName(
   SCIP_CUTSEL*          cutsel              /**< cut selector */
   )
{
   assert(cutsel != NULL);

   return cutsel->name;
}

/** calls cut selectors to select cuts */
SCIP_RETCODE SCIPcutselsSelect(
   SCIP_SET*             set,                /**< global SCIP settings */
   SCIP_ROW**            cuts,               /**< array with cuts to select from */
   int                   ncuts,              /**< length of cuts */
   int                   nforcedcuts,        /**< number of forced cuts at start of given array */
   SCIP_Bool             root,               /**< are we at the root node? */
   int                   maxnselectedcuts,   /**< maximum number of cuts to be selected */
   int*                  nselectedcuts       /**< pointer to return number of selected cuts */
   )
{
   int i;
   SCIP_RESULT result = SCIP_DIDNOTFIND;

   assert(nselectedcuts != NULL);

   /* sort the cut selectors by priority */
   SCIPsetSortCutsels(set);

   /* Redefine maxnselectedcuts to be w.r.t the optional cuts. */
   maxnselectedcuts -= nforcedcuts;

   /* try all cut selectors until one succeeds */
   *nselectedcuts = 0;
   for( i = 0; i < set->ncutsels && result == SCIP_DIDNOTFIND; ++i )
   {
      SCIP_CUTSEL* cutsel;

      cutsel = set->cutsels[i];

      assert(cutsel != NULL);
      assert(ncuts - nforcedcuts > 0);
      assert(maxnselectedcuts > 0);

      SCIP_CALL( cutsel->cutselselect(set->scip, cutsel, &(cuts[nforcedcuts]), ncuts - nforcedcuts, cuts, nforcedcuts,
            root, maxnselectedcuts, nselectedcuts, &result) );

      assert(*nselectedcuts <= maxnselectedcuts);
      assert(result == SCIP_SUCCESS || result == SCIP_DIDNOTFIND);
      assert(result != SCIP_DIDNOTFIND || *nselectedcuts == 0);
   }

   return SCIP_OKAY;
}

/** gets description of cut selector */
const char* SCIPcutselGetDesc(
   SCIP_CUTSEL*          cutsel              /**< cut selector */
   )
{
   assert(cutsel != NULL);

   return cutsel->desc;
}

/** copies the given cut selector to a new scip */
SCIP_RETCODE SCIPcutselCopyInclude(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_SET*             set                 /**< SCIP_SET of SCIP to copy to */
   )
{
   assert(cutsel != NULL);
   assert(set != NULL);
   assert(set->scip != NULL);

   if( cutsel->cutselcopy != NULL )
   {
      SCIPsetDebugMsg(set, "including cut selector %s in subscip %p\n", SCIPcutselGetName(cutsel), (void*)set->scip);
      SCIP_CALL( cutsel->cutselcopy(set->scip, cutsel) );
   }
   return SCIP_OKAY;
}

/** frees memory of cut selector */
SCIP_RETCODE SCIPcutselFree(
   SCIP_CUTSEL**         cutsel,             /**< pointer to cut selector data structure */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(cutsel != NULL);

   if( *cutsel == NULL )
      return SCIP_OKAY;

   assert(!(*cutsel)->initialized);
   assert(set != NULL);

   /* call destructor of cut selector */
   if( (*cutsel)->cutselfree != NULL )
   {
      SCIP_CALL( (*cutsel)->cutselfree(set->scip, *cutsel) );
   }

   /* free clocks */
   SCIPclockFree(&(*cutsel)->cutseltime);
   SCIPclockFree(&(*cutsel)->setuptime);

   BMSfreeMemoryArrayNull(&(*cutsel)->name);
   BMSfreeMemoryArrayNull(&(*cutsel)->desc);
   BMSfreeMemory(cutsel);

   return SCIP_OKAY;
}

/** initializes cut selector */
SCIP_RETCODE SCIPcutselInit(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(cutsel != NULL);
   assert(set != NULL);

   if( cutsel->initialized )
   {
      SCIPerrorMessage("cut selector <%s> already initialized", cutsel->name);
      return SCIP_INVALIDCALL;
   }

   if( set->misc_resetstat )
   {
      SCIPclockReset(cutsel->setuptime);
      SCIPclockReset(cutsel->cutseltime);
   }

   if( cutsel->cutselinit != NULL )
   {
      /* start timing */
      SCIPclockStart(cutsel->setuptime, set);

      SCIP_CALL( cutsel->cutselinit(set->scip, cutsel) );

      /* stop timing */
      SCIPclockStop(cutsel->setuptime, set);
   }

   cutsel->initialized = TRUE;

   return SCIP_OKAY;
}

/** deinitializes cut selector */
SCIP_RETCODE SCIPcutselExit(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(cutsel != NULL);
   assert(set != NULL);

   if( !cutsel->initialized )
   {
      SCIPerrorMessage("cut selector <%s> not initialized", cutsel->name);
      return SCIP_INVALIDCALL;
   }

   if( cutsel->cutselexit != NULL )
   {
      /* start timing */
      SCIPclockStart(cutsel->setuptime, set);

      SCIP_CALL( cutsel->cutselexit(set->scip, cutsel) );

      /* stop timing */
      SCIPclockStop(cutsel->setuptime, set);
   }
   cutsel->initialized = FALSE;

   return SCIP_OKAY;
}

/** informs cut selector that the branch and bound process is being started */
SCIP_RETCODE SCIPcutselInitsol(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(cutsel != NULL);
   assert(set != NULL);

   /* call solving process initialization method of cut selector */
   if( cutsel->cutselinitsol != NULL )
   {
      /* start timing */
      SCIPclockStart(cutsel->setuptime, set);

      SCIP_CALL( cutsel->cutselinitsol(set->scip, cutsel) );

      /* stop timing */
      SCIPclockStop(cutsel->setuptime, set);
   }

   return SCIP_OKAY;
}

/** informs cut selector that the branch and bound process is being started */
SCIP_RETCODE SCIPcutselExitsol(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_SET*             set                 /**< global SCIP settings */
   )
{
   assert(cutsel != NULL);
   assert(set != NULL);

   /* call solving process deinitialization method of cut selector */
   if( cutsel->cutselexitsol != NULL )
   {
      /* start timing */
      SCIPclockStart(cutsel->setuptime, set);

      SCIP_CALL( cutsel->cutselexitsol(set->scip, cutsel) );

      /* stop timing */
      SCIPclockStop(cutsel->setuptime, set);
   }

   return SCIP_OKAY;
}

/** gets user data of cut selector */
SCIP_CUTSELDATA* SCIPcutselGetData(
   SCIP_CUTSEL*          cutsel              /**< cut selector */
   )
{
   assert(cutsel != NULL);

   return cutsel->cutseldata;
}

/** sets user data of cut selector; user has to free old data in advance! */
void SCIPcutselSetData(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_CUTSELDATA*      cutseldata          /**< new cut selector user data */
)
{
   assert(cutsel != NULL);

   cutsel->cutseldata = cutseldata;
}

/** gets priority of cut selector */
int SCIPcutselGetPriority(
   SCIP_CUTSEL*          cutsel              /**< cut selector */
   )
{
   assert(cutsel != NULL);

   return cutsel->priority;
}

/** enables or disables all clocks of @p cutsel, depending on the value of the flag */
void SCIPcutselEnableOrDisableClocks(
   SCIP_CUTSEL*          cutsel,             /**< the cut selector for which all clocks should be enabled or disabled */
   SCIP_Bool             enable              /**< should the clocks of the cut selector be enabled? */
   )
{
   assert(cutsel != NULL);

   SCIPclockEnableOrDisable(cutsel->setuptime, enable);
   SCIPclockEnableOrDisable(cutsel->cutseltime, enable);
}


/* new callback/method setter methods */

/** sets copy method of cut selector */
void SCIPcutselSetCopy(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_DECL_CUTSELCOPY  ((*cutselcopy))     /**< copy method of cut selector or NULL if you don't want to copy your plugin into sub-SCIPs */
   )
{
   assert(cutsel != NULL);

   cutsel->cutselcopy = cutselcopy;
}

/** sets destructor method of cut selector */
void SCIPcutselSetFree(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_DECL_CUTSELFREE  ((*cutselfree))     /**< destructor of cut selector */
   )
{
   assert(cutsel != NULL);

   cutsel->cutselfree = cutselfree;
}

/** sets initialization method of cut selector */
void SCIPcutselSetInit(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_DECL_CUTSELINIT  ((*cutselinit))     /**< initialize cut selector */
   )
{
   assert(cutsel != NULL);

   cutsel->cutselinit = cutselinit;
}

/** sets deinitialization method of cut selector */
void SCIPcutselSetExit(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_DECL_CUTSELEXIT  ((*cutselexit))     /**< deinitialize cut selector */
   )
{
   assert(cutsel != NULL);

   cutsel->cutselexit = cutselexit;
}

/** sets solving process initialization method of cut selector */
void SCIPcutselSetInitsol(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_DECL_CUTSELINITSOL ((*cutselinitsol))/**< solving process initialization method of cut selector */
   )
{
   assert(cutsel != NULL);

   cutsel->cutselinitsol = cutselinitsol;
}

/** sets solving process deinitialization method of cut selector */
void SCIPcutselSetExitsol(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_DECL_CUTSELEXITSOL ((*cutselexitsol))/**< solving process deinitialization method of cut selector */
   )
{
   assert(cutsel != NULL);

   cutsel->cutselexitsol = cutselexitsol;
}

/** sets priority of cut selector */
void SCIPcutselSetPriority(
   SCIP_CUTSEL*          cutsel,             /**< cut selector */
   SCIP_SET*             set,                /**< global SCIP settings */
   int                   priority            /**< new priority of the cut selector */
   )
{
   assert(cutsel != NULL);
   assert(set != NULL);

   cutsel->priority = priority;
   set->cutselssorted = FALSE;
}

/** is cut selector initialized? */
SCIP_Bool SCIPcutselIsInitialized(
   SCIP_CUTSEL*          cutsel              /**< cut selector */
   )
{
   assert(cutsel != NULL);

   return cutsel->initialized;
}

/** gets time in seconds used in this cut selector for setting up for next stages */
SCIP_Real SCIPcutselGetSetupTime(
   SCIP_CUTSEL*          cutsel              /**< cut selector */
   )
{
   assert(cutsel != NULL);

   return SCIPclockGetTime(cutsel->setuptime);
}

/** gets time in seconds used in this cut selector */
SCIP_Real SCIPcutselGetTime(
   SCIP_CUTSEL*          cutsel              /**< cut selector */
   )
{
   assert(cutsel != NULL);

   return SCIPclockGetTime(cutsel->cutseltime);
}

/** compares two cut selectors w. r. to their priority */
SCIP_DECL_SORTPTRCOMP(SCIPcutselComp)
{  /*lint --e{715}*/
   return ((SCIP_CUTSEL*)elem2)->priority - ((SCIP_CUTSEL*)elem1)->priority;
}
