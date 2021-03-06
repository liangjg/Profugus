//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   SPn/solvers/EigenvalueSolverBuilder.t.hh
 * \author Thomas M. Evans, Steven Hamilton
 * \date   Mon Feb 24 13:49:22 2014
 * \brief  EigenvalueSolverBuilder template member definitions.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef SPn_solvers_EigenvalueSolverBuilder_t_hh
#define SPn_solvers_EigenvalueSolverBuilder_t_hh

#include <iostream>

#include "PowerIteration.hh"
#include "Arnoldi.hh"
#include "Davidson_Eigensolver.hh"
#include "RayleighQuotient.hh"
#include "InverseOperator.hh"
#include "ShiftedInverseOperator.hh"
#include "EigenvalueSolverBuilder.hh"

namespace profugus
{

//---------------------------------------------------------------------------//
/*!
 * \brief Build a profugus EigenvalueSolver for standard eigenvalue problem.
 *
 * This function creates a EigenvalueSolver object from a given database
 * and a single Epetra operator.  The type of solver to be constructed is
 * determined by the database entry "eigensolver", which can be "Power",
 * "Arnoldi".
 */
template <class T>
Teuchos::RCP<EigenvalueSolver<T> >
EigenvalueSolverBuilder<T>::build_solver( RCP_ParameterList db,
                                          Teuchos::RCP<OP>  A)
{
    RCP_EigenvalueSolver solver;

    // Determine type of solver to be constructed.
    std::string eigensolver = lower(
        db->get("eigensolver", std::string("Arnoldi")));

    if( eigensolver=="arnoldi" )
    {
        solver = Teuchos::rcp(new Arnoldi<T>(db));
        solver->set_operator(A);
    }
    else if( eigensolver=="power" )
    {
        solver = Teuchos::rcp(new PowerIteration<T>(db));
        solver->set_operator(A);
    }
    else
    {
        VALIDATE(false,
                  "Error: Invalid EigenvalueSolver option." << std::endl
                 << " Specify eigenvalue solver by setting eigensolver="
                 << "'arnoldi', 'power'"
                 << std::endl);
    }

    return solver;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Build a profugus EigenvalueSolver for generalized eigenvalue problem.
 *
 * This function creates a EigenvalueSolver object from a given database
 * and two Epetra operators.  The type of solver to be constructed is
 * determined by the database entry "eigensolver", which can be "Power",
 * "Arnoldi","Davidson","RQI"
 *
 * For standard eigenvalue solvers (Arnoldi, Power Iteration), the generalized
 * eigenproblem must be converted to a standard eigenvalue problem by
 * inverting A, i.e. the effective operator is \f$ A^{-1}B \f$.
 * We perform this operation using the InverseOperator class.  The nested
 * database "operator_db" controls the behavior of that operator construction.
 * See the documentation of InverseOperator and LinearSolverBuilder for
 * further details.  For Rayleigh Quotient Iteration, a shift-and-invert
 * operator must be constructed.  This class is also constructed from
 * entries in a nested operator_db.
 *
 * This function allows a preconditioner to be passed in.  For a Davidson
 * solver, this preconditioner is passed directly to the eigensolver.  For
 * all other options, the preconditioner is given to the relevant linear
 * solver.  Note that the preconditioner is optional.  No preconditioner
 * should be given if it is desired to use internal preconditioners
 * (i.e. native Aztec/Stratimikos preconditioning).
 */
template <class T>
Teuchos::RCP<EigenvalueSolver<T> >
EigenvalueSolverBuilder<T>::build_solver( RCP_ParameterList db,
                                              Teuchos::RCP<OP>  A,
                                              Teuchos::RCP<OP>  B,
                                              Teuchos::RCP<OP>  P )
{
    using std::string;
    RCP_EigenvalueSolver solver;

    // Determine type of solver to be constructed.
    std::string eigensolver = lower(
        db->get("eigensolver", string("Arnoldi")));

    if( eigensolver=="arnoldi" )
    {
        // Get nested operator_db
        RCP_ParameterList odb = Teuchos::sublist(db, "operator_db");

        // Build inverse operator for Arnoldi
        Teuchos::RCP<InverseOperator<T> > AinvB(
            new InverseOperator<T>(odb) );
        AinvB->set_operator(A);
        if( P != Teuchos::null )
        {
            AinvB->set_preconditioner(P);
        }
        AinvB->set_rhs_operator(B);

        // Build the solver
        solver = Teuchos::rcp(new Arnoldi<T>(db));
        solver->set_operator(AinvB);
    }
    else if( eigensolver=="power" )
    {
        // Get nested operator_db
        RCP_ParameterList odb = Teuchos::sublist(db, "operator_db");

        // Build inverse operator for Power Iteration
        Teuchos::RCP<InverseOperator<T> > AinvB(
            new InverseOperator<T>(odb) );
        AinvB->set_operator(A);
        if( P != Teuchos::null )
        {
            AinvB->set_preconditioner(P);
        }
        AinvB->set_rhs_operator(B);

        // Build the solver
        solver = Teuchos::rcp(new PowerIteration<T>(db));
        solver->set_operator(AinvB);
    }
    else if( eigensolver=="davidson" )
    {
        Teuchos::RCP<Davidson_Eigensolver<T> > davidson =
            Teuchos::rcp( new Davidson_Eigensolver<T>(db, A, B) );
        if( P != Teuchos::null )
        {
            davidson->set_preconditioner(P);
        }

        solver = davidson;
    }
    else if( eigensolver=="rqi" || eigensolver=="rayleigh" ||
             eigensolver=="rayleigh_quotient" )
    {
        // Get nested operator_db
        RCP_ParameterList odb = Teuchos::sublist(db, "operator_db");

        // Build ShiftedInverseOperator
        Teuchos::RCP<ShiftedInverseOperator<T> > shift_op(
            new ShiftedInverseOperator<T> (odb) );
        shift_op->set_operator(A);
        if( P != Teuchos::null )
        {
            shift_op->set_preconditioner(P);
        }
        shift_op->set_rhs_operator(B);

        // Build RQI solver and set operators
        Teuchos::RCP<RayleighQuotient<T> > rqi_solver =
            Teuchos::rcp( new RayleighQuotient<T>(db) );
        rqi_solver->set_operator(A);
        rqi_solver->set_rhs_operator(B);
        rqi_solver->set_shifted_operator(shift_op);

        // Set base class pointer
        solver = rqi_solver;
    }
    else
    {
        VALIDATE(false,
                 "Error: Invalid EigenvalueSolver option." << std::endl
                 << " Specify eigenvalue solver by setting eigensolver="
                 << "'arnoldi', 'power', 'davidson', 'rqi'"
                 << std::endl);
    }

    return solver;
}

} // end namespace profugus

#endif //solver_EigenvalueSolverBuilder_t_hh

//---------------------------------------------------------------------------//
//                 end of EigenvalueSolverBuilder.t.hh
//---------------------------------------------------------------------------//
