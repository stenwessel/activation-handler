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

/**@file   activation_makespan.c
 * @ingroup DEFPLUGINS_ACTIVATION
 * @brief  makespan activation handler
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
#include "scip/activation_makespan.h"

/* fundamental activation handler properties */
#define ACTIVATION_NAME        "makespan"
#define ACTIVATION_DESC        "makespan activation handler"

#define INITIAL_CONSMAP_SIZE   10            /**< initial size of the hashmap storing the constraints that use this activation handler */

/*
 * Data structures
 */

/*
 * Data that defines the behavior of the activation handler.
 */
struct ActivationData
{
    SCIP_VAR***           matrix;             /**< matrix of variables on which the symmetry acts (managed by the constraint): rows are jobs, columns are machines */
    int*                  jobtimes;           /**< array that defines the processing time of each job */
    int                   nmachines;          /**< number of machines (number of columns in matrix) */
    int                   njobs;              /**< number of jobs (number of rows in matrix, size of jobtimes array) */
    SCIP_HASHMAP*         makespans;          /**< map used as buffer when searching for sub-symmetries, to record fixed makespans per machine at the current node */
    SCIP_HASHMAP*         newsymmakespans;    /**< map used as buffer when searching for sub-symmetries, to record when a new sub-symmetry is found */
};
typedef struct ActivationData ACTIVATIONDATA;

struct SCIP_ActivationhdlrData
{
   SCIP_HASHMAP*         consmap;            /**< map of constraints to the variable fixings that activate the constraint */
};

/*
 * Local methods
 */

/** initializes the activation handler data structure */
static
SCIP_RETCODE createActivationhdlrData(
   SCIP*                     scip,                 /**< SCIP data structure */
   SCIP_ACTIVATIONHDLRDATA** activationhdlrdata    /**< pointer to store activation handler data structure */
   )
{
   assert(scip != NULL);
   assert(activationhdlrdata != NULL);

   SCIP_CALL( SCIPallocMemory(scip, activationhdlrdata) );
   SCIP_CALL( SCIPhashmapCreate(&(*activationhdlrdata)->consmap, SCIPblkmem(scip), INITIAL_CONSMAP_SIZE) );

   return SCIP_OKAY;
}

/** clears activationdata from activation handler data */
static
SCIP_RETCODE removeActivationData(
   SCIP*                 scip,               /**< SCIP data structure */
   ACTIVATIONDATA**      activationdata      /**< pointer to variable fixings data structure */
   )
{
   int i;

   assert(scip != NULL);
   assert(activationdata != NULL);
   assert(*activationdata != NULL);

   SCIPhashmapFree(&(*activationdata)->newsymmakespans);
   SCIPhashmapFree(&(*activationdata)->makespans);

   SCIPfreeBlockMemoryArray(scip, &(*activationdata)->jobtimes, (*activationdata)->njobs);

   for( i = 0; i < (*activationdata)->njobs; ++i )
   {
      SCIPfreeBlockMemoryArray(scip, &(*activationdata)->matrix[i], (*activationdata)->nmachines);
   }
   SCIPfreeBlockMemoryArray(scip, &(*activationdata)->matrix, (*activationdata)->njobs);

   SCIPfreeBlockMemory(scip, activationdata);

   return SCIP_OKAY;
}

/** stores activationdata for the provided constraint */
static
SCIP_RETCODE storeConsActivationData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_VAR***           matrix,             /**< matrix of scheduling variables */
   int*                  jobtimes,           /**< job processing times */
   int                   nmachines,          /**< number of machines (columns in the matrix) */
   int                   njobs               /**< number of jobs (rows in the matrix) */
   )
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;
   ACTIVATIONDATA* activationdata;
   int i;

   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(cons != NULL);
   assert(matrix != NULL);
   assert(jobtimes != NULL);
   assert(nmachines > 0);
   assert(njobs > 0);

   /* get activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   /* if activationdata already exist for this constraint, remove the old entry */
   activationdata = (ACTIVATIONDATA*)SCIPhashmapGetImage(activationhdlrdata->consmap, cons);

   if( activationdata != NULL )
   {
      SCIP_CALL( removeActivationData(scip, &activationdata) );
   }
   assert(activationdata == NULL);

   /* create a new activation data structure */
   SCIP_CALL( SCIPallocBlockMemory(scip, &activationdata) );

   activationdata->nmachines = nmachines;
   activationdata->njobs = njobs;

   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &activationdata->matrix, njobs) );
   for( i = 0; i < njobs; ++i )
   {
      SCIP_CALL( SCIPduplicateBlockMemoryArray(scip, &activationdata->matrix[i], matrix[i], nmachines) );
   }

   SCIP_CALL( SCIPduplicateBlockMemoryArray(scip, &activationdata->jobtimes, jobtimes, njobs) );

   /* This maps a makespan value to the number of machines that have this makespan, hence it can never be larger than the number of machines */
   SCIP_CALL( SCIPhashmapCreate(&activationdata->makespans, SCIPblkmem(scip), activationdata->nmachines) );

   /* New sub-symmetries are only discovered when >= 2 have the same makespan, hence the size of this map will never be larger than half the number of machines */
   SCIP_CALL( SCIPhashmapCreate(&activationdata->newsymmakespans, SCIPblkmem(scip), activationdata->nmachines / 2) );


   SCIP_CALL( SCIPhashmapInsert(activationhdlrdata->consmap, cons, (void*)activationdata) );

   SCIP_CALL( SCIPsetConsActivationhdlr(scip, cons, activationhdlr) );

   return SCIP_OKAY;
}

/** removes activationdata for all constraints using this activation handler */
static
SCIP_RETCODE removeAllActivationData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HASHMAP*         hashmap             /**< hashmap containing activation data */
   )
{
   int nentries;
   int i;
   SCIP_HASHMAPENTRY* entry;
   ACTIVATIONDATA* activationdata;

   assert(scip != NULL);
   assert(hashmap != NULL);

   nentries = SCIPhashmapGetNEntries(hashmap);

   for( i = 0; i < nentries; ++i )
   {
      entry = SCIPhashmapGetEntry(hashmap, i);

      if( entry != NULL )
      {
         activationdata = (ACTIVATIONDATA*)SCIPhashmapEntryGetImage(entry);
         SCIP_CALL( removeActivationData(scip, &activationdata) );
      }
   }

   return SCIP_OKAY;
}

/*
 * Find the sub-symmetries corresponding to machines that have equal fixed makespan over jobs 1..k, for every k.
 *
 * We have a sub-symmetry w.r.t. the rows/jobs (k+1, ..., |J|) and columns/machines (m_1, ..., m_\ell) if:
 * - Jobs {1, ..., k} are scheduled on m_i or not scheduled on m_i for all i=m_1, ..., m_\ell (i.e., the schedules of the first k jobs are _fixed_ at the machines)
 * - The makespans of the machines m_i of the jobs 1..k are all equal: \sum_{j=1}^k p_j x_{j,m_i} = \sum_{j=1}^k p_j x_{j,m_{i'}} for all machines m_i, m_{i'}
 *
 * The algorithm is as follows:
 * We iterate over the rows of the matrix of variables. For row/job k, we populate:
 * fixedmakespan[m] <- \sum_{j=1}^k p_j x_{j,m} if all x_{j,m} are fixed (to either 0 or 1), or,
 * if some x_{j,m} is not fixed, fixedmakespan[m] <- -1.
 *
 * After processing each row, we count how many machines have equal fixed makespan.
 * If there are >= 2, this defines a sub-symmetry and its corresponding submatrix is recorded.
 *
 * As an optimization, if the submatrix corresponding to the found sub-symmetry has a first row that is fully fixed to zero,
 * we postpone recording this submatrix and first process the next row.
 *
 * The following additional data structures are used:
 * - activationdata->makespans: maps a makespan value to the number of machines that have this fixed makespan
 * - activationdata->newsymmakespans: used as a hashset to record makespan values v for which activationdata->makespans[v] >= 2
 * - reinsertmakespans: buffer array of makespans for which the submatrix turns out to have a first row fixed to zero.
 *       In this case, the submatrix is deferred to after processing the next row, and hence must be re-inserted into activationdata->newsymmakespans
 */
static
SCIP_RETCODE findSubSymmetries(
      SCIP*                 scip,               /**< SCIP data structure */
      ACTIVATIONDATA*       activationdata,     /**< activation data structure */
      SCIP_ACTIVATIONSUBMATRIX** submatrix      /**< return values: pointer to the first found sub-symmetric submatrix */
)
{
   int* fixedmakespan;
   int j;
   int m;
   int e;
   int k;
   int i;
   SCIP_VAR* var;
   int count;
   SCIP_ACTIVATIONSUBMATRIX* subm;
   SCIP_HASHMAPENTRY* entry;
   SCIP_Bool allundetermined;
   SCIP_Bool allfixedzero;
   int* reinsertmakespans;
   int reinsertmakespanshead;
#ifdef SCIP_DEBUG
   int submatrixcount;
   int u;
   int v;
   int prevsum;
   int sum;
#endif

   assert(scip != NULL);
   assert(activationdata != NULL);
   assert(submatrix != NULL);

   *submatrix = NULL;

#ifdef SCIP_DEBUG
   submatrixcount = 0;
#endif

   /* Record for every machine the makespan that is fixed at this node
    * For a given job k, fixedmakespan[m] is equal to the makespan of this machine up to job k
    * fixedmakespan[m] is -1 when x_{j,m} is not fixed to either 0 or 1 for some j=1, ... k (indeterminate) */
   SCIP_CALL( SCIPallocBufferArray(scip, &fixedmakespan, activationdata->nmachines) );
   for( m = 0; m < activationdata->nmachines; ++m )
      fixedmakespan[m] = 0;

   /* Buffer for makespans that are found as sub-symmetry, but are deferred to the next row.
    * They need to be re-inserted into the newsymmakespans hashset after iteration */
   SCIP_CALL( SCIPallocBufferArray(scip, &reinsertmakespans, activationdata->nmachines / 2 + 1) );
   reinsertmakespanshead = 0;

   /* Initially, all machines have fixed makespan of zero */
   SCIPhashmapInsertInt(activationdata->makespans, (void*)(size_t)0, activationdata->nmachines);

   /* Iterate over the rows of the matrix */
   for( j = 0; j < activationdata->njobs; ++j )
   {
      /* Check if we found sub-symmetries in the previous row, and process them */
      if( !SCIPhashmapIsEmpty(activationdata->newsymmakespans) )
      {
         count = SCIPhashmapGetNEntries(activationdata->newsymmakespans);
         for( e = 0; e < count; ++e )
         {
            entry = SCIPhashmapGetEntry(activationdata->newsymmakespans, e);
            if( entry == NULL )
               continue;

            m = (int)(size_t)SCIPhashmapEntryGetOrigin(entry);

            SCIP_CALL( SCIPallocBlockMemory(scip, &subm) );
            subm->next = NULL;
            subm->ncols = SCIPhashmapGetImageInt(activationdata->makespans, (void*)(size_t)m);
            subm->nrows = activationdata->njobs - j;

            /* In general, the sub-symmetry is a packing orbitope, except for when the submatrix includes all columns of the matrix; then it is a partitioning orbitope */
            subm->orbitopetype = subm->ncols == activationdata->nmachines ? SCIP_ORBITOPETYPE_PARTITIONING : SCIP_ORBITOPETYPE_PACKING;

            SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->cols, subm->ncols) );

            k = 0;
            for( i = 0; i < activationdata->nmachines && k < subm->ncols; ++i )
            {
               if( fixedmakespan[i] == m )
                  subm->cols[k++] = i;
            }

            /* Check the values in the first row of the submatrix
             * If all the values are fixed to zero, ignore this submatrix now and defer to the next 'row' */
            allfixedzero = TRUE;
            for( i = 0; i < subm->ncols; ++i )
            {
               if( SCIPvarGetUbLocal(activationdata->matrix[j][subm->cols[i]]) > 0.5 )
               {
                  allfixedzero = FALSE;
                  break;
               }
            }

            if( allfixedzero )
            {
               SCIPfreeBlockMemoryArray(scip, &subm->cols, subm->ncols);
               SCIPfreeBlockMemory(scip, &subm);
               reinsertmakespans[reinsertmakespanshead++] = m;
               continue;
            }

            SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->rows, subm->nrows) );
            assert(j + subm->nrows <= activationdata->njobs);
            for( k = 0; k < subm->nrows; ++k )
               subm->rows[k] = j + k;

            subm->next = *submatrix;
            *submatrix = subm;

#ifdef SCIP_DEBUG
            submatrixcount += 1;

            /* DEBUG: verify that indeed this is a valid submatrix */
            prevsum = 0;
            for( v = 0; v < subm->ncols; ++v )
            {
               sum = 0;

               for( u = 0; u < subm->nrows; ++u )
               {
                  if( SCIPvarGetLbLocal(activationdata->matrix[subm->rows[u]][subm->cols[v]]) > 0.5 )
                     sum += activationdata->jobtimes[u];
                  else if( !(SCIPvarGetLbLocal(activationdata->matrix[subm->rows[u]][subm->cols[v]]) > 0.5 || SCIPvarGetUbLocal(activationdata->matrix[subm->rows[u]][subm->cols[v]]) < 0.5) )
                     SCIPdebugMsg(scip, "!!! Some variable above the submatrix is not fixed to either 0/1!\n");
               }

               if( v > 0 && sum != prevsum )
                  SCIPdebugMsg(scip, "!!! Some column sum above the submatrix is not equal to the others!\n");

               prevsum = sum;
            }
#endif
         }

         /* We have now processed the newly found sub-symmetries */
         SCIPhashmapRemoveAll(activationdata->newsymmakespans);

         /* EXCEPT for the ones that have the first row fixed to zero: re-insert makespans that are deferred to the next row */
         for( i = 0; i < reinsertmakespanshead; ++i )
         {
            SCIPhashmapInsertInt(activationdata->newsymmakespans, (void*)(size_t)reinsertmakespans[i], 0);
         }
         reinsertmakespanshead = 0;
      }

      /* Indicates that fixedmakespan[m] == -1 for all m. In that case, we can terminate looping over the rows. */
      allundetermined = TRUE;

      for( m = 0; m < activationdata->nmachines; ++m )
      {
         /* If the fixed makespan is undetermined (because some earlier entries are not fixed), it stays that way */
         if( fixedmakespan[m] == -1 )
            continue;

         allundetermined = FALSE;

         var = activationdata->matrix[j][m];

         if( SCIPvarGetLbLocal(var) > 0.5 )
         {
            /* Fixed to 1, fixed makespan of this machine increases with the size of this job
             * Decrease the makespan counter for the current value */
            count = SCIPhashmapGetImageInt(activationdata->makespans, (void*)(size_t)fixedmakespan[m]) - 1;
            if( count <= 0 )
               SCIPhashmapRemove(activationdata->makespans, (void*)(size_t)fixedmakespan[m]);
            else
               SCIPhashmapSetImageInt(activationdata->makespans, (void*)(size_t)fixedmakespan[m], count);

            /* Update new makespan */
            fixedmakespan[m] = fixedmakespan[m] + activationdata->jobtimes[j];

            /* Update new makespan count */
            count = SCIPhashmapGetImageInt(activationdata->makespans, (void*)(size_t)fixedmakespan[m]);
            if( count == INT_MAX )
               SCIPhashmapInsertInt(activationdata->makespans, (void*)(size_t)fixedmakespan[m], 1);
            else
            {
               SCIPhashmapSetImageInt(activationdata->makespans, (void*)(size_t)fixedmakespan[m], count + 1);

               /* Mark this as a new sub-symmetry that is found */
               if( !SCIPhashmapExists(activationdata->newsymmakespans, (void*)(size_t)fixedmakespan[m]) )
                  SCIPhashmapInsertInt(activationdata->newsymmakespans, (void*)(size_t)fixedmakespan[m], 0);
            }
         }
         else if( !(SCIPvarGetUbLocal(var) < 0.5) )
         {
            /* Not fixed (to either 0 or 1), then its fixed makespan becomes indeterminate from now on */

            /* Decrease the makespan counter for the current value */
            count = SCIPhashmapGetImageInt(activationdata->makespans, (void*)(size_t)fixedmakespan[m]) - 1;
            if( count <= 0 )
               SCIPhashmapRemove(activationdata->makespans, (void*)(size_t)fixedmakespan[m]);
            else
               SCIPhashmapSetImageInt(activationdata->makespans, (void*)(size_t)fixedmakespan[m], count);

            fixedmakespan[m] = -1;
         }
      }

      if( allundetermined )
         break;
   }

   SCIP_CALL( SCIPhashmapRemoveAll(activationdata->makespans) );
   SCIP_CALL( SCIPhashmapRemoveAll(activationdata->newsymmakespans) );

   SCIPfreeBufferArray(scip, &reinsertmakespans);
   SCIPfreeBufferArray(scip, &fixedmakespan);

#ifdef SCIP_DEBUG
   if( submatrixcount > 0 )
   {
      SCIPdebugMsg(scip, "Found %d submatrices\n", submatrixcount);
   }
#endif

   return SCIP_OKAY;
}


/*
 * Callback methods of activation handler
 */

/* Initializes the activation handler by replacing the provided original variables by their transformations if necessary */
static
SCIP_DECL_ACTIVATIONINIT(activationInitMakespan)
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;
   ACTIVATIONDATA* activationdata;
   SCIP_HASHMAPENTRY* entry;
   int nentries;
   int i;
   int j;
   int m;

   if( !SCIPisTransformed(scip) )
      return SCIP_OKAY;

   /* Get the transformed variables */

   /* get activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   nentries = SCIPhashmapGetNEntries(activationhdlrdata->consmap);

   for( i = 0; i < nentries; ++i )
   {
      entry = SCIPhashmapGetEntry(activationhdlrdata->consmap, i);

      if( entry != NULL )
      {
         activationdata = (ACTIVATIONDATA*)SCIPhashmapEntryGetImage(entry);

         for( j = 0; j < activationdata->njobs; ++j )
         {
            for( m = 0; m < activationdata->nmachines; m++ )
            {
               SCIP_CALL( SCIPgetTransformedVar(scip, activationdata->matrix[j][m], &(activationdata->matrix[j][m])) );
            }
         }
      }
   }

   return SCIP_OKAY;
}

/** copy method for activation handler plugins (called when SCIP copies plugins) */
static
SCIP_DECL_ACTIVATIONCOPY(activationCopyMakespan)
{
   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   /* call inclusion method of activation handler */
   SCIP_CALL( SCIPincludeActivationMakespan(scip) );

   // TODO: copy data! (?)

   return SCIP_OKAY;
}

/** sets destructor method of activation handler */
static
SCIP_DECL_ACTIVATIONFREE(activationFreeMakespan)
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;

   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   /* free activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   /* free variable fixings in consmap */
   SCIP_CALL( removeAllActivationData(scip, activationhdlrdata->consmap) );
   SCIP_CALL( SCIPhashmapRemoveAll(activationhdlrdata->consmap) );

   SCIPhashmapFree(&activationhdlrdata->consmap);

   SCIPfreeMemory(scip, &activationhdlrdata);

   SCIPactivationhdlrSetData(activationhdlr, NULL);

   return SCIP_OKAY;
}

/* finds the sub-symmetries and return the found sub-symmetric submatrices */
static
SCIP_DECL_ACTIVATIONFINDDATA(activationFindActivationDataMakespan)
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;
   ACTIVATIONDATA* adata;
   SCIP_CONS* origcons;
   SCIP_ACTIVATIONSUBMATRIX* submatrix;

   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(cons != NULL);
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   /* get original constraint */
   origcons = cons;

   if( !SCIPconsIsOriginal(origcons) )
      origcons = origcons->transorigcons;

   /* get activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   /* get variable fixings for this constraint */
   adata = (ACTIVATIONDATA*)SCIPhashmapGetImage(activationhdlrdata->consmap, origcons);

   /* if not found, then we do not activate the constraint and leave result NULL */
   if( adata == NULL )
      return SCIP_OKAY;

   SCIP_CALL( findSubSymmetries(scip, adata, &submatrix) );

   *activationdata = (void*)submatrix;

   return SCIP_OKAY;
}

/*
 * Activation handler specific interface methods
 */

/** adds variables to the makespan activation handler */
SCIP_RETCODE SCIPregisterConsActivationMakespan(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_VAR***           matrix,             /**< matrix of scheduling variables */
   int*                  jobtimes,           /**< job processing times */
   int                   nmachines,          /**< number of machines (columns in the matrix) */
   int                   njobs               /**< number of jobs (rows in the matrix) */
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

   SCIP_CALL( storeConsActivationData(scip, activationhdlr, cons, matrix, jobtimes, nmachines, njobs) );

   return SCIP_OKAY;
}

/** creates the makespan activation handler and includes it in SCIP */
SCIP_RETCODE SCIPincludeActivationMakespan(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata = NULL;

   /* create variable fixings activation handler data */
   SCIP_CALL( createActivationhdlrData(scip, &activationhdlrdata) );

   SCIP_CALL( SCIPincludeActivationhdlr(scip, ACTIVATION_NAME, ACTIVATION_DESC, NULL,
         activationFreeMakespan, activationInitMakespan, NULL, NULL, activationFindActivationDataMakespan, activationhdlrdata) );

   return SCIP_OKAY;
}
