//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   Alea/mc_solvers/AdditiveSchwarzWrapper.hh
 * \author Steven Hamilton
 * \brief  Wrap a Belos solver into an operator.
 */
//---------------------------------------------------------------------------//

#ifndef Alea_mc_solvers_AdditiveSchwarzWrapper_hh
#define Alea_mc_solvers_AdditiveSchwarzWrapper_hh

#include "AleaSolver.hh"

#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"

#include "AleaTypedefs.hh"

#include "Ifpack2_AdditiveSchwarz.hpp"

namespace alea
{

//---------------------------------------------------------------------------//
/*!
 * \class AdditiveSchwarzWrapper
 * \brief Wrap an Ifpack2 AdditiveSchwarz object into an AleaSolver
 */
//---------------------------------------------------------------------------//
class AdditiveSchwarzWrapper : public AleaSolver
{
  public:

    AdditiveSchwarzWrapper(Teuchos::RCP<const MATRIX> A,
        Teuchos::RCP<Ifpack2::Preconditioner<SCALAR,LO,GO,NODE> > prec,
        Teuchos::RCP<Teuchos::ParameterList> pl);

  private:

    // Implementation of apply
    void applyImpl(const MV &x, MV &y) const;

    // Belos solver
    Teuchos::RCP<Ifpack2::AdditiveSchwarz<MATRIX> > d_schwarz;
};

}

#endif // Alea_mc_solvers_AdditiveSchwarzWrapper_hh

