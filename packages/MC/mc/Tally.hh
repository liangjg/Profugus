//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   mc/Tally.hh
 * \author Thomas M. Evans
 * \date   Wed May 14 15:10:09 2014
 * \brief  Tally base class definition.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef mc_Tally_hh
#define mc_Tally_hh

#include <memory>

#include "Physics.hh"
#include "harness/DBC.hh"

namespace profugus
{

//===========================================================================//
/*!
 * \class Tally
 * \brief Base class for tallies.
 */
//===========================================================================//

class Tally
{
  public:
    //@{
    //! Typedefs.
    typedef Physics                    Physics_t;
    typedef std::shared_ptr<Physics_t> SP_Physics;
    //@}

  protected:
    // >>> DATA

    // Physics.
    SP_Physics b_physics;

  public:
    //! Constructor.
    Tally(SP_Physics physics)
        : b_physics(physics)
    {
        Ensure (b_physics);
    }

    // Destructor.
    virtual ~Tally() = 0;

    // >>> PUBLIC INTERFACE

    //! Track particle, using pre-calculated physics information (multipliers)
    virtual void accumulate(double step, const Particle &p) = 0;

    //! Accumulate first and second moments
    virtual void end_history() = 0;

    //! Do post-processing on first and second moments
    virtual void finalize(double num_histories) = 0;

    //! Begin active cycles in a kcode calculation (default no-op)
    virtual void begin_active_cycles() { /* * */ }

    //! Begin a new cycle in a kcode calculation (default no-op)
    virtual void begin_cycle() { /* * */ }

    //! End a cycle in a kcode calculation (default no-op)
    virtual void end_cycle(double num_particles) { /* * */ }

    //! Clear/re-initialize all tally values between solves
    virtual void reset() = 0;
};

} // end namespace profugus

#endif // mc_Tally_hh

//---------------------------------------------------------------------------//
//                 end of Tally.hh
//---------------------------------------------------------------------------//