//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   cuda_mc/Fixed_Solver_Tester.hh
 * \author Steven Hamilton
 * \date   Wed Jan 20 16:13:24 2016
 * \brief  Fixed_Solver_Tester class definition.
 * \note   Copyright (C) 2016 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef cuda_mc_Fixed_Solver_Tester_hh
#define cuda_mc_Fixed_Solver_Tester_hh

#include <vector>
#include <memory>
#include "Matprop/xs/XS.hh"

namespace cuda_mc
{

//===========================================================================//
/*!
 * \class Fixed_Solver_Tester
 * \brief Wrapper to kernel launches to test Fixed_Solver class.
 */
//===========================================================================//

class Fixed_Solver_Tester
{
  public:

      typedef std::vector<int>              Vec_Int;
      typedef std::vector<double>           Vec_Dbl;
      typedef Teuchos::RCP<profugus::XS>    RCP_XS;

      static void test_transport( const Vec_Dbl  &x_edges,
                                  const Vec_Dbl  &y_edges,
                                  const Vec_Dbl  &z_edges,
                                  const Vec_Int  &matids,
                                        RCP_XS    xs,
                                        int       num_particles,
                                        Vec_Dbl  &tally);
};

} // end namespace cuda_mc

#endif // cuda_mc_Fixed_Solver_Tester_hh

//---------------------------------------------------------------------------//
//                 end of Fixed_Solver_Tester.hh
//---------------------------------------------------------------------------//