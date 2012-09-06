// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_BCStrong_hpp
#define cf3_sdm_BCStrong_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/PropertyList.hpp"

#include "math/MatrixTypes.hpp"

#include "sdm/Tags.hpp"
#include "sdm/BC.hpp"
#include "sdm/ShapeFunction.hpp"
#include "sdm/Operations.hpp"
#include "sdm/PhysDataBase.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

/// @author Willem Deconinck
template <typename PHYSDATA>
class sdm_API BCStrong : public BC
{
public: // typedefs
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  enum { NDIM = PHYSDATA::NDIM }; ///< number of dimensions
  enum { NEQS = PHYSDATA::NEQS }; ///< number of independent variables or equations
  typedef PHYSDATA PhysData;
  typedef Eigen::Matrix<Real,NDIM,1> RealVectorNDIM;
  typedef Eigen::Matrix<Real,NEQS,1> RealVectorNEQS;

public: // functions

  /// constructor
  BCStrong( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "BCStrong"; }

  virtual void execute();

private: // functions

protected: // configuration

  virtual void initialize()
  {
    BC::initialize();

    CFdebug << "Initializing BC " << uri() << CFendl;
    CFdebug << "    with caches " << shared_caches().uri() << CFendl;
    CFdebug << "    with solution_field " << solution_field().uri() << CFendl;
    face_elem = shared_caches().template get_cache<SFDElement>("face_elem");
    face_elem->options().set("space",solution_field().dict().template handle<mesh::Dictionary>());
    inner_cell = shared_caches().template get_cache<SFDElement>("inner_cell");
    inner_cell->options().set("space",solution_field().dict().template handle<mesh::Dictionary>());
    face_pt_solution              = shared_caches().template get_cache< SolutionPointField<NEQS,NDIM> >(sdm::Tags::solution());
    face_pt_solution->options().set("field",solution_field().uri());
  }

  virtual void set_face_entities(const mesh::Entities& face_entities)
  {
    BC::set_face_entities(face_entities);
    face_elem->cache(m_face_entities);
    face_pt_solution->cache(m_face_entities);

    inner_cell_face_data.resize(face_elem->get().sf->nb_nodes());
    for(Uint face_pt=0; face_pt<inner_cell_face_data.size(); ++face_pt)
      inner_cell_face_data[face_pt] = boost::shared_ptr<PhysData>(new PhysData);

    boundary_face_data.resize(face_elem->get().sf->nb_nodes());
    for(Uint face_pt=0; face_pt<boundary_face_data.size(); ++face_pt)
      boundary_face_data[face_pt] = boost::shared_ptr<PhysData>(new PhysData);

  }

  virtual void set_face_element(const Uint face_elem_idx)
  {
    BC::set_face_element(face_elem_idx);
    face_elem->set_cache(m_face_elem_idx);
    face_pt_solution->set_cache(m_face_elem_idx);
  }

  virtual void unset_face_element()
  {
    BC::unset_face_element();
    face_elem->get().unlock();
    face_pt_solution->get().unlock();
  }

  virtual void set_inner_cell()
  {
    cf3_assert(inner_cell);
    inner_cell->cache(cell_entities,cell_idx);
  }

  virtual void unset_inner_cell()
  {
    cf3_assert(inner_cell);
    inner_cell->get().unlock();
  }

  virtual void compute_solution(const PhysData& inner_cell_data, const RealVectorNDIM& boundary_face_normal, RealVectorNEQS& boundary_face_solution) = 0;

  void set_connectivity()
  {
    CFdebug << "setting connectivity" << CFendl;
    cf3_assert(inner_cell);
    cf3_assert(inner_cell->get().sf);
    Uint nb_face_pts = inner_cell->get().sf->face_flx_pts(cell_face_nb).size();

    CFdebug << "nb_face_pts = " << nb_face_pts << CFendl;
    inner_cell_face_pt_idx = inner_cell->get().sf->face_flx_pts(cell_face_nb);
    boundary_face_pt_idx.resize(nb_face_pts);

    mesh::Field::View cell_coords   = solution_field().dict().coordinates().view(inner_cell->get().space->connectivity()[inner_cell->get().idx]);
    std::vector<RealVector> cell_face_coords(nb_face_pts,RealVector(NDIM));
    for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
    {
      inner_cell->get().reconstruct_from_solution_space_to_flux_points[inner_cell_face_pt_idx[face_pt]](cell_coords,cell_face_coords[face_pt]);
    }
    if (inner_cell->get().sf->order() == 0)
    {
      boundary_face_pt_idx[0] = 0;
    }
    else
    {
      RealMatrix bdry_face_coords = face_elem->get().space->get_coordinates(face_elem->get().idx);
      for (Uint bdry_face_pt=0; bdry_face_pt<nb_face_pts; ++bdry_face_pt)
      {
        const RealVector& bdry_face_pt_coord = bdry_face_coords.row(bdry_face_pt);
        bool matched = false;
        for (Uint inner_cell_face_pt=0; inner_cell_face_pt<nb_face_pts; ++inner_cell_face_pt)
        {
          bool m=true;
          for (Uint d=0; d<NDIM; ++d)
            m = m && ( std::abs(cell_face_coords[inner_cell_face_pt][d] - bdry_face_pt_coord[d]) < 200*math::Consts::eps() );
          if ( m )
          {
            matched=true;
            boundary_face_pt_idx[inner_cell_face_pt] = bdry_face_pt;
            break;
          }
        }
        if (!matched)
        {
          CFinfo << "cell_face_pts:"<<CFendl;
          for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
            CFinfo << cell_face_coords[face_pt].transpose() << CFendl;
          CFinfo << "bdry_face_pts:"<<CFendl;
          for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
            CFinfo << bdry_face_coords.row(face_pt) << CFendl;
        }
        cf3_assert_desc(inner_cell->get().space->uri().string()+"["+common::to_str(inner_cell->get().idx)+"]",matched);
      }
    }
  }


  void compute_face()
  {
    set_connectivity();

    Uint nb_face_pts = inner_cell->get().sf->face_flx_pts(cell_face_nb).size();

    // inner cell data
    for (Uint face_pt=0; face_pt<nb_face_pts; ++face_pt)
      compute_flx_pt_phys_data(inner_cell->get(),inner_cell_face_pt_idx[face_pt],*inner_cell_face_data[face_pt]);
  }

  virtual void compute_flx_pt_phys_data(const SFDElement& elem, const Uint flx_pt, PHYSDATA& point_data )
  {
    mesh::Field::View sol_pt_solution = solution_field().view(elem.space->connectivity()[elem.idx]);
    mesh::Field::View sol_pt_coords = solution_field().dict().coordinates().view(elem.space->connectivity()[elem.idx]);
    elem.reconstruct_from_solution_space_to_flux_points[flx_pt](sol_pt_solution,point_data.solution);
    elem.reconstruct_from_solution_space_to_flux_points[flx_pt](sol_pt_coords,point_data.coord);
//    std::cout << "reconstruct \n" << elem.entities->get_coordinates(elem.idx) << "   to flx_pt " << flx_pt << "\n" << point_data.coord.transpose() << std::endl;
  }


protected: // fast-access-data (for convenience no "m_" prefix)

  // Data
std::vector<boost::shared_ptr< PhysData > > inner_cell_face_data;
std::vector<boost::shared_ptr< PhysData > > boundary_face_data;
std::vector<Uint> inner_cell_face_pt_idx;
std::vector<Uint> boundary_face_pt_idx;


  Handle<mesh::Entities const> cell_entities;
  Uint cell_idx;
  Uint cell_face_nb;
  Uint cell_flx_pt;

  // In face solution points ( = cell flux points )
  Handle< CacheT<SolutionPointField<NEQS,NDIM> > > face_pt_solution;

  Handle< CacheT<SFDElement> > inner_cell;
  Handle< CacheT<SFDElement> > face_elem;

  RealVectorNDIM unit_normal;
  RealVectorNEQS face_sol;

  std::vector<RealVectorNEQS> flx_pt_solution;

}; // end BCStrong

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
inline BCStrong<PHYSDATA>::BCStrong( const std::string& name )
  : BC(name)
{
  properties()["brief"] = std::string("Spectral Finite Difference BC");
  properties()["description"] = std::string("");
}

/////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
inline void BCStrong<PHYSDATA>::execute()
{

  find_inner_cell(m_face_entities,m_face_elem_idx,cell_entities,cell_idx,cell_face_nb);
  set_inner_cell();

  compute_face();


//  CFdebug << "computing for face " << m_face_entities->uri() << "["<<m_face_elem_idx<<"]" << CFendl;

  // 1) compute solution in ALL flux points
  flx_pt_solution.resize(inner_cell->get().sf->nb_flx_pts());
  mesh::Field::View sol_pt_solution = solution_field().view(inner_cell->get().space->connectivity()[inner_cell->get().idx]);
  inner_cell->get().reconstruct_from_solution_space_to_flux_points(sol_pt_solution,flx_pt_solution);

  // 2) compute solution in face flux points, and modify it there
  for(Uint face_pt=0; face_pt<boundary_face_pt_idx.size(); ++face_pt)
  {
    cell_flx_pt = inner_cell_face_pt_idx[face_pt];
    cf3_assert(face_pt<inner_cell_face_data.size());
    cf3_assert(cell_flx_pt<flx_pt_solution.size());
    cf3_assert(inner_cell_face_data[face_pt]);
    compute_solution(*inner_cell_face_data[face_pt],unit_normal,flx_pt_solution[cell_flx_pt]);
    for (Uint v=0; v<NEQS; ++v)
    {
//      CFdebug << " - " << face_pt_solution->get()[boundary_face_pt_idx[face_pt]][v] << CFendl;
      face_pt_solution->get()[boundary_face_pt_idx[face_pt]][v] = flx_pt_solution[cell_flx_pt][v];
//      CFdebug << " + " << face_pt_solution->get()[boundary_face_pt_idx[face_pt]][v] << CFendl;
//      std::cout << solution_field()[field_index[boundary_face_pt_idx[face_pt]]][v] << " ";
//      std::cout << face_sol[v] << " ";
    }
  }

//  CFdebug << "face solution computed" << CFendl;

  // 3) recompute the solution in solution points using flux point values
  inner_cell->get().reconstruct_from_flux_points_to_solution_space(inner_cell->get().sf->flx_pt_dirs(inner_cell_face_pt_idx[0])[0],flx_pt_solution,sol_pt_solution);

//  compute_face();
//  for(Uint face_pt=0; face_pt<boundary_face_pt_idx.size(); ++face_pt)
//  {
//    cell_flx_pt = inner_cell_face_pt_idx[face_pt];
//    compute_solution(*inner_cell_face_data[face_pt],unit_normal,face_sol);
////    common::TableConstRow<Uint>::type field_index = face_elem->get().space->connectivity()[m_face_elem_idx];

////    std::cout << "boundary -- " << face_elem->get().entities->uri() << "[" << face_elem->get().idx << "]" << " : face_points = " << field_index[boundary_face_pt_idx[face_pt]] << "  ---> " ;
//    for (Uint v=0; v<NEQS; ++v)
//    {
//      face_pt_solution->get()[boundary_face_pt_idx[face_pt]][v] = face_sol[v];
//      std::cout << solution_field()[field_index[boundary_face_pt_idx[face_pt]]][v] << " ";
////      std::cout << face_sol[v] << " ";
//    }
////    std::cout << "     inner flx_pt idx = " << inner_cell_face_pt_idx[face_pt];
////    std::cout << std::endl;

//  }
  unset_inner_cell();
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_BCStrong_hpp
