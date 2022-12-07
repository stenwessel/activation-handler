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
/*  You should have received a copy of the ZIB Academic License.             */
/*  along with SCIP; see the file COPYING. If not visit scipopt.org.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   ProbDataTSP.h
 * @brief  C++ problem data for TSP
 * @author Timo Berthold
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __TSPPROBDATA_H__
#define __TSPPROBDATA_H__

#include "objscip/objscip.h"
#include "GomoryHuTree.h"

namespace tsp
{

/** SCIP user problem data for TSP */
class ProbDataTSP : public scip::ObjProbData
{
   GRAPH*                graph_;             /**< graph data */

public:

   /** default constructor */
   ProbDataTSP(
      GRAPH*             g                   /**< graph data */
      )
      : graph_(g)
   {
      capture_graph(graph_);
   }

   /** destructor */
   virtual ~ProbDataTSP()
   {
      if( graph_ != NULL )
         release_graph(&graph_); /*lint !e1551*/
   }

   /** Copies user data if you want to copy it to a subscip */
   virtual SCIP_RETCODE scip_copy(
      SCIP*              scip,               /**< SCIP data structure */
      SCIP*              sourcescip,         /**< source SCIP main data structure */
      SCIP_HASHMAP*      varmap,             /**< a hashmap which stores the mapping of source variables to
                                              * corresponding target variables */
      SCIP_HASHMAP*      consmap,            /**< a hashmap which stores the mapping of source contraints to
                                              * corresponding target constraints */
      ObjProbData**      objprobdata,        /**< pointer to store the copied problem data object */
      SCIP_Bool          global,             /**< create a global or a local copy? */
      SCIP_RESULT*       result              /**< pointer to store the result of the call */
      );

   /** destructor of user problem data to free original user data (called when original problem is freed) */
   virtual SCIP_RETCODE scip_delorig(
      SCIP*              scip                /**< SCIP data structure */
      );

   /** destructor of user problem data to free transformed user data (called when transformed problem is freed) */
   virtual SCIP_RETCODE scip_deltrans(
      SCIP*              scip                /**< SCIP data structure */
      );

   /** creates user data of transformed problem by transforming the original user problem data
    *  (called after problem was transformed)
    */
   virtual SCIP_RETCODE scip_trans(
      SCIP*              scip,               /**< SCIP data structure */
      ObjProbData**      objprobdata,        /**< pointer to store the transformed problem data object */
      SCIP_Bool*         deleteobject        /**< pointer to store whether SCIP should delete the object after solving */
      );

   /* get the graph */
   GRAPH* getGraph()
   {
      return graph_;
   }

};/*lint !e1712*/

} /* namespace tsp */

#endif
