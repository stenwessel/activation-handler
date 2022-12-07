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

/**@file   objcutsel.h
 * @brief  C++ wrapper for cut selectors
 * @author Felipe Serrano
 * @author Mark Turner
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_OBJCUTSEL_H__
#define __SCIP_OBJCUTSEL_H__

#include <cstring>

#include "scip/scip.h"
#include "objscip/objcloneable.h"

namespace scip
{

/** @brief C++ wrapper for cut selectors
 *
 *  This class defines the interface for cut selectors implemented in C++.
 *
 *  - \ref CUTSEL "Instructions for implementing a cut selector"
 *  - \ref CUTSELECTORS "List of available cut selectors"
 *  - \ref type_cutsel.h "Corresponding C interface"
 */
class ObjCutsel : public ObjCloneable
{
public:
   /*lint --e{1540}*/

   /** SCIP data structure */
   SCIP* scip_;

   /** name of the cut selector */
   char* scip_name_;

   /** description of the cut selector */
   char* scip_desc_;

   /** priority of the cut selector */
   const int scip_priority_;

   /** default constructor */
   ObjCutsel(
      SCIP*              scip,               /**< SCIP data structure */
      const char*        name,               /**< name of cut selector */
      const char*        desc,               /**< description of cut selector */
      int                priority            /**< priority of the cut */
      )
      : scip_(scip),
        scip_name_(0),
        scip_desc_(0),
        scip_priority_(priority)
   {
      SCIP_CALL_ABORT( SCIPduplicateMemoryArray(scip_, &scip_name_, name, std::strlen(name)+1) );
      SCIP_CALL_ABORT( SCIPduplicateMemoryArray(scip_, &scip_desc_, desc, std::strlen(desc)+1) );
   }

   /** destructor */
   virtual ~ObjCutsel()
   {
      /*lint --e{64}*/
      SCIPfreeMemoryArray(scip_, &scip_name_);
      SCIPfreeMemoryArray(scip_, &scip_desc_);
   }

   /** destructor of cut selector to free user data (called when SCIP is exiting)
    *
    *  @see SCIP_DECL_CUTSELFREE(x) in @ref type_cutsel.h
    */
   virtual SCIP_DECL_CUTSELFREE(scip_free)
   {  /*lint --e{715}*/
      return SCIP_OKAY;
   }

   /** initialization method of cut selector (called after problem was transformed)
    *
    *  @see SCIP_DECL_CUTSELINIT(x) in @ref type_cutsel.h
    */
   virtual SCIP_DECL_CUTSELINIT(scip_init)
   {  /*lint --e{715}*/
      return SCIP_OKAY;
   }

   /** deinitialization method of cut selector (called before transformed problem is freed)
    *
    *  @see SCIP_DECL_CUTSELEXIT(x) in @ref type_cutsel.h
    */
   virtual SCIP_DECL_CUTSELEXIT(scip_exit)
   {  /*lint --e{715}*/
      return SCIP_OKAY;
   }

   /** solving process initialization method of cut selector (called when branch and bound process is about to begin)
    *
    *  @see SCIP_DECL_CUTSELINITSOL(x) in @ref type_cutsel.h
    */
   virtual SCIP_DECL_CUTSELINITSOL(scip_initsol)
   {  /*lint --e{715}*/
      return SCIP_OKAY;
   }

   /** solving process deinitialization method of cut selector (called before branch and bound process data is freed)
    *
    *  @see SCIP_DECL_CUTSELEXITSOL(x) in @ref type_cutsel.h
    */
   virtual SCIP_DECL_CUTSELEXITSOL(scip_exitsol)
   {  /*lint --e{715}*/
      return SCIP_OKAY;
   }

   /** cut selection method of cut selector
    *
    *  @see SCIP_DECL_CUTSELSELECT(x) in @ref type_cutsel.h
    */
   virtual SCIP_DECL_CUTSELSELECT(scip_select) = 0;
};

} /* namespace scip */



/** creates the cut selector for the given cut selector object and includes it in SCIP
 *
 *  The method should be called in one of the following ways:
 *
 *   1. The user is responsible for deleting the object:
 *       SCIP_CALL( SCIPcreate(&scip) );
 *       ...
 *       MyCutsel* mycutsel = new MyCutsel(...);
 *       SCIP_CALL( SCIPincludeObjCutsel(scip, &mycutsel, FALSE) );
 *       ...
 *       SCIP_CALL( SCIPfree(&scip) );
 *       delete mycutsel;    // delete cutsel AFTER SCIPfree() !
 *
 *   2. The object pointer is passed to SCIP and deleted by SCIP in the SCIPfree() call:
 *       SCIP_CALL( SCIPcreate(&scip) );
 *       ...
 *       SCIP_CALL( SCIPincludeObjCutsel(scip, new MyCutsel(...), TRUE) );
 *       ...
 *       SCIP_CALL( SCIPfree(&scip) );  // destructor of MyCutsel is called here
 */
SCIP_EXPORT
SCIP_RETCODE SCIPincludeObjCutsel(
   SCIP*                 scip,               /**< SCIP data structure */
   scip::ObjCutsel*      objcutsel,          /**< cut selector object */
   SCIP_Bool             deleteobject        /**< should the cut selector object be deleted when cut selector is freed? */
   );

/** returns the cutsel object of the given name, or 0 if not existing */
SCIP_EXPORT
scip::ObjCutsel* SCIPfindObjCutsel(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           name                /**< name of cut selector */
   );

/** returns the cutsel object for the given cut selector */
SCIP_EXPORT
scip::ObjCutsel* SCIPgetObjCutsel(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CUTSEL*          cutsel              /**< cut selector */
   );

#endif
