//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   mc/Particle.hh
 * \author Thomas M. Evans
 * \date   Fri Apr 25 11:26:16 2014
 * \brief  Particle class definition.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef mc_Particle_hh
#define mc_Particle_hh

#include "utils/Definitions.hh"
#include "utils/Metaclass.hh"
#include "rng/RNG.hh"
#include "geometry/RTK_State.hh"
#include "Definitions.hh"

namespace profugus
{

//===========================================================================//
/*!
 * \class Particle
 * \brief Particle class for MC transport.
 */
/*!
 * \example mc/test/tstParticle.cc
 *
 * Test of Particle.
 */
//===========================================================================//

class Particle
{
  public:
    //@{
    //! Typedefs.
    typedef def::Space_Vector   Space_Vector;
    typedef RNG                 RNG_t;
    typedef RTK_State           Geo_State_t;
    typedef Metaclass<Particle> Metadata;
    //@}

  private:
    // >>> DATA

    // Material id in current region.
    int d_matid;

    // Particle group index.
    int d_group;

    // Particle weight.
    double d_wt;

    // Random number generator (reference counted).
    RNG d_rng;

    // Alive/dead status.
    bool d_alive;

    // Particle geometric state.
    Geo_State_t d_geo_state;

    // Particle distance to travel in mean free paths
    double d_dist_mfp;

    // Problem/application specific metadata.
    Metadata d_metadata;

  public:
    // Constructor
    Particle();

    // >>> PARTICLE FUNCTIONS

    //! Set a new weight.
    void set_wt(double wt) { d_wt = wt; }

    //! Set particle group.
    void set_group(int g) { d_group = g; }

    //! Multiply weight.
    void multiply_wt(double wt) { d_wt *= wt; }

    //! Set a new random number generator.
    void set_rng(const RNG &rng) { d_rng = rng; }

    //! Set the material id of the region occupied by the particle.
    void set_matid(int matid) { d_matid = matid; }

    //! Set a new distance to travel in mean free paths
    void set_dist_mfp( const double dist_mfp ) { d_dist_mfp = dist_mfp; }

    //! Add a value to the distance to travel in mean free paths.
    void add_dist_mfp( const double dist ) { d_dist_mfp += dist; }

    //! Kill the particle.
    void kill() { d_alive = false; }

    //! Set particle status to alive.
    void live() { d_alive = true; }

    //@{
    //! Get a handle to the geometric state of the particle.
    Geo_State_t& geo_state() { return d_geo_state; }
    const Geo_State_t& geo_state() const { return d_geo_state; }
    //@}

    //@{
    //! Access particle data.
    bool alive() const { return d_alive; }
    double wt() const { return d_wt; }
    RNG& rng() { return d_rng; }
    int matid() const { return d_matid; }
    int group() const { return d_group; }
    double dist_mfp() const { return d_dist_mfp; }
    //@}

    //@{
    //! Get particle metadata.
    const Metadata& metadata() const { return d_metadata; }
    Metadata& metadata() { return d_metadata; }
    //@}
};

} // end namespace profugus

#endif // mc_Particle_hh

//---------------------------------------------------------------------------//
//                 end of Particle.hh
//---------------------------------------------------------------------------//
