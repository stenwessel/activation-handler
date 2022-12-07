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

/**@file   activation_colorcomp.c
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
#include "scip/activation_colorcomp.h"

/* fundamental activation handler properties */
#define ACTIVATION_NAME        "colorcomp"
#define ACTIVATION_DESC        "color components activation handler"

#define SCIP_ACT_COLORCOMP_INVERTED1 1
#define SCIP_ACT_COLORCOMP_INVERTED2 2

/*
 * Data structures
 */
struct GraphData
{
   SCIP_VAR***           matrix;             /**< matrix of coloring variables (nvertices x ncolors) */
   int                   nvertices;          /**< number of vertices (rows in the matrix) */
   int                   ncolors;            /**< number of colors (columns in the matrix) */
   int**                 adjacencies;        /**< array that contains for every vertex an array of adjacent vertices */
   int*                  nadjacencies;       /**< array that contains for every vertex the number of adjacent vertices */
   SCIP_Bool             allcolorpairs;      /**< whether to consider all color pairs or only consecutive-color pairs */
   int                   strategy;      /**<  */
   SCIP_Bool*            removedvertices;
   int*                  dfsstack;
   int*                  component;
};
typedef struct GraphData GRAPHDATA;

struct SCIP_ActivationhdlrData
{
   SCIP_HASHMAP*         consmap;            /**< map of constraints to the graph data */
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

static
SCIP_RETCODE removeGraphData(
   SCIP*                 scip,               /**< SCIP data structure */
   GRAPHDATA**           graphdata           /**< pointer to graph data structure */
   )
{
   int i;

   assert(scip != NULL);
   assert(graphdata != NULL);
   assert(*graphdata != NULL);

   SCIPfreeBlockMemoryArray(scip, &(*graphdata)->component, (*graphdata)->nvertices);
   SCIPfreeBlockMemoryArray(scip, &(*graphdata)->dfsstack, (*graphdata)->nvertices);
   SCIPfreeBlockMemoryArray(scip, &(*graphdata)->removedvertices, (*graphdata)->nvertices);

   for( i = 0; i < (*graphdata)->nvertices; ++i )
   {
      SCIPfreeBlockMemoryArray(scip, &(*graphdata)->adjacencies[i], (*graphdata)->nadjacencies[i]);
   }
   SCIPfreeBlockMemoryArray(scip, &(*graphdata)->adjacencies, (*graphdata)->nvertices);
   SCIPfreeBlockMemoryArray(scip, &(*graphdata)->nadjacencies, (*graphdata)->nvertices);

   for( i = 0; i < (*graphdata)->nvertices; ++i )
   {
      SCIPfreeBlockMemoryArray(scip, &(*graphdata)->matrix[i], (*graphdata)->ncolors);
   }
   SCIPfreeBlockMemoryArray(scip, &(*graphdata)->matrix, (*graphdata)->nvertices);
   SCIPfreeBlockMemory(scip, graphdata);

   return SCIP_OKAY;
}

static
SCIP_RETCODE storeConsGraphData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ACTIVATIONHDLR*  activationhdlr,     /**< activation handler */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_VAR***           matrix,             /**< matrix of color variables (nvertices x ncolors) */
   int                   nvertices,          /**< number of rows in the matrix (number of vertices in the graph) */
   int                   ncolors,            /**< number of columns in the matrix (number of colors) */
   int**                 adjacencies,        /**< array that contains for every vertex an array of adjacent vertices */
   int*                  nadjacencies,       /**< array that contains for every vertex the number of adjacent vertices */
   SCIP_Bool             allcolorpairs,       /**< whether to consider all color pairs or only consecutive-color pairs */
   int                   strategy            /**< */
   )
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;
   GRAPHDATA* graphdata;
   int i;

   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(cons != NULL);
   assert(matrix != NULL);
   assert(nvertices > 0);
   assert(ncolors > 0);
   assert(adjacencies != NULL);
   assert(nadjacencies != NULL);

   /* get activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   /* if patterns already exist for this constraint, remove the old entry */
   graphdata = (GRAPHDATA*)SCIPhashmapGetImage(activationhdlrdata->consmap, cons);

   if( graphdata != NULL )
   {
      SCIP_CALL( removeGraphData(scip, &graphdata) );
   }
   assert(graphdata == NULL);

   /* create a new patterns structure */
   SCIP_CALL( SCIPallocBlockMemory(scip, &graphdata) );

   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &graphdata->matrix, nvertices) );
   for( i = 0; i < nvertices; ++i )
   {
      SCIP_CALL( SCIPduplicateBlockMemoryArray(scip, &graphdata->matrix[i], matrix[i], ncolors) );
   }

   SCIP_CALL(SCIPduplicateBlockMemoryArray(scip, &graphdata->nadjacencies, nadjacencies, nvertices));

   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &graphdata->adjacencies, nvertices) );
   for( i = 0; i < nvertices; ++i )
   {
      SCIP_CALL( SCIPduplicateBlockMemoryArray(scip, &graphdata->adjacencies[i], adjacencies[i], nadjacencies[i]) );
   }

   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &graphdata->removedvertices, nvertices) );
   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &graphdata->dfsstack, nvertices) );
   SCIP_CALL( SCIPallocBlockMemoryArray(scip, &graphdata->component, nvertices) );

   graphdata->nvertices = nvertices;
   graphdata->ncolors = ncolors;
   graphdata->allcolorpairs = allcolorpairs;
   graphdata->strategy = strategy;

   SCIP_CALL( SCIPhashmapInsert(activationhdlrdata->consmap, cons, (void*)graphdata) );

   SCIP_CALL( SCIPsetConsActivationhdlr(scip, cons, activationhdlr) );

   return SCIP_OKAY;
}

static
SCIP_RETCODE removeAllGraphData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HASHMAP*         hashmap             /**< hashmap containing graph data */
   )
{
   int nentries;
   int i;
   SCIP_HASHMAPENTRY* entry;
   GRAPHDATA* graphdata;

   assert(scip != NULL);
   assert(hashmap != NULL);

   nentries = SCIPhashmapGetNEntries(hashmap);

   for( i = 0; i < nentries; ++i )
   {
      entry = SCIPhashmapGetEntry(hashmap, i);

      if( entry != NULL )
      {
         graphdata = (GRAPHDATA*)SCIPhashmapEntryGetImage(entry);
         SCIP_CALL( removeGraphData(scip, &graphdata) );
      }
   }

   return SCIP_OKAY;
}

static
SCIP_RETCODE findColorPairsComponents(
      SCIP*                 scip,               /**< SCIP data structure */
      GRAPHDATA*            graphdata,          /**< graph data structure */
      SCIP_ACTIVATIONSUBMATRIX** submatrix      /**< pointer to the first submatrix to store the result */
   )
{
   int c1;
   int c2;
   int i;
   int j;
   int v;
   int startvertex;
   int dfsstackhead;
   int componenthead;
   SCIP_Bool atleastoneremoved;

   assert(scip != NULL);
   assert(graphdata != NULL);

   *submatrix = NULL;

   /* Iterate over color pairs */
   for( c1 = 0; c1 < graphdata->ncolors; ++c1 )
   {
      for( c2 = c1 + 1; c2 < graphdata->ncolors && (graphdata->allcolorpairs == FALSE ? (c2 < c1 + 2) : TRUE); ++c2 )
      {
         /* set vertices that are fixed to be not colored by c1 and c2 to be removed from the graph */
         atleastoneremoved = FALSE;
         for( i = 0; i < graphdata->nvertices; ++i )
         {
            if( SCIPvarGetUbLocal(graphdata->matrix[i][c1]) < 0.5 && SCIPvarGetUbLocal(graphdata->matrix[i][c2]) < 0.5 )
            {
               graphdata->removedvertices[i] = TRUE;
               atleastoneremoved = TRUE;
            }
            else
            {
               graphdata->removedvertices[i] = FALSE;
            }
         }

         /* If nothing was removed, we only find the original component (and then this is not a new sub-symmetry) */
         if( atleastoneremoved == FALSE )
            continue;

         /* Iteratively run DFS to find the components of the graph */
         for( startvertex = 0; startvertex < graphdata->nvertices; ++startvertex )
         {
            /* If the start vertex is already removed from the graph, continue */
            if( graphdata->removedvertices[startvertex] == TRUE )
               continue;

            /* Initialize stack and component buffers */
            dfsstackhead = -1;
            componenthead = -1;

            /* Mark the start vertex as visited (removed), and add it to the current component */
            graphdata->removedvertices[startvertex] = TRUE;
            graphdata->component[++componenthead] = startvertex;

            /* Push the start vertex onto the head of the stack */
            graphdata->dfsstack[++dfsstackhead] = startvertex;

            /* While the stack is not empty */
            while( dfsstackhead >= 0 )
            {
               /* Pop a vertex from the stack */
               i = graphdata->dfsstack[dfsstackhead--];

               /* Find an unvisited adjacent vertex */
               for( j = 0; j < graphdata->nadjacencies[i]; ++j )
               {
                  v = graphdata->adjacencies[i][j];
                  if( graphdata->removedvertices[v] == FALSE )
                  {
                     /* Push i back on the stack, mark v as visited and in the component. Also put it on the stack */
                     graphdata->dfsstack[++dfsstackhead] = i;

                     graphdata->removedvertices[v] = TRUE;
                     graphdata->component[++componenthead] = v;

                     graphdata->dfsstack[++dfsstackhead] = v;
                     break;
                  }
               }
            }

            /* If the component is the whole graph, this is not a new sub-symmetry -> skip the iterative DFS */
            if( componenthead >= graphdata->nvertices - 1 )
            {
               startvertex = graphdata->nvertices;
               continue;
            }

            /* Record this component as a sub-symmetry */
            SCIP_ACTIVATIONSUBMATRIX* subm;
            SCIP_CALL( SCIPallocBlockMemory(scip, &subm) );

            subm->orbitopetype = SCIP_ORBITOPETYPE_PACKING;
            subm->ncols = 2;
            subm->nrows = componenthead + 1;

            SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->cols, subm->ncols) );
            subm->cols[0] = c1;
            subm->cols[1] = c2;

            SCIP_CALL( SCIPduplicateBlockMemoryArray(scip, &subm->rows, graphdata->component, subm->nrows) );

            /* Sorting is necessary such that the vertex order is the same order as the rows of the main orbitope */
            SCIPsortInt(subm->rows, subm->nrows);

            subm->next = *submatrix;
            *submatrix = subm;
         }
      }
   }

   return SCIP_OKAY;
}


static
SCIP_RETCODE findInvertedColorPairsComponents(
      SCIP*                 scip,               /**< SCIP data structure */
      GRAPHDATA*            graphdata,          /**< graph data structure */
      SCIP_ACTIVATIONSUBMATRIX** submatrix      /**< pointer to the first submatrix to store the result */
   )
{
   int c1;
   int c2;
   int c3;
   int i;
   int j;
   int v;
   int startvertex;
   int dfsstackhead;
   int componenthead;
   SCIP_Bool atleastoneremoved;

   assert(scip != NULL);
   assert(graphdata != NULL);

   *submatrix = NULL;

   /* Iterate over color pairs */
   for( c1 = 0; c1 < graphdata->ncolors; ++c1 )
   {
      for( c2 = c1 + 1; c2 < graphdata->ncolors && (graphdata->allcolorpairs == FALSE ? (c2 < c1 + 2) : TRUE); ++c2 )
      {
         /* set vertices that are fixed to be colored by c1 or c2 to be removed from the graph */
         atleastoneremoved = FALSE;
         for( i = 0; i < graphdata->nvertices; ++i )
         {
            if( SCIPvarGetLbLocal(graphdata->matrix[i][c1]) > 0.5 || SCIPvarGetLbLocal(graphdata->matrix[i][c2]) > 0.5 )
            {
               graphdata->removedvertices[i] = TRUE;
               atleastoneremoved = TRUE;
            }
            else
            {
               graphdata->removedvertices[i] = FALSE;
            }
         }

         /* If nothing was removed, we only find the original component (and then this is not a new sub-symmetry) */
         if( atleastoneremoved == FALSE )
            continue;

         /* Iteratively run DFS to find the components of the graph */
         for( startvertex = 0; startvertex < graphdata->nvertices; ++startvertex )
         {
            /* If the start vertex is already removed from the graph, continue */
            if( graphdata->removedvertices[startvertex] == TRUE )
               continue;

            /* Initialize stack and component buffers */
            dfsstackhead = -1;
            componenthead = -1;

            /* Mark the start vertex as visited (removed), and add it to the current component */
            graphdata->removedvertices[startvertex] = TRUE;
            graphdata->component[++componenthead] = startvertex;

            /* Push the start vertex onto the head of the stack */
            graphdata->dfsstack[++dfsstackhead] = startvertex;

            /* While the stack is not empty */
            while( dfsstackhead >= 0 )
            {
               /* Pop a vertex from the stack */
               i = graphdata->dfsstack[dfsstackhead--];

               /* Find an unvisited adjacent vertex */
               for( j = 0; j < graphdata->nadjacencies[i]; ++j )
               {
                  v = graphdata->adjacencies[i][j];
                  if( graphdata->removedvertices[v] == FALSE )
                  {
                     /* Push i back on the stack, mark v as visited and in the component. Also put it on the stack */
                     graphdata->dfsstack[++dfsstackhead] = i;

                     graphdata->removedvertices[v] = TRUE;
                     graphdata->component[++componenthead] = v;

                     graphdata->dfsstack[++dfsstackhead] = v;
                     break;
                  }
               }
            }

            /* Record this component as a sub-symmetry */
            SCIP_ACTIVATIONSUBMATRIX* subm;
            SCIP_CALL( SCIPallocBlockMemory(scip, &subm) );

            subm->orbitopetype = SCIP_ORBITOPETYPE_PACKING;
            subm->ncols = graphdata->ncolors - 2;
            subm->nrows = componenthead + 1;

            SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->cols, subm->ncols) );
            j = 0;
            for( c3 = 0; c3 < graphdata->ncolors; ++c3 )
            {
               if( c3 != c1 && c3 != c2 )
                  subm->cols[j++] = c3;
            }

            SCIP_CALL( SCIPduplicateBlockMemoryArray(scip, &subm->rows, graphdata->component, subm->nrows) );

            /* Sorting is necessary such that the vertex order is the same order as the rows of the main orbitope */
            SCIPsortInt(subm->rows, subm->nrows);

            subm->next = *submatrix;
            *submatrix = subm;
         }
      }
   }

   return SCIP_OKAY;
}


static
SCIP_RETCODE findInvertedSingleColorComponents(
      SCIP*                 scip,               /**< SCIP data structure */
      GRAPHDATA*            graphdata,          /**< graph data structure */
      SCIP_ACTIVATIONSUBMATRIX** submatrix      /**< pointer to the first submatrix to store the result */
   )
{
   int c1;
   int c2;
   int i;
   int j;
   int v;
   int startvertex;
   int dfsstackhead;
   int componenthead;
   SCIP_Bool atleastoneremoved;

   assert(scip != NULL);
   assert(graphdata != NULL);

   *submatrix = NULL;

   /* Iterate over colors */
   for( c1 = 0; c1 < graphdata->ncolors; ++c1 )
   {
      /* set vertices that are fixed to be colored by c1 to be removed from the graph */
      atleastoneremoved = FALSE;
      for( i = 0; i < graphdata->nvertices; ++i )
      {
         if( SCIPvarGetLbLocal(graphdata->matrix[i][c1]) > 0.5 )
         {
            graphdata->removedvertices[i] = TRUE;
            atleastoneremoved = TRUE;
         }
         else
         {
            graphdata->removedvertices[i] = FALSE;
         }
      }

      /* If nothing was removed, we only find the original component (and then this is not a new sub-symmetry) */
      if( atleastoneremoved == FALSE )
         continue;

      /* Iteratively run DFS to find the components of the graph */
      for( startvertex = 0; startvertex < graphdata->nvertices; ++startvertex )
      {
         /* If the start vertex is already removed from the graph, continue */
         if( graphdata->removedvertices[startvertex] == TRUE )
            continue;

         /* Initialize stack and component buffers */
         dfsstackhead = -1;
         componenthead = -1;

         /* Mark the start vertex as visited (removed), and add it to the current component */
         graphdata->removedvertices[startvertex] = TRUE;
         graphdata->component[++componenthead] = startvertex;

         /* Push the start vertex onto the head of the stack */
         graphdata->dfsstack[++dfsstackhead] = startvertex;

         /* While the stack is not empty */
         while( dfsstackhead >= 0 )
         {
            /* Pop a vertex from the stack */
            i = graphdata->dfsstack[dfsstackhead--];

            /* Find an unvisited adjacent vertex */
            for( j = 0; j < graphdata->nadjacencies[i]; ++j )
            {
               v = graphdata->adjacencies[i][j];
               if( graphdata->removedvertices[v] == FALSE )
               {
                  /* Push i back on the stack, mark v as visited and in the component. Also put it on the stack */
                  graphdata->dfsstack[++dfsstackhead] = i;

                  graphdata->removedvertices[v] = TRUE;
                  graphdata->component[++componenthead] = v;

                  graphdata->dfsstack[++dfsstackhead] = v;
                  break;
               }
            }
         }

         /* Record this component as a sub-symmetry */
         SCIP_ACTIVATIONSUBMATRIX* subm;
         SCIP_CALL( SCIPallocBlockMemory(scip, &subm) );

         subm->orbitopetype = SCIP_ORBITOPETYPE_PACKING;
         subm->ncols = graphdata->ncolors - 1;
         subm->nrows = componenthead + 1;

         SCIP_CALL( SCIPallocBlockMemoryArray(scip, &subm->cols, subm->ncols) );
         j = 0;
         for( c2 = 0; c2 < graphdata->ncolors; ++c2 )
         {
            if( c2 != c1 )
               subm->cols[j++] = c2;
         }

         SCIP_CALL( SCIPduplicateBlockMemoryArray(scip, &subm->rows, graphdata->component, subm->nrows) );

         /* Sorting is necessary such that the vertex order is the same order as the rows of the main orbitope */
         SCIPsortInt(subm->rows, subm->nrows);

         subm->next = *submatrix;
         *submatrix = subm;
      }
   }

   return SCIP_OKAY;
}


/*
 * Callback methods of activation handler
 */

/** copy method for activation handler plugins (called when SCIP copies plugins) */
static
SCIP_DECL_ACTIVATIONCOPY(activationCopyColorComp)
{
   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   /* call inclusion method of activation handler */
   SCIP_CALL( SCIPincludeActivationColorComp(scip) );

   // TODO: copy data! (?)

   return SCIP_OKAY;
}

/** sets destructor method of activation handler */
static
SCIP_DECL_ACTIVATIONFREE(activationFreeColorComp)
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;

   assert(scip != NULL);
   assert(activationhdlr != NULL);
   assert(strcmp(SCIPactivationhdlrGetName(activationhdlr), ACTIVATION_NAME) == 0);

   /* free activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   /* free variable fixings in consmap */
   SCIP_CALL( removeAllGraphData(scip, activationhdlrdata->consmap) );
   SCIP_CALL( SCIPhashmapRemoveAll(activationhdlrdata->consmap) );

   SCIPhashmapFree(&activationhdlrdata->consmap);

   SCIPfreeMemory(scip, &activationhdlrdata);

   SCIPactivationhdlrSetData(activationhdlr, NULL);

   return SCIP_OKAY;
}

/** is active method of activation handler */
static
SCIP_DECL_ACTIVATIONFINDDATA(activationFindActivationDataColorComp)
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;
   GRAPHDATA* graphdata;
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
   graphdata = (GRAPHDATA*)SCIPhashmapGetImage(activationhdlrdata->consmap, origcons);

   /* if not found, then we do not activate the constraint and leave result NULL */
   if( graphdata == NULL )
   {
      return SCIP_OKAY;
   }

   SCIP_ACTIVATIONSUBMATRIX* submatrix;
   if( graphdata->strategy == SCIP_ACT_COLORCOMP_INVERTED1 ) {
      SCIP_CALL( findInvertedSingleColorComponents(scip, graphdata, &submatrix) );
   }
   else if( graphdata->strategy == SCIP_ACT_COLORCOMP_INVERTED2 ) {
      SCIP_CALL( findInvertedColorPairsComponents(scip, graphdata, &submatrix) );
   }
   else {
      SCIP_CALL( findColorPairsComponents(scip, graphdata, &submatrix) );
   }

   *activationdata = (void*)submatrix;

   return SCIP_OKAY;
}


static
SCIP_DECL_ACTIVATIONINIT(activationInitColorComp)
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata;
   GRAPHDATA* graphdata;
   SCIP_HASHMAPENTRY* entry;
   int nentries;
   int i;
   int j;
   int m;

   if( !SCIPisTransformed(scip) )
   {
      return SCIP_OKAY;
   }

   // Transform the variables

   /* get activation handler data */
   activationhdlrdata = SCIPactivationhdlrGetData(activationhdlr);
   assert(activationhdlrdata != NULL);

   nentries = SCIPhashmapGetNEntries(activationhdlrdata->consmap);

   for( i = 0; i < nentries; ++i )
   {
      entry = SCIPhashmapGetEntry(activationhdlrdata->consmap, i);

      if( entry != NULL )
      {
         graphdata = (GRAPHDATA*)SCIPhashmapEntryGetImage(entry);

         for( j = 0; j < graphdata->nvertices; ++j )
         {
            for( m = 0; m < graphdata->ncolors; ++m )
            {
               SCIP_CALL( SCIPgetTransformedVar(scip, graphdata->matrix[j][m], &(graphdata->matrix[j][m])) );
            }
         }
      }
   }

   return SCIP_OKAY;
}



/*
 * Activation handler specific interface methods
 */

/** registers orbitope constraint to use this activation handler
 */
SCIP_RETCODE SCIPregisterConsActivationColorComp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_VAR***           matrix,             /**< matrix of color variables (nvertices x ncolors) */
   int                   nvertices,          /**< number of rows in the matrix (number of vertices in the graph) */
   int                   ncolors,            /**< number of columns in the matrix (number of colors) */
   int**                 adjacencies,        /**< array that contains for every vertex an array of adjacent vertices */
   int*                  nadjacencies,       /**< array that contains for every vertex the number of adjacent vertices */
   SCIP_Bool             allcolorpairs,      /**< whether to consider all color pairs or only consecutive-color pairs */
   int                   strategy            /**< component-finding strategy */
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

   SCIP_CALL( storeConsGraphData(scip, activationhdlr, cons, matrix, nvertices, ncolors, adjacencies, nadjacencies, allcolorpairs, strategy) );

   return SCIP_OKAY;
}

/** creates the sub-orbitope activation handler and includes it in SCIP
 */
SCIP_RETCODE SCIPincludeActivationColorComp(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_ACTIVATIONHDLRDATA* activationhdlrdata = NULL;

   /* create activation handler data */
   SCIP_CALL( createActivationhdlrData(scip, &activationhdlrdata) );

   SCIP_CALL( SCIPincludeActivationhdlr(scip, ACTIVATION_NAME, ACTIVATION_DESC, NULL,
         activationFreeColorComp, activationInitColorComp, NULL, NULL, activationFindActivationDataColorComp, activationhdlrdata) );

   return SCIP_OKAY;
}
