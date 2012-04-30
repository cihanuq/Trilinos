// @HEADER
// ***********************************************************************
//
//           Panzer: A partial differential equation assembly
//       engine for strongly coupled complex multiphysics systems
//                 Copyright (2011) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Roger P. Pawlowski (rppawlo@sandia.gov) and
// Eric C. Cyr (eccyr@sandia.gov)
// ***********************************************************************
// @HEADER

#ifndef __Panzer_EpetraLinearObjContainer_hpp__
#define __Panzer_EpetraLinearObjContainer_hpp__

#include "Panzer_config.hpp"

#include <map>

// Epetra includes
#include "Epetra_Vector.h"
#include "Epetra_CrsMatrix.h"

#include "Panzer_LinearObjFactory.hpp"

#include "Teuchos_RCP.hpp"

#include "Thyra_VectorBase.hpp"
#include "Thyra_LinearOpBase.hpp"

namespace panzer {

class EpetraLinearObjContainer : public LinearObjContainer {
public:
   typedef LinearObjContainer::Members Members;

   typedef Epetra_Vector VectorType;
   typedef Epetra_CrsMatrix CrsMatrixType;

   virtual void initialize() 
   {
      if(get_x()!=Teuchos::null) get_x()->PutScalar(0.0);
      if(get_dxdt()!=Teuchos::null) get_dxdt()->PutScalar(0.0);
      if(get_f()!=Teuchos::null) get_f()->PutScalar(0.0);
      if(get_A()!=Teuchos::null) get_A()->PutScalar(0.0);
   }

   //! Wipe out stored data.
   void clear()
   {
      set_x(Teuchos::null);
      set_dxdt(Teuchos::null);
      set_f(Teuchos::null);
      set_A(Teuchos::null);
   }
   
   inline void set_x(const Teuchos::RCP<Epetra_Vector> & in) { x = in; } 
   inline const Teuchos::RCP<Epetra_Vector> get_x() const { return x; }

   inline void set_dxdt(const Teuchos::RCP<Epetra_Vector> & in) { dxdt = in; } 
   inline const Teuchos::RCP<Epetra_Vector> get_dxdt() const { return dxdt; }

   inline void set_f(const Teuchos::RCP<Epetra_Vector> & in) { f = in; } 
   inline const Teuchos::RCP<Epetra_Vector> get_f() const { return f; }

   inline void set_A(const Teuchos::RCP<Epetra_CrsMatrix> & in) { A = in; } 
   inline const Teuchos::RCP<Epetra_CrsMatrix> get_A() const { return A; }

private:
   Teuchos::RCP<Epetra_Vector> x, dxdt, f;
   Teuchos::RCP<Epetra_CrsMatrix> A;
};

}

#endif
