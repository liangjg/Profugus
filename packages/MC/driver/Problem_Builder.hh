//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   driver/Problem_Builder.hh
 * \author Thomas M. Evans
 * \date   Wed Mar 12 22:25:22 2014
 * \brief  Problem_Builder class definition.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#ifndef driver_Problem_Builder_hh
#define driver_Problem_Builder_hh

#include <string>
#include <vector>
#include <memory>

#include "Teuchos_RCP.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_DefaultComm.hpp"
#include "Teuchos_Array.hpp"
#include "Teuchos_TwoDArray.hpp"

#include "mc/Physics.hh"

namespace mc
{

//===========================================================================//
/*!
 * \class Problem_Builder
 * \brief Read and initialize an mc problem,
 */
//===========================================================================//

class Problem_Builder
{
  public:
    //@{
    //! Typedefs.
    typedef Teuchos::ParameterList      ParameterList;
    typedef Teuchos::RCP<ParameterList> RCP_ParameterList;
    typedef profugus::Physics           Physics_t;
    typedef Physics_t::Geometry_t       Geometry_t;
    typedef std::shared_ptr<Physics_t>  SP_Physics;
    typedef std::shared_ptr<Geometry_t> SP_Geometry;
    //@}

  private:
    // >>> DATA

    // Problem-parameterlist (talks to solver components).
    RCP_ParameterList d_db;

    // Physics and geometry.
    SP_Physics  d_physics;
    SP_Geometry d_geometry;

  public:
    // Constructor.
    Problem_Builder();

    // Setup the problem.
    void setup(const std::string &xml_file);

    // >>> ACCESSORS

    //! Get problem database.
    RCP_ParameterList problem_db() const { return d_db; }

    //! Get the geometry.
    SP_Geometry get_geometry() const { return d_geometry; }

    //! Get the physics.
    SP_Physics get_physics() const { return d_physics; }

  private:
    // >>> IMPLEMENTATION

    // Typedefs.
    typedef Teuchos::Comm<int>          Comm;
    typedef Teuchos::RCP<const Comm>    RCP_Comm;
    typedef Teuchos::Array<int>         OneDArray_int;
    typedef Teuchos::Array<double>      OneDArray_dbl;
    typedef Teuchos::Array<std::string> OneDArray_str;
    typedef Teuchos::TwoDArray<int>     TwoDArray_int;
    typedef Teuchos::TwoDArray<double>  TwoDArray_dbl;

    // Build implementation.
    void build_geometry();
    void build_physics();
    void build_source(const ParameterList &source_db);

    // Number of assemblies and pins per assembly.
    int d_Na[2];
    int d_Np[2];

    // Teuchos communicator.
    RCP_Comm d_comm;

    // Problem-setup parameterlists.
    RCP_ParameterList d_coredb;
    RCP_ParameterList d_assblydb;
    RCP_ParameterList d_matdb;
};

} // end namespace mc

#endif // driver_Problem_Builder_hh

//---------------------------------------------------------------------------//
//                 end of Problem_Builder.hh
//---------------------------------------------------------------------------//
