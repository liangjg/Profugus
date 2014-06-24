//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   driver/Manager.cc
 * \author Thomas M. Evans
 * \date   Wed Jun 18 11:21:16 2014
 * \brief  Manager member definitions.
 * \note   Copyright (C) 2014 Oak Ridge National Laboratory, UT-Battelle, LLC.
 */
//---------------------------------------------------------------------------//

#include "Teuchos_XMLParameterListHelpers.hpp"

#include "harness/DBC.hh"
#include "comm/Timing.hh"
#include "comm/global.hh"
#include "mc/Fission_Source.hh"
#include "mc/Uniform_Source.hh"
#include "Manager.hh"

namespace mc
{

//---------------------------------------------------------------------------//
// CONSTRUCTOR
//---------------------------------------------------------------------------//
/*!
 * \brief Constructor.
 */
Manager::Manager()
    : d_node(profugus::node())
    , d_nodes(profugus::nodes())
{
}

//---------------------------------------------------------------------------//
/*!
 * \brief Setup the problem.
 * \param xml_file
 */
void Manager::setup(const std::string &xml_file)
{
    SCOPED_TIMER("Manager.setup");

    SCREEN_MSG("Reading xml file -> " << xml_file);
    SCREEN_MSG("Building and initializing geometry, physics, "
               << "variance reduction, and tallies");

    // use the problem builder to setup the problem
    Problem_Builder builder;
    builder.setup(xml_file);

    // get the problem database from the problem-builder
    d_db = builder.problem_db();
    Check (!d_db.is_null());

    // store the problem name
    d_problem_name = d_db->get("problem_name", std::string("MC"));
    d_db->setName(d_problem_name + "-PROBLEM");

    // get the geometry and physics
    d_geometry = builder.get_geometry();
    d_physics  = builder.get_physics();
    Check (d_geometry);
    Check (d_physics);

    // get the variance reduction
    auto var_reduction = builder.get_var_reduction();
    Check (var_reduction);

    // get the external source shape (it could be null)
    auto shape = builder.get_source_shape();

    // problem type
    std::string prob_type = shape ? "fixed" : "eigenvalue";

    // set the problem type in the final db
    d_db->set("problem_type", prob_type);

    // build the random controller
    d_rnd_control = std::make_shared<RNG_Control_t>(
        d_db->get<int>("seed", 32442));

    SCREEN_MSG("Building " << prob_type << " solver");

    // make the tallier
    SP_Tallier tallier(std::make_shared<Tallier_t>());
    tallier->set(d_geometry, d_physics);

    // make the transporter
    SP_Transporter transporter(std::make_shared<Transporter_t>(
                                   d_db, d_geometry, d_physics));
    transporter->set(tallier);
    transporter->set(var_reduction);

    // build the appropriate solver (default is eigenvalue)
    if (prob_type == "eigenvalue")
    {
        // make the fission source
        std::shared_ptr<profugus::Fission_Source> source(
            std::make_shared<profugus::Fission_Source>(
                d_db, d_geometry, d_physics, d_rnd_control));

        // make the solver
        d_kcode_solver = std::make_shared<KCode_Solver_t>(d_db);

        // set it
        d_kcode_solver->set(transporter, source);

        // assign the base solver
        d_solver = d_kcode_solver;

    }
    else if (prob_type == "fixed")
    {
        // make the uniform source
        std::shared_ptr<profugus::Uniform_Source> source(
            std::make_shared<profugus::Uniform_Source>(
                d_db, d_geometry, d_physics, d_rnd_control));
        source->build_source(shape);

        // make the solver
        d_fixed_solver = std::make_shared<Fixed_Source_Solver_t>();

        // set it
        d_fixed_solver->set(transporter, source);

        // assign the base solver
        d_solver = d_fixed_solver;
    }
    else
    {
        throw profugus::assertion(
            "Undefined problem type; choose eigenvalue or fixed");
    }

    Ensure (d_geometry);
    Ensure (d_physics);
    Ensure (d_solver);
}

//---------------------------------------------------------------------------//
/*!
 * \brief Solve the problem.
 */
void Manager::solve()
{
    if (d_db->get<bool>("do_transport", true))
    {
        SCOPED_TIMER("Manager.solve");

        SCREEN_MSG("Executing solver");

        // run the appropriate solver
        if (d_kcode_solver)
        {
            Check (!d_fixed_solver);
            d_kcode_solver->solve();
        }
        else if (d_fixed_solver)
        {
            Check (!d_kcode_solver);
            d_fixed_solver->solve();
        }
        else
        {
            throw profugus::assertion("No legitimate solver built");
        }
    }
}

//---------------------------------------------------------------------------//
/*!
 * \brief Do output.
 */
void Manager::output()
{
    SCOPED_TIMER("Manager.output");

    SCREEN_MSG("Outputting data");

    // >>> OUTPUT FINAL DATABASE

    // output the final database
    if (d_node == 0)
    {
        std::ostringstream m;
        m << d_problem_name << "_db.xml";
        Teuchos::writeParameterListToXmlFile(*d_db, m.str());
    }

    profugus::global_barrier();
}

} // end namespace mc

//---------------------------------------------------------------------------//
//                 end of Manager.cc
//---------------------------------------------------------------------------//