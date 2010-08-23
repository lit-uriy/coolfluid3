#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Triag2D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

Triag2D::Triag2D() 
{
  m_shape = shape;
  m_dimension = dimension;
  m_dimensionality = dimensionality;
  m_nb_faces = nb_faces;
  m_nb_edges = nb_edges;
}

////////////////////////////////////////////////////////////////////////////////

// Define the members so functions taking a reference to these work.
// See http://stackoverflow.com/questions/272900/c-undefined-reference-to-static-class-member
const GeoShape::Type Triag2D::shape;
const Uint Triag2D::nb_faces;
const Uint Triag2D::nb_edges;
const Uint Triag2D::dimensionality;
const Uint Triag2D::dimension;

} // Mesh
} // CF
