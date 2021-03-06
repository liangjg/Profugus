//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   MC/mc/Variance_Reduction.hh
 * \author Thomas M. Evans
 * \date   Thu May 08 11:08:46 2014
 * \brief  Variance_Reduction base class definition.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef MC_mc_Variance_Reduction_hh
#define MC_mc_Variance_Reduction_hh

#include <memory>

#include "harness/DBC.hh"
#include "utils/Definitions.hh"
#include "geometry/RTK_Geometry.hh"
#include "Physics.hh"

namespace profugus
{

//===========================================================================//
/*!
 * \class Variance_Reduction
 * \brief Base class for applying variance reduction methods
 */
//===========================================================================//

template <class Geometry>
class Variance_Reduction
{
  public:
    //@{
    //! Useful typedefs.
    typedef Geometry                         Geometry_t;
    typedef std::shared_ptr<Geometry_t>      SP_Geometry;
    typedef typename Geometry_t::Geo_State_t Geo_State_t;
    typedef Physics<Geometry>                Physics_t;
    typedef std::shared_ptr<Physics_t>       SP_Physics;
    typedef typename Physics_t::Particle_t   Particle_t;
    typedef typename Physics_t::SP_Particle  SP_Particle;
    typedef typename Physics_t::Bank_t       Bank_t;
    //@}

  public:
    // Constructor.
    explicit Variance_Reduction() { /* * */ }

    // Virtual destructor.
    virtual ~Variance_Reduction() { /*...*/ }

    //! Set the geometry class
    void set(SP_Geometry geometry) { REQUIRE(geometry); b_geometry = geometry; }

    //! Set the physics class
    void set(SP_Physics physics) { REQUIRE(physics); b_physics = physics; }

    //! Return the splitting boolean
    bool uses_splitting() { return b_splitting; }

    // >>> VARIANCE REDUCTION INTERFACE

    //! Apply variance reduction method after a surface crossing
    virtual void post_surface(Particle_t& particle, Bank_t& bank) const = 0;

    //! Apply variance reduction method after a collision
    virtual void post_collision(Particle_t& particle, Bank_t& bank) const = 0;

  protected:
    // Problem geometry implementation.
    SP_Geometry b_geometry;

    // Problem physics implementation.
    SP_Physics b_physics;

    // Boolean is true if VR method uses splitting
    bool b_splitting;
};

} // end namespace profugus

#endif // MC_mc_Variance_Reduction_hh

//---------------------------------------------------------------------------//
//                 end of Variance_Reduction.hh
//---------------------------------------------------------------------------//
