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

#include "scip/nodesel_dfs.h"
#include "scip/cons_integral.h"

/** execution method of presolver */
static
SCIP_DECL_PRESOLEXEC(presolExecTest)
{  /*lint --e{715}*/
   SCIP_CALL( SCIPinterruptSolve(scip) );
   *result = SCIP_DIDNOTRUN;
   return SCIP_OKAY;
}

/** execution method of primal heuristic */
static
SCIP_DECL_HEUREXEC(heurExecTest)
{  /*lint --e{715}*/
   SCIP_CALL( SCIPinterruptSolve(scip) );
   *result = SCIP_DIDNOTRUN;
   return SCIP_OKAY;
}

/** reduced cost pricing method of variable pricer for feasible LPs */
static
SCIP_DECL_PRICERREDCOST(pricerRedcostTest)
{  /*lint --e{715}*/
   *result = SCIP_SUCCESS;

   return SCIP_OKAY;
}

/* it can be called in SCIP_STAGE_PROBLEM and can get to
 *  SCIP_STAGE_TRANSFORMED
 *  SCIP_STAGE_PRESOLVING
 *  SCIP_STAGE_PRESOLVED
 *  SCIP_STAGE_SOLVING
 *  SCIP_STAGE_SOLVED
 *
 *  If stage == SCIP_STAGE_SOLVING and enableNLP is true, then SCIP will build its NLP
 */
SCIP_RETCODE TESTscipSetStage(SCIP* scip, SCIP_STAGE stage, SCIP_Bool enableNLP)
{
   /* no output nor warnings */
   /* @todo reset output to previous level! */
   SCIP_CALL( SCIPsetIntParam(scip, "display/verblevel", 0) );

   /* make sure that at least DFS is included; we need one node selector to call SCIPtransformProb, which is also called
    * by SCIP(pre)solve */
   if ( SCIPfindNodesel(scip, "dfs") == NULL )
   {
      SCIP_CALL( SCIPincludeNodeselDfs(scip) );
   }

   /* make sure that we have included the integral conshdlr to supress a warning */
   if ( SCIPfindConshdlr(scip, "integral") == NULL )
   {
      SCIP_CALL( SCIPincludeConshdlrIntegral(scip) );
   }

   /* SCIP can go to SOLVED after presolving if there are no vars, cons, nor pricer; we add a pricer to avoid this */
   switch( stage )
   {
      case SCIP_STAGE_PRESOLVED:
      case SCIP_STAGE_PRESOLVING:
      case SCIP_STAGE_SOLVING:
         SCIP_CALL( SCIPincludePricerBasic(scip, NULL, "pricerTest", "pricer to avoid SCIP skipping SOLVING", 0, FALSE,
                  pricerRedcostTest, NULL, NULL) );
         SCIP_CALL( SCIPactivatePricer(scip, SCIPfindPricer(scip, "pricerTest")) );
         break;

      default:
         break;
   }

   switch( stage )
   {
      case SCIP_STAGE_TRANSFORMED:
         SCIP_CALL( SCIPtransformProb(scip) );
         break;

      case SCIP_STAGE_PRESOLVED:
         SCIP_CALL( SCIPpresolve(scip) );
         break;

      case SCIP_STAGE_SOLVED:
         SCIP_CALL( SCIPsolve(scip) );
         break;

      case SCIP_STAGE_PRESOLVING:
         SCIP_CALL( SCIPincludePresolBasic(scip, NULL, "presolTest",
                  "Presol to stop in PRESOLVING", 1, -1,
                  SCIP_PRESOLTIMING_ALWAYS, presolExecTest, NULL) );
         SCIP_CALL( SCIPpresolve(scip) );
         break;

      case SCIP_STAGE_SOLVING:
         SCIP_CALL( SCIPincludeHeurBasic(scip, NULL,
                  "heurTest", "heuristic to stop in SOLVING", '!', 1, 1, 0,
                  -1, SCIP_HEURTIMING_BEFORENODE, FALSE, heurExecTest, NULL) );

         /* enable NLP */
         if( enableNLP )
         {
            SCIP_CALL( SCIPpresolve(scip) );
            SCIPenableNLP(scip);
         }

         SCIP_CALL( SCIPsolve(scip) );
         break;

      default:
         fprintf(stderr, "Not implemented yet!\n");
         return SCIP_ERROR;
   }

   return SCIP_OKAY;
}
