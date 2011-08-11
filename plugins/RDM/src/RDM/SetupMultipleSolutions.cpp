// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/Field.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/Geometry.hpp"

#include "Physics/PhysModel.hpp"

#include "RDM/RDSolver.hpp"

#include "SetupMultipleSolutions.hpp"


using namespace CF::Common;
using namespace CF::Common::Comm;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < SetupMultipleSolutions, CAction, LibRDM > SetupMultipleSolutions_Builder;

///////////////////////////////////////////////////////////////////////////////////////

SetupMultipleSolutions::SetupMultipleSolutions ( const std::string& name ) : CF::Solver::Action(name)
{
  // options

  m_options.add_option< OptionT<Uint> >( "nb_levels", 1u )
      ->description("Number of solution levels to be created")
      ->pretty_name("Number of levels");
}

void SetupMultipleSolutions::execute()
{
  RDM::RDSolver& mysolver = solver().as_type< RDM::RDSolver >();

  /* nb_levels == rkorder */

  const Uint nb_levels = option("nb_levels").value<Uint>();

  CMesh& mesh = *m_mesh.lock();

  CGroup& fields = mysolver.fields();

  // get the geometry field group

  Geometry& geometry = mesh.geometry();

  // construct vector of variables

  const Uint nbdofs = physical_model().neqs();

  std::string vars;
  for(Uint i = 0; i < nbdofs; ++i)
  {
   vars += "u" + to_str(i) + "[1]";
   if( i != nbdofs-1 ) vars += ",";
  }


  // configure 1st solution

  Field::Ptr solution = find_component_ptr_with_tag<Field>( geometry, RDM::Tags::solution() );
  if ( is_null( solution ) )
  {
    solution =
        geometry.create_field( RDM::Tags::solution(), vars )
        .as_ptr<Field>();

    solution->add_tag(Tags::solution());
  }

  if( ! fields.get_child_ptr( solution->name() ) )
    fields.create_component<CLink>( solution->name() ).link_to(solution).add_tag(RDM::Tags::solution());



  // create the other solutions based on the first solution field

  std::vector< Field::Ptr > rk_steps;

  rk_steps.push_back(solution);

  for(Uint step = 1; step < nb_levels; ++step)
  {
    Field::Ptr solution_k = find_component_ptr_with_tag<Field>( geometry, RDM::Tags::solution() + to_str(step));
    if ( is_null( solution_k ) )
    {
      std::string name = std::string(Tags::solution()) + to_str(step);
      solution_k = geometry.create_field( name, vars ).as_ptr<Field>();
      solution_k->add_tag("rksteps");
    }

    cf_assert( solution_k );
    rk_steps.push_back(solution_k);
  }

  for( Uint step = 1; step < rk_steps.size(); ++step)
  {
    if( ! fields.get_child_ptr( rk_steps[step]->name() ) )
      fields.create_component<CLink>( rk_steps[step]->name() ).link_to(solution).add_tag("rksteps");
  }

  /// @todo here we should check if space() order is correct,
  ///       if not the change space() by enriching or other appropriate action

  // configure residual

  Field::Ptr residual = find_component_ptr_with_tag<Field>( geometry, RDM::Tags::residual());
  if ( is_null( residual ) )
  {
    residual = geometry.create_field(Tags::residual(), "rhs" ).as_ptr<Field>();
    residual->add_tag(Tags::residual());
  }

  if( ! fields.get_child_ptr( RDM::Tags::residual() ) )
    fields.create_component<CLink>( RDM::Tags::residual() ).link_to(residual).add_tag(RDM::Tags::residual());

  // configure wave_speed

  Field::Ptr wave_speed = find_component_ptr_with_tag<Field>( geometry, RDM::Tags::wave_speed());
  if ( is_null( wave_speed ) )
  {
    wave_speed = geometry.create_field(Tags::wave_speed(), "ws").as_ptr<Field>();
    wave_speed->add_tag(Tags::wave_speed());
  }

  if( ! fields.get_child_ptr( RDM::Tags::wave_speed() ) )
    fields.create_component<CLink>( RDM::Tags::wave_speed() ).link_to(wave_speed).add_tag(RDM::Tags::wave_speed());

  /// @todo apply here the bubble insertion if needed

  // parallelize the solution if not yet done

  CommPattern& pattern = solution->parallelize();

  std::vector<URI> parallel_fields;
  parallel_fields.push_back( solution->uri() );

  for(Uint step = 1; step < nb_levels; ++step)
  {
    rk_steps[step]->parallelize_with( pattern );
    parallel_fields.push_back( rk_steps[step]->uri() );
  }

  mysolver.actions().get_child("Synchronize").configure_option("Fields", parallel_fields);

}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
