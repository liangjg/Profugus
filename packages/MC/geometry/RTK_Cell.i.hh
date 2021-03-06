//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   MC/geometry/RTK_Cell.i.hh
 * \author Thomas M. Evans
 * \date   Tue Dec 21 12:47:16 2010
 * \brief  Member definitions of class RTK_Cell.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef MC_geometry_RTK_Cell_i_hh
#define MC_geometry_RTK_Cell_i_hh

namespace profugus
{

//---------------------------------------------------------------------------//
// INLINE FUNCTIONS
//---------------------------------------------------------------------------//
/*!
 * \brief Find the segment for a point
 */
int RTK_Cell::segment(double x,
                      double y) const
{
    using def::X; using def::Y;

    REQUIRE(d_segments == 1 || d_segments == 4);

    // search segments
    if (d_segments == 4)
    {
        if (y < 0.0)
        {
            if (x < 0.0)
                return 3;
            else
                return 2;
        }
        else
        {
            if (x < 0.0)
                return 1;
        }
    }

    // we are in segment 0 if we get here
    return 0;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Find the cell for a point.
 *
 * \param region region id
 * \param segment segment id
 */
int RTK_Cell::cell(int region,
                   int segment) const
{
    REQUIRE(d_num_regions > 0);
    REQUIRE(region < d_num_regions);
    REQUIRE(segment < d_segments);
    REQUIRE(d_segments == 1 || d_segments == 4);

    ENSURE(region + d_num_regions * segment < d_num_cells);
    return region + d_num_regions * segment;
}

//---------------------------------------------------------------------------//
/*!
 * \brief Return the matid for a region.
 */
int RTK_Cell::matid(int region) const
{
    REQUIRE(d_num_regions > 0);
    REQUIRE(!d_vessel ? region >= 0 && region < d_num_regions :
             region == Geo_State_t::MODERATOR || region == Geo_State_t::VESSEL);

    // return if region is in the shells
    if (region < d_num_shells)
        return d_ids[region];

    // return the vessel-id if the region is vessel
    if (region == Geo_State_t::VESSEL)
    {
        CHECK(d_vessel);
        return d_vessel_id;
    }

    // we are in the moderator
    return d_mod_id;
}

//---------------------------------------------------------------------------//
// PRIVATE INLINE FUNCTIONS
//---------------------------------------------------------------------------//
/*
 * \brief Distance to external radial surfaces.
 */
void RTK_Cell::dist_to_radial_face(int          axis,
                                   double       p,
                                   double       dir,
                                   Geo_State_t &state)
{
    // check high/low faces
    if (dir > 0.0)
    {
        d_db   = (d_extent[axis][HI] - p) / dir;
        d_face = Geo_State_t::plus_face[axis];
    }
    else if (dir < 0.0)
    {
        d_db   = (d_extent[axis][LO] - p) / dir;
        d_face = Geo_State_t::minus_face[axis];
    }
    CHECK(d_db >= 0.0);

    // updated distance to boundary info
    if (d_db < state.dist_to_next_region)
    {
        state.dist_to_next_region = d_db;
        state.exiting_face        = d_face;
        state.next_face           = Geo_State_t::NONE;
    }
}

//---------------------------------------------------------------------------//
/*
 * \brief Distance to external radial surfaces.
 */
void RTK_Cell::dist_to_axial_face(double       p,
                                  double       dir,
                                  Geo_State_t &state)
{
    // check high/low faces
    if (dir > 0.0)
    {
        d_db   = (d_z - p) / dir;
        d_face = Geo_State_t::PLUS_Z;
    }
    else if (dir < 0.0)
    {
        d_db   = -p / dir;
        d_face = Geo_State_t::MINUS_Z;
    }
    CHECK(d_db >= 0.0);

    // updated distance to boundary info
    if (d_db < state.dist_to_next_region)
    {
        state.dist_to_next_region = d_db;
        state.exiting_face        = d_face;
        state.next_face           = Geo_State_t::NONE;
    }
}

//---------------------------------------------------------------------------//
/*
 * \brief Get the extents in the current reference frame
 */
void RTK_Cell::get_extents(Space_Vector &lower,
                           Space_Vector &upper) const
{
    using def::X; using def::Y; using def::Z;
    lower[X] = d_extent[X][LO]; upper[X] = d_extent[X][HI];
    lower[Y] = d_extent[Y][LO]; upper[Y] = d_extent[Y][HI];
    lower[Z] = 0.0            ; upper[Z] = d_z            ;
}

//---------------------------------------------------------------------------//

} // end namespace profugus

#endif // MC_geometry_RTK_Cell_i_hh

//---------------------------------------------------------------------------//
//              end of geometry/RTK_Cell.i.hh
//---------------------------------------------------------------------------//
