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

/**@file   activation_suborbitope.c
 * @ingroup DEFPLUGINS_ACTIVATION
 * @brief  sub-orbitope activation handler
 * @author Sten Wessel
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <stdio.h>
#include <string.h>

#include "blockmemshell/memory.h"
#include "scip/type_misc.h"
#include "scip/pub_activation.h"
#include "scip/type_activation.h"
#include "scip/struct_activation.h"
#include "scip/struct_cons.h"
#include "scip/struct_misc.h"
#include "scip/activation_suborbitope.h"

/* fundamental activation handler properties */
#define ACTIVATION_NAME        "suborbitope"
#define ACTIVATION_DESC        "sub-orbitope activation handler"

/*
 * Data structures
 */
struct Patterns
{
   SCIP_VAR***           matrix;             /**< matrix of variables on which the symmetry acts (managed by the constraint) */
   int                   m;                  /**< number of rows in the matrix */
   int                   n;                  /**< number of columns in the matrix */
   int                   zeroheight;         /**< height of zeros column that activates a submatrix */
   int                   oneheight;          /**< height of ones column that activates a submatrix */
};
typedef struct Patterns PATTERNS;

struct SCIP_ActivationhdlrData
{
   SCIP_HASHMAP*         consmap;            /**< map of constraints to the patterns that activate the constraint */
};

/*
 * Local methods
 */

/** initializes the activation handler data structure */
static
SCIP_RETCODE createActivationhdlrData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLRDATA** activationhdlrdata    /**< pointer to store activation handler data structure */
   )
{
   assert(scip != NULL);
   assert(activationhdlrdata != NULL);

   SCIP_CALL( SCIPallocMemory(scip, activationhdlrdata) );
   SCIP_CALL( SCIPhashmapCreate(&(*activationhdlrdata)->consmap, SCIPblkmem(scip), 10) );

   return SCIP_OKAY;
}

/** clears all patterns from activation handler data */
static
SCIP_RETCODE removePatterns(
   SCIP*                 scip,               /**< SCIP data structure */
   PATTERNS**            patterns            /**< pointer to patterns data structure */
   )
{
   int i;

   assert(scip != NULL);
   assert(patterns != NULL);
   assert(*patterns != NULL);

   for( i = 0; i < (*patterns)->m; ++i )
   {
      SCIPfreeBlockMemoryArray(scip, &(*patterns)->matrix[i], (*patterns)->n);
   }
   SCIPfreeBlockMemoryArray(scip, &(*patterns)->matrix, (*patterns)->m);
   SCIPfreeBlockMemory(scip, patterns);

   return SCIP_OKAY;
}

static
SCIP_RETCODE storeConsPatterns(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_VAR***           matrix,             /**< matrix */
   int                   m,                  /**< number of rows in the matrix */
   int                   n,                  /**< number of columns in the matrix */
   int                   zeroheight,         /**< height of zeros column that activates a submatrix */
   int                   oneheight           /**< height of ones column that activates a submatrix */
   )
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;
   PATTERNS* patterns;
   int i;

   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(cons != NULL);
   assert(zeroheight > 0);
   assert(oneheight > 0);

   /* get activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   /* if patterns already exist for this constraint, remove the old entry */
   patterns = (PATTERNS*)SCIPhashmapGetImage(activationhdlrdata->consmap, cons);

   if( patterns != NULL )
   {
      SCIP_CALL( removePatterns(scip, &patterns) );
   }
   assert(patterns == NULL);

   /* create a new patterns structure */
   SCIP_CALL( SCIPallocBlockMemory(scip, &patterns) );

   patterns->zeroheight = zeroheight;
   patterns->oneheight = oneheight;

   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &patterns->matrix, m) );
   for( i = 0; i < m; ++i )
   {
      SCIP_CALL( SCIPduplicateBlockMemoryArray(scip, &patterns->matrix[i], matrix[i], n) );
   }

   patterns->m = m;
   patterns->n = n;
   SCIP_CALL( SCIPhashmapInsert(activationhdlrdata->consmap, cons, (void*)patterns) );

   SCIP_CALL( SCIPsetConsActivationhdlr(scip, cons, activationhdlr) );

   return SCIP_OKAY;
}

static
SCIP_RETCODE removeAllPatterns(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HASHMAP*         hashmap             /**< hashmap containing variable fixings */
   )
{
   int nentries;
   int i;
   SCIP_HASHMAPENTRY* entry;
   PATTERNS* patterns;

   assert(scip != NULL);
   assert(hashmap != NULL);

   nentries = SCIPhashmapGetNEntries(hashmap);

   for( i = 0; i < nentries; ++i )
   {
      entry = SCIPhashmapGetEntry(hashmap, i);

      if( entry != NULL )
      {
         patterns = (PATTERNS*)SCIPhashmapEntryGetImage(entry);
         SCIP_CALL( removePatterns(scip, &patterns) );
      }
   }

   return SCIP_OKAY;
}

static
SCIP_RETCODE findPatternMatches(
      SCIP*                 scip,               /**< SCIP data structure */
      PATTERNS*             patterns,           /**< patterns data structure */
      SCIP_ACTIVATIONSUBMATRIX** submatrix       /**< pointer to the first submatrix to store the result */
   )
{
   SCIP_VAR*** matrix;
   int* zerotowerheight;
   int* onetowerheight;
   int i;
   int j;
   int m;
   int n;
   int zeroheight;
   int oneheight;
   int* zerousedcols;
   int* oneusedcols;
   int nzerousedcols;
   int noneusedcols;
   SCIP_Bool transformed;
   SCIP_VAR* var;

   assert(scip != NULL);
   assert(patterns != NULL);

   matrix = patterns->matrix;
   m = patterns->m;
   n = patterns->n;
   zeroheight = patterns->zeroheight;
   oneheight = patterns->oneheight;
   *submatrix = NULL;

   transformed = SCIPisTransformed(scip);

   SCIP_CALL( SCIPallocBufferArray(scip, &zerotowerheight, n) );
   SCIP_CALL( SCIPallocBufferArray(scip, &onetowerheight, n) );

   SCIP_CALL( SCIPallocBufferArray(scip, &zerousedcols, n) );
   SCIP_CALL( SCIPallocBufferArray(scip, &oneusedcols, n) );

   for( j = 0; j < n; ++j )
   {
      zerotowerheight[j] = 0;
      onetowerheight[j] = 0;
   }

   for( i = 0; i < m; ++i )
   {
      nzerousedcols = 0;
      noneusedcols = 0;

      for( j = 0; j < n; ++j )
      {
         if( zerotowerheight[j] >= zeroheight )
         {
            zerousedcols[nzerousedcols++] = j;
         }
         else if( onetowerheight[j] >= oneheight )
         {
            oneusedcols[noneusedcols++] = j;
         }

         if( transformed )
         {
            SCIPgetTransformedVar(scip, matrix[i][j], &var);
         }
         else
         {
            var = matrix[i][j];
         }

         if( SCIPvarGetUbLocal(var) < 0.5 )
         {
            zerotowerheight[j] += 1;
            onetowerheight[j] = 0;
         }
         else if( SCIPvarGetLbLocal(var) > 0.5 )
         {
            onetowerheight[j] += 1;
            zerotowerheight[j] = 0;
         }
         else
         {
            zerotowerheight[j] = 0;
            onetowerheight[j] = 0;
         }
      }

      if( nzerousedcols >= 2 )
      {
         SCIP_ACTIVATIONSUBMATRIX* subm;
         SCIP_CALL( SCIPallocBlockMemory(scip, &subm) );
         subm->next = NULL;
         subm->ncols = nzerousedcols;
         subm->nrows = m - i;
         subm->orbitopetype = SCIP_ORBITOPETYPE_FULL;

         SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->cols, subm->ncols) );
         SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->rows, subm->nrows) );

         int k;

         for( k = 0; k < subm->ncols; ++k )
         {
            subm->cols[k] = zerousedcols[k];
         }

         for( k = 0; k < subm->nrows; ++k )
         {
            subm->rows[k] = i + k;
         }

         subm->next = *submatrix;
         *submatrix = subm;
      }

      if( noneusedcols >= 2 )
      {
         SCIP_ACTIVATIONSUBMATRIX* subm;
         SCIP_CALL( SCIPallocBlockMemory(scip, &subm) );
         subm->next = NULL;
         subm->ncols = noneusedcols;
         subm->nrows = m - i;
         subm->orbitopetype = SCIP_ORBITOPETYPE_FULL;

         SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->cols, subm->ncols) );
         SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->rows, subm->nrows) );

         int k;

         for( k = 0; k < subm->ncols; ++k )
         {
            subm->cols[k] = oneusedcols[k];
         }

         for( k = 0; k < subm->nrows; ++k )
         {
            subm->rows[k] = i + k;
         }

         subm->next = *submatrix;
         *submatrix = subm;
      }
   }

   SCIPfreeBufferArray(scip, &zerotowerheight);
   SCIPfreeBufferArray(scip, &onetowerheight);
   SCIPfreeBufferArray(scip, &zerousedcols);
   SCIPfreeBufferArray(scip, &oneusedcols);

   return SCIP_OKAY;
}


/*
 * Callback methods of activation handler
 */

/** copy method for activation handler plugins (called when SCIP copies plugins) */
static
SCIP_DECL_ACTIVATIONCOPY(activationCopySuborbitope)
{
   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   /* call inclusion method of activation handler */
   SCIP_CALL( SCIPincludeActivationSuborbitope(scip) );

   // TODO: copy data! (?)

   return SCIP_OKAY;
}

/** sets destructor method of activation handler */
static
SCIP_DECL_ACTIVATIONFREE(activationFreeSuborbitope)
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;

   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   /* free activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   /* free variable fixings in consmap */
   SCIP_CALL( removeAllPatterns(scip, activationhdlrdata->consmap) );
   SCIP_CALL( SCIPhashmapRemoveAll(activationhdlrdata->consmap) );

   SCIPhashmapFree(&activationhdlrdata->consmap);

   SCIPfreeMemory(scip, &activationhdlrdata);

   SCIPactivationhdlrSetData(activationhdlr, NULL);

   return SCIP_OKAY;
}

/** is active method of activation handler */
static
SCIP_DECL_ACTIVATIONFINDDATA(activationFindActivationDataSuborbitope)
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;
   PATTERNS* patterns;
   SCIP_CONS* origcons;

   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(cons != NULL);
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   /* get original constraint */
   origcons = cons;

   if( !SCIPconsIsOriginal(origcons) )
   {
      // TODO: there is no API for this?
      origcons = origcons->transorigcons;
   }

   /* get activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   /* get variable fixings for this constraint */
   patterns = (PATTERNS*)SCIPhashmapGetImage(activationhdlrdata->consmap, origcons);

   /* if not found, then we do not activate the constraint and leave result NULL */
   if( patterns == NULL )
   {
      return SCIP_OKAY;
   }

   SCIP_ACTIVATIONSUBMATRIX* submatrix;
   SCIP_CALL( findPatternMatches(scip, patterns, &submatrix) );

   *activationdata = (void*)submatrix;

   return SCIP_OKAY;
}


/*
 * Activation handler specific interface methods
 */

/** registers orbitope constraint to use this activation handler
 */
SCIP_RETCODE SCIPregisterConsActivationSuborbitope(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_VAR***           matrix,             /**< matrix */
   int                   m,                  /**< number of rows in the matrix */
   int                   n,                  /**< number of columns in the matrix */
   int                   zeroheight,         /**< height of zeros column that activates a submatrix */
   int                   oneheight           /**< height of ones column that activates a submatrix */
   )
{
   SCIP_ACTIVATIONHDLR* activationhdlr;

   assert(scip != NULL);
   assert(cons != NULL);

   /* find activation handler */
   activationhdlr = SCIPfindActivationhdlr(scip, ACTIVATION_NAME);
   if( activationhdlr == NULL )
   {
      SCIPerrorMessage("Could not find activation handler <%s>.\n", ACTIVATION_NAME);
      return SCIP_PLUGINNOTFOUND;
   }
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   SCIP_CALL( storeConsPatterns(scip, activationhdlr, cons, matrix, m, n, zeroheight, oneheight) );

   return SCIP_OKAY;
}

/** creates the sub-orbitope activation handler and includes it in SCIP
 */
SCIP_RETCODE SCIPincludeActivationSuborbitope(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata = NULL;

   /* create activation handler data */
   SCIP_CALL( createActivationhdlrData(scip, &activationhdlrdata) );

   SCIP_CALL( SCIPincludeActivationhdlr(scip, ACTIVATION_NAME, ACTIVATION_DESC, NULL,
         activationFreeSuborbitope, NULL, NULL, NULL, activationFindActivationDataSuborbitope, activationhdlrdata) );

   return SCIP_OKAY;
}
