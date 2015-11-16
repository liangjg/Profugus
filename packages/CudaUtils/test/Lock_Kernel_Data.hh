//---------------------------------*-C++-*-----------------------------------//
/*!
 * \file   cuda_utils/test/Lock_Kernel_Data.hh
 * \author Seth R Johnson
 * \date   Thu Aug 15 08:21:19 2013
 * \brief  Lock_Kernel_Data declarations.
 * \note   Copyright (C) 2013 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef cuda_utils_test_Lock_Kernel_Data_hh
#define cuda_utils_test_Lock_Kernel_Data_hh

#include "../Vector_Traits.hh"
#include "../Device_Vector.hh"
#include "../Atomic_Lock.hh"
#include "../Launch_Args.hh"

namespace cuda
{
//---------------------------------------------------------------------------//
//! Lock kernel data wrapper
template<typename Arch_Switch>
struct Lock_Kernel_Data
{
    // >>> Typedefs
    typedef cuda::Vector_Traits<Arch_Switch, float> Traits_t;
    typedef typename Traits_t::Device_Vector_Float  Device_Vector_Float;
    typedef typename Traits_t::Device_Vector_Int    Device_Vector_Int;
    typedef Atomic_Lock<Arch_Switch>                Lock_t;
    typedef Launch_Args<Arch_Switch>                Launch_Args_t;

    // >>> Launch args

    Launch_Args_t       launch_args;
    Lock_t              lock;
    Device_Vector_Float output;

    // >>> Constructor

    Lock_Kernel_Data(unsigned int num_elements)
      : lock()
      , output(num_elements)
    {
        /* * */
    }
};

} // end namespace cuda

#endif // cuda_utils_test_Lock_Kernel_Data_hh

//---------------------------------------------------------------------------//
//              end of cuda_utils/Lock_Kernel_Data.hh
//---------------------------------------------------------------------------//