// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/mpl/for_each.hpp>

#include "Common/CBuilder.hpp"

#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"

#include "Solver/CPhysicalModel.hpp"

#include "RDM/CSysLDA.hpp"

#include "RDM/SupportedTypes.hpp"    // supported elements

#include "RDM/Euler2D.hpp"       // supported physics

#include "RDM/SchemeCSysLDA.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CSysLDA, RDM::DomainTerm, LibRDM > CSysLDA_Builder;

////////////////////////////////////////////////////////////////////////////////

/// Looper defines a functor taking the type that boost::mpl::for_each passes.
/// It is the core of the looping mechanism.
template < typename PHYS>
struct CSysLDA::ElementLoop
{
  /// region to loop on
  Mesh::CRegion& region;
  /// component containing the element loop
  CSysLDA& comp;
  /// Constructor
  ElementLoop( CSysLDA& comp_in, Mesh::CRegion& region_in ) : comp(comp_in), region(region_in) {}
  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& T )
  {
    /// definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    /// parametrization of the numerical scheme
     typedef SchemeCSysLDA< SF, QD, PHYS > SchemeT;

    boost_foreach(Mesh::CElements& elements,
                  Common::find_components_recursively_with_filter<Mesh::CElements>(region,IsElementType<SF>()))
    {
      // get the scheme or create it if does not exist
      Component::Ptr cscheme = comp.get_child_ptr( SchemeT::type_name() );
      typename SchemeT::Ptr scheme;
      if( is_null( cscheme ) )
        scheme = comp.create_component< SchemeT >( SchemeT::type_name() );
      else
        scheme = cscheme->as_ptr_checked<SchemeT>();

      // loop on elements of that type
      scheme->set_elements(elements);

      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        scheme->select_loop_idx(elem);
        scheme->execute();
      }
    }
  }

};

//////////////////////////////////////////////////////////////////////////////

CSysLDA::CSysLDA ( const std::string& name ) : RDM::DomainTerm(name)
{
  regist_typeinfo(this);
}

CSysLDA::~CSysLDA() {}

void CSysLDA::execute()
{
  /// @todo physical model should be a configuration option of the solver
  CPhysicalModel::Ptr pm = find_component_ptr_recursively<CPhysicalModel>( *Core::instance().root() );
  if( is_null(pm) )
    throw ValueNotFound(FromHere(), "could not found any physical model to use");

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {
    std::string physics = pm->type();

    if ( physics == "Euler2D" )
    {
      CSysLDA::ElementLoop<Euler2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }

  }
}

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF