//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   Alea/mc_solvers/AdjointMcEventKernel.hh
 * \author Steven Hamilton
 * \brief  Perform adjoint MC histories to solve linear system.
 */
//---------------------------------------------------------------------------//

#ifndef Alea_mc_solvers_AdjointMcEventKernel_hh
#define Alea_mc_solvers_AdjointMcEventKernel_hh

#include "MC_Data.hh"

#include "Kokkos_View.hpp"
#include "Kokkos_Core.hpp"
#include "Kokkos_Random.hpp"

#include "AleaTypedefs.hh"

namespace alea
{

//---------------------------------------------------------------------------//
/*!
 * \class AdjointMcEventKernel
 * \brief Perform Monte Carlo random walks on linear system.
 *
 * This class performs random walks using the adjoint Monte Carlo algorithm.
 * The interface of this function conforms to the Kokkos "parallel_for"
 * functor API to enable automated shared memory parallelism.
 */
//---------------------------------------------------------------------------//

class AdjointMcEventKernel
{
  public:

    // Required typedefs for Kokkos functor API

    // Execution policy and team member types
    typedef Kokkos::RangePolicy<DEVICE> range_policy;
    typedef range_policy::member_type   policy_member;
    typedef Kokkos::TeamPolicy<DEVICE>  team_policy;

    typedef Kokkos::Random_XorShift64_Pool<DEVICE>  generator_pool;
    typedef typename generator_pool::generator_type generator_type;

    AdjointMcEventKernel(const MC_Data_View                  &mc_data,
                         const const_scalar_view              coeffs,
                         Teuchos::RCP<Teuchos::ParameterList> pl,
                         generator_pool                       pool);

    //! Solve problem (this is a host function)
    void solve(const MV &x, MV &y);

  private:

    enum TRANSITION {STANDARD, BINNED, SHARED_MEM};

    // Build the initial CDF and weights (host function)
    void build_initial_distribution(const MV &x);

    void solve_impl( const scalar_view &y_device ) const;

    template <class device>
    void sort_by_state( const History_Data &data ) const;

    // Vector length
    int d_N;

    // Data for Monte Carlo
    const MC_Data_View       d_mc_data;
    const random_scalar_view d_coeffs;
    const scalar_view        d_start_cdf;
    const scalar_view        d_start_wt;

    // Kokkos random generator pool
    generator_pool d_rand_pool;

    // Problem parameters
    TRANSITION d_transition_type;
    int    d_max_history_length;
    bool   d_use_expected_value;
    bool   d_print;
    int    d_num_histories;
    int    d_num_batches;
    int    d_histories_batch;
    SCALAR d_start_wt_factor;
    Teuchos::RCP<Teuchos::ParameterList> d_pl;
};

} // namespace alea

#include "AdjointMcEventKernel.i.hh"

#endif // Alea_mc_solvers_AdjointMcEventKernel_hh

