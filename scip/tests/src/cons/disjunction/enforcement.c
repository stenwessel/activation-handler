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

/**@file   enforcement.c
 * @brief  unit test that checks that the enforcement of the disjunction constraint handler works correctly
 * @author Benjamin Mueller
 */

#include "scip/scip.h"
#include "include/scip_test.h"
#include "scip/scipdefplugins.h"
#include <stdio.h>

/* TESTS  */
Test(disjunction, enforcement)
{
   SCIP* scip;
   SCIP_VAR* lambdas[9];
   SCIP_CONS* cons;
   char name[SCIP_MAXSTRLEN];
   int i;
   int k;

   /* create an empty problem */
   SCIP_CALL( SCIPcreate(&scip) );
   SCIP_CALL( SCIPincludeDefaultPlugins(scip) );
   SCIP_CALL( SCIPcreateProbBasic(scip, "disjunction_enforcement") );

   /* create variables */
   for( i = 0; i < 9; ++i )
   {
      (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "lambda_%d", i);
      SCIP_CALL( SCIPcreateVarBasic(scip, &lambdas[i], name, 0.0, 1.0, (i == 8) ? -1.0 : 0.0, SCIP_VARTYPE_CONTINUOUS) );
      SCIP_CALL( SCIPaddVar(scip, lambdas[i]) );
   }

   /* create linear constraints */
   for( k = 0; k < 2; ++k )
   {
      SCIP_CALL( SCIPcreateConsBasicLinear(scip, &cons, "reg", 0, NULL, NULL, 0.0, 1.0) );

      for( i = 0; i < 4; ++i )
      {
         SCIP_CALL( SCIPaddCoefLinear(scip, cons, lambdas[4 * k + i], 1.0) );
      }

      SCIP_CALL( SCIPaddCons(scip, cons) );
      SCIP_CALL( SCIPreleaseCons(scip, &cons) );
   }

   /* create first disjunction constraint */
   {
      SCIP_CONS* conss[2];
      SCIP_Real vals[5] = {1.0, -0.5, 1.0, -0.5, -1.0};
      SCIP_VAR*vars1[5] = {lambdas[0], lambdas[1], lambdas[2], lambdas[3], lambdas[8]};
      SCIP_VAR* vars2[5] = {lambdas[4], lambdas[5], lambdas[6], lambdas[7], lambdas[8]};

      SCIP_CALL( SCIPcreateConsBasicLinear(scip, &conss[0], "A1", 5, vars1, vals, 0.0, SCIPinfinity(scip)) );
      SCIP_CALL( SCIPcreateConsBasicLinear(scip, &conss[1], "A2", 5, vars2, vals, 0.0, SCIPinfinity(scip)) );
      SCIP_CALL( SCIPcreateConsBasicDisjunction(scip, &cons, "disj1", 2, conss, NULL) );
      SCIP_CALL( SCIPaddCons(scip, cons) );
      SCIP_CALL( SCIPreleaseCons(scip, &cons) );

      /* release linear constraints*/
      SCIP_CALL( SCIPreleaseCons(scip, &conss[0]) );
      SCIP_CALL( SCIPreleaseCons(scip, &conss[1]) );
   }

   /* create second disjunction constraint */
   {
      SCIP_CONS* conss[2];
      SCIP_Real vals[5] = {-1.0, 1.3, -0.25, 0.475, -1.0};
      SCIP_VAR* vars1[5] = {lambdas[0], lambdas[1], lambdas[2], lambdas[3], lambdas[8]};
      SCIP_VAR* vars2[5] = {lambdas[4], lambdas[5], lambdas[6], lambdas[7], lambdas[8]};

      SCIP_CALL( SCIPcreateConsBasicLinear(scip, &conss[0], "B1", 5, vars1, vals, 0.0, SCIPinfinity(scip)) );
      SCIP_CALL( SCIPcreateConsBasicLinear(scip, &conss[1], "B2", 5, vars2, vals, 0.0, SCIPinfinity(scip)) );
      SCIP_CALL( SCIPcreateConsBasicDisjunction(scip, &cons, "disj2", 2, conss, NULL) );
      SCIP_CALL( SCIPaddCons(scip, cons) );
      SCIP_CALL( SCIPreleaseCons(scip, &cons) );

      /* release linear constraints*/
      SCIP_CALL( SCIPreleaseCons(scip, &conss[0]) );
      SCIP_CALL( SCIPreleaseCons(scip, &conss[1]) );
   }

   /* solve problem */
   SCIP_CALL( SCIPprintOrigProblem(scip, NULL, "cip", FALSE) );
   SCIP_CALL( SCIPsolve(scip) );

   cr_expect(SCIPisFeasEQ(scip, SCIPgetPrimalbound(scip), -1.0));

   /* release variables */
   for( i = 0; i < 9; ++i )
   {
      SCIP_CALL( SCIPreleaseVar(scip , &lambdas[i]) );
   }

   /* release SCIP */
   SCIP_CALL( SCIPfree(&scip ) );
}
