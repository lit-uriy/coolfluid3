#include <boost/foreach.hpp>

#include "Common/ObjectProvider.hpp"
#include "Quad2D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
////////////////////////////////////////////////////////////////////////////////

Quad2D::Quad2D()
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
const GeoShape::Type Quad2D::shape;
const Uint Quad2D::nb_faces;
const Uint Quad2D::nb_edges;
const Uint Quad2D::dimensionality;
const Uint Quad2D::dimension;

} // Mesh
} // CF
