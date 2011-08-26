// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include <boost/utility.hpp>

#include "Math/LibMath.hpp"
#include "Common/MPI/PE.hpp"
#include "Common/Component.hpp"
#include "Common/OptionT.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Math/LSS/System.hpp"
#include "Math/LSS/Matrix.hpp"
#include "Math/LSS/Vector.hpp"
#include "Math/LSS/BlockAccumulator.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file System.cpp implementation of LSS::System
  @author Tamas Banyai
**/

////////////////////////////////////////////////////////////////////////////////////////////

#include "Math/LSS/EmptyLSS/EmptyLSSVector.hpp"
#include "Math/LSS/EmptyLSS/EmptyLSSMatrix.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CF_HAVE_TRILINOS
  #include "Math/LSS/Trilinos/TrilinosMatrix.hpp"
  #include "Math/LSS/Trilinos/TrilinosVector.hpp"
#endif // CF_HAVE_TRILINOS

////////////////////////////////////////////////////////////////////////////////////////////

using namespace CF;
using namespace CF::Math;

////////////////////////////////////////////////////////////////////////////////////////////

LSS::System::System(const std::string& name) :
  Component(name)
{
  options().add_option< CF::Common::OptionT<std::string> >( "solver" , "Trilinos" );
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::create(CF::Common::Comm::CommPattern& cp, Uint neq, std::vector<Uint>& node_connectivity, std::vector<Uint>& starting_indices)
{
  if (is_created()) destroy();
  std::string solvertype=options().option("solver").value_str();

  if (solvertype=="EmptyLSS"){
      m_mat=new LSS::EmptyLSSMatrix("Matrix");
      m_rhs=new LSS::EmptyLSSVector("RHS");
      m_sol=new LSS::EmptyLSSVector("Solution");
  }

  if (solvertype=="Trilinos"){
    #ifdef CF_HAVE_TRILINOS
      m_mat(new TrilinosMatrix("Matrix"));
      m_rhs(new TrilinosVector("RHS"));
      m_sol(new TrilinosVector("Solution"));
    #else
      throw Common::SetupError(FromHere(),"Trilinos is selected for linear solver, but COOLFluiD was not compiled with it.");
    #endif
  }

  m_mat.create(cp,neq,node_connectivity,starting_indices,m_sol,m_rhs);
  m_rhs.create(cp.gid().size(),neq);
  m_sol.create(cp.gid().size(),neq);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::swap(LSS::Matrix::Ptr matrix, LSS::Vector::Ptr solution, LSS::Vector::Ptr rhs)
{
  if ((matrix->is_created()!=solution->is_created())||(matrix->is_created()!=rhs->is_created()))
    throw Common::SetupError(FromHere(),"Inconsistent states.");
  if ((matrix->solvertype()!=solution->solvertype())||(matrix->solvertype()!=rhs->solvertype()))
    throw Common::NotSupported(FromHere(),"Inconsistent linear solver types.");
  if ((matrix->neq()!=solution->neq())||(matrix->neq()!=rhs->neq()))
    throw Common::BadValue(FromHere(),"Inconsistent number of equations.");
  if ((matrix->blockcol_size()!=solution->size())||(matrix->blockcol_size()!=rhs->size()))
    throw Common::BadValue(FromHere(),"Inconsistent number of block rows.");
  if (m_mat!=matrix) delete m_mat;
  if (m_rhs!=matrix) delete m_rhs;
  if (m_sol!=matrix) delete m_sol;
  m_mat=matrix;
  m_rhs=rhs;
  m_sol=solution;
  options().option("solver").put_value(matrix->solvertype());
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::destroy()
{
  delete m_mat;
  delete m_sol;
  delete m_rhs;
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::solve()
{
  cf_assert(is_created());
  m_mat->solve(m_sol,m_rhs);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::set_values(const LSS::BlockAccumulator& values)
{
  cf_assert(is_created());
  m_mat->set_values(values);
  m_sol->set_sol_values(values);
  m_rhs->set_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::add_values(const LSS::BlockAccumulator& values)
{
  cf_assert(is_created());
  m_mat->add_values(values);
  m_sol->add_sol_values(values);
  m_rhs->add_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::get_values(LSS::BlockAccumulator& values)
{
  cf_assert(is_created());
  m_mat->get_values(values);
  m_sol->get_sol_values(values);
  m_rhs->get_rhs_values(values);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::dirichlet(const Uint iblockrow, const Uint ieq, Real value, bool preserve_symmetry=false)
{
  cf_assert(is_created());
  std::vector<Real> v;
  if (preserve_symmetry)
  {
    m_mat->get_column_and_replace_to_zero(iblockrow,ieq,v);
    if (preserve_symmetry)
      for (int i=0; i<(const int)v.size(); i++)
        m_rhs->add_value(i,-v[i]*value);
  }
  m_mat->set_row(iblockrow,ieqn,1.,0.);
  m_sol->set_value(iblockrow,ieqn,value);
  m_rhs->set_value(iblockrow,ieqn,value);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::periodicity (const Uint iblockrow_to, const Uint iblockrow_from)
{
  cf_assert(is_created());
  m_mat->tie_bockrow_pairs(iblockrow_to,iblockrow_from);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::set_diagonal(const Vector& diag)
{
  cf_assert(is_created());
  m_mat->set_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::add_diagonal(const LSS::Vector& diag)
{
  cf_assert(is_created());
  m_mat->add_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::get_diagonal(LSS::Vector& diag)
{
  cf_assert(is_created());
  m_mat->get_diagonal(diag);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::reset(Real reset_to=0.)
{
  cf_assert(is_created());
  m_mat->reset(reset_to);
  m_sol->reset(reset_to);
  m_rhs->reset(reset_to);
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::print(std::iostream& stream)
{
  if (is_created())
  {
    m_mat->print(stream);
    m_sol->print(stream);
    m_rhs->print(stream);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

inline void LSS::System::print(const std::string& filename)
{
  if (is_created())
  {
    std::ofstream ofs(filename.c_str());
    m_mat->print(ofs);
    m_sol->print(ofs);
    m_rhs->print(ofs);
    ofs.close();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

inline bool LSS::System::is_created()
{
  int numcreated=0;
  if (m_mat!=nullptr) if (m_mat->is_created()) numcreated+=1;
  if (m_sol!=nullptr) if (m_sol->is_created()) numcreated+=2;
  if (m_rhs!=nullptr) if (m_rhs->is_created()) numcreated+=4;
  switch (numcreated) {
    case 0 : return false;
    case 7 : return true;
    default: throw Common::SetupError(FromHere(),"LSS System is in inconsistent state.");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////
