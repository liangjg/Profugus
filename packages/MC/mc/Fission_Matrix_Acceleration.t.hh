//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   mc/Fission_Matrix_Acceleration.t.hh
 * \author Thomas M. Evans
 * \date   Thu Nov 13 19:53:21 2014
 * \brief  Fission_Matrix_Acceleration template member definitions.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef mc_Fission_Matrix_Acceleration_t_hh
#define mc_Fission_Matrix_Acceleration_t_hh

#include "harness/DBC.hh"
#include "harness/Soft_Equivalence.hh"
#include "spn/Dimensions.hh"
#include "spn/SpnSolverBuilder.hh"
#include "spn/Linear_System_FV.hh"
#include "spn/Moment_Coefficients.hh"
#include "spn/Eigenvalue_Solver.hh"
#include "Fission_Matrix_Acceleration.hh"

namespace profugus
{

//---------------------------------------------------------------------------//
// CONSTRUCTOR
//---------------------------------------------------------------------------//
/*!
 * \brief Constructor.
 */
template<class T>
Fission_Matrix_Acceleration_Impl<T>::Fission_Matrix_Acceleration_Impl()
{
}

//---------------------------------------------------------------------------//
// PUBLIC INTERFACE
//---------------------------------------------------------------------------//
/*!
 * \brief Build the SPN problem.
 *
 * \param builder
 */
template<class T>
void Fission_Matrix_Acceleration_Impl<T>::build_problem(
    const Problem_Builder_t &builder)
{
    // get the problem database from the problem-builder
    b_db = builder.problem_db();

    // get the mesh objects from the builder
    b_mesh    = builder.mesh();
    b_indexer = builder.indexer();
    b_gdata   = builder.global_data();

    // get the material database from the problem builder
    b_mat = builder.mat_db();

    // build the problem dimensions
    d_dim = Teuchos::rcp(new profugus::Dimensions(b_db->get("SPn_order", 1)));

    // make the linear system for this problem
    d_system = Teuchos::rcp(
        new Linear_System_FV<T>(b_db, d_dim, b_mat, b_mesh, b_indexer, b_gdata));

    // make the forward and adjoint eigenvectors
    d_forward = VTraits::build_vector(d_system->get_Map());
    d_adjoint = VTraits::build_vector(d_system->get_Map());

    // make the matrices (A,B) for the SPN problem, Ap = (1/k)Bp
    d_system->build_Matrix();
    d_system->build_fission_matrix();

    // build the weight corrections in the FM mesh
    d_nu.resize(b_mesh->num_cells());

    ENSURE(!b_mesh.is_null());
    ENSURE(!b_indexer.is_null());
    ENSURE(!b_gdata.is_null());
    ENSURE(!b_mat.is_null());
    ENSURE(!d_dim.is_null());
    ENSURE(!d_adjoint.is_null());
    ENSURE(!d_forward.is_null());
    ENSURE(!d_system.is_null());
}

//---------------------------------------------------------------------------//
/*!
 * \brief Initialize the acceleration at the beginning of the K-code solve.
 *
 * This call invokes one eigenvalue solve:
 * \f[
   \mathbf{A}^{\dagger}\phi^{\dagger} =
   \frac{1}{k}\mathbf{B}^{\dagger}\phi^{\dagger}\:,
 * \f]
 */
template<class T>
void Fission_Matrix_Acceleration_Impl<T>::initialize(RCP_ParameterList mc_db)
{
    typedef Eigenvalue_Solver<T>               Solver_t;
    typedef typename Solver_t::External_Source Source_t;

    REQUIRE(mc_db->isSublist("fission_matrix_db"));

    // get the fission matrix db
    RCP_ParameterList fmdb = Teuchos::sublist(mc_db, "fission_matrix_db");

    // set the damping (defaults to 1.0)
    d_beta = fmdb->get("damping", 1.0);

    // make a "null" external source to pass to the solver
    Teuchos::RCP<const Source_t> null_source;
    CHECK(null_source.is_null());

    // make an eigenvalue solver, use the database from the SPN problem
    Teuchos::RCP<Solver_t> eigensolver =
        Teuchos::rcp_dynamic_cast<Solver_t>(
            SpnSolverBuilder::build("eigenvalue", b_db));

    // >>> ADJOINT SOLVE

    // setup the solver for the adjoint solve
    eigensolver->setup(b_mat, b_mesh, b_indexer, b_gdata, d_system, true);

    // solve the adjoint problem
    eigensolver->solve(null_source);

    // store the eigenvalue
    d_keff = eigensolver->get_eigenvalue();
    CHECK(d_keff > 0.0);

    // get the eigenvector and assign it
    std::vector<int> index(1, 0);
    ATraits::SetBlock(*eigensolver->get_eigenvector(), index, *d_adjoint);

    // >>> FORWARD SOLVE

    // setup the solver for the forward solve (puts the operators back to
    // forward)
    eigensolver->setup(b_mat, b_mesh, b_indexer, b_gdata, d_system, false);

    // solve the adjoint problem
    eigensolver->solve(null_source);
    CHECK(profugus::soft_equiv(eigensolver->get_eigenvalue(), d_keff, 1.0e-3));

    // assign the forward eigenvector
    ATraits::SetBlock(*eigensolver->get_eigenvector(), index, *d_forward);

    // >>> BUILD THE FISSION_MATRIX_SOLVER

    // build and set the fission matrix solver
    d_fm_solver = Teuchos::rcp(
        new Fission_Matrix_Solver<T>(fmdb, b_mesh, b_mat, d_system, d_keff));
    d_fm_solver->set_eigenvectors(d_forward, d_adjoint);
}

//---------------------------------------------------------------------------//
/*!
 * \brief Start cycle initialization with fission container at \f$l\f$.
 *
 * Calculate beginning of cycle fission density for use in acceleration at end
 * of cycle.
 */
template<class T>
void Fission_Matrix_Acceleration_Impl<T>::start_cycle(
    double                        k_l,
    const Fission_Site_Container &f)
{
    REQUIRE(!d_fm_solver.is_null());
    d_fm_solver->set_u_begin(f, k_l);
}

//---------------------------------------------------------------------------//
/*!
 * \brief End cycle acceleration to create fission container at \f$l+1\f$.
 *
 * Build new fission source based on SPN acceleration.
 */
template<class T>
void Fission_Matrix_Acceleration_Impl<T>::end_cycle(
    Fission_Site_Container &f)
{
    REQUIRE(!d_fm_solver.is_null());

    // solve the acceleration equation
    d_fm_solver->solve(f);
    CHECK(VTraits::local_length(d_fm_solver->get_g()) ==
          VTraits::local_length(d_forward));

    // convert the correction vector from SPN space to fission sites
    convert_g(d_fm_solver->get_g());
}

//---------------------------------------------------------------------------//
// PRIVATE FUNCTIONS
//---------------------------------------------------------------------------//
/*!
 * \brief Convert g to fission density.
 */
template<class T>
void Fission_Matrix_Acceleration_Impl<T>::convert_g(RCP_Const_Vector gc)
{
    REQUIRE(b_mesh->num_cells() * d_system->get_dims()->num_equations()
            * b_mat->xs().num_groups() <=
            VectorTraits<T>::local_length(gc));
    REQUIRE(d_nu.size() == b_mesh->num_cells());

    // SPN moments
    double u_m[4] = {0.0, 0.0, 0.0, 0.0};

    // get cross sections from the database
    const auto &xs = b_mat->xs();

    // number of equations (moments) in the SPN solution
    int N = d_system->get_dims()->num_equations();

    // number of groups
    int Ng = xs.num_groups();

    // number of local cells
    int Nc = b_mesh->num_cells();

    // the state field is ordered group->cell whereas the u vector is
    // ordered cell->moments->group
    Teuchos::ArrayView<const double> gcv = VectorTraits<T>::get_data(gc);

    // loop over cells on this domain
    for (int cell = 0; cell < Nc; ++cell)
    {
        // get nu-sigma_f for this cell
        const auto &nu_sigf = xs.vector(b_mat->matid(cell), XS::NU_SIG_F);
        CHECK(nu_sigf.length() == Ng);

        // volume of this cell
        double V = b_mesh->volume(cell);

        // loop over groups
        for (int g = 0; g < Ng; ++g)
        {
            // loop over moments
            for (int n = 0; n < N; ++n)
            {
                CHECK(d_system->index(g, n, cell) < gcv.size());

                // get the moment from the solution vector
                u_m[n] = gcv[d_system->index(g, n, cell)];
            }

            // do f^T * phi_0
            d_nu[cell] +=  Moment_Coefficients::u_to_phi(
                u_m[0], u_m[1], u_m[2], u_m[3]) * nu_sigf[g] * V;
        }
    }
}

} // end namespace profugus

#endif // mc_Fission_Matrix_Acceleration_t_hh

//---------------------------------------------------------------------------//
//                 end of Fission_Matrix_Acceleration.t.hh
//---------------------------------------------------------------------------//
