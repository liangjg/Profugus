//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   geometry/Mesh_State.cc
 * \author Thomas M. Evans and Seth R Johnson
 * \date   Monday July 21 18:23:40 2014
 * \brief  Mesh_State member definitions.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "Mesh_State.hh"

namespace profugus
{

//---------------------------------------------------------------------------//
// PUBLIC FUNCTIONS
//---------------------------------------------------------------------------//
/*!
 * \brief Pack the state.
 */
void Mesh_State::pack(char *buffer) const
{
    Not_Implemented("packing Mesh_State.");
}

//---------------------------------------------------------------------------//
/*!
 * \brief Unpack the geometric state.
 */
void Mesh_State::unpack(const char *buffer)
{
    Not_Implemented("unpacking Mesh_State.");
}

} // end namespace profugus

//---------------------------------------------------------------------------//
//                 end of Mesh_State.cc
//---------------------------------------------------------------------------//