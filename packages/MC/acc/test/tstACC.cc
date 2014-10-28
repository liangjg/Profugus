//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   acc/test/tstACC.cc
 * \author Thomas M. Evans
 * \date   Tue Oct 28 10:47:44 2014
 * \brief  Test for ACC
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include <vector>

#include "gtest/utils_gtest.hh"

//---------------------------------------------------------------------------//
// TESTS
//---------------------------------------------------------------------------//

TEST(ACC, simple)
{
    double x[128];

#pragma acc kernels
    for (int i = 0; i < 128; ++i)
    {
        x[i] = 1.0;
    }    
}

//---------------------------------------------------------------------------//
//                 end of tstACC.cc
//---------------------------------------------------------------------------//