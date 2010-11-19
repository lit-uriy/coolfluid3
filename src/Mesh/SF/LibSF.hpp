// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LibSF_hpp
#define CF_Mesh_LibSF_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/ExportAPI.hpp"
#include "Common/LibraryRegister.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro SF_API
/// @note build system defines COOLFLUID_MESH_SF_EXPORTS when compiling SF files
#ifdef COOLFLUID_MESH_SF_EXPORTS
#   define MESH_SF_API      CF_EXPORT_API
#   define MESH_SF_TEMPLATE
#else
#   define MESH_SF_API      CF_IMPORT_API
#   define MESH_SF_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace SF {

////////////////////////////////////////////////////////////////////////////////

  /// Shape functions module
  /// @author Tiago Quintino, Willem Deconinck, Bart Janssens
  class MESH_SF_API LibSF :
      public Common::LibraryRegister<LibSF>
  {
  public:

    /// Static function that returns the module name.
    /// Must be implemented for the LibraryRegister template
    /// @return name of the module
    static std::string library_name() { return "SF"; }

    /// Static function that returns the description of the module.
    /// Must be implemented for the LibraryRegister template
    /// @return descripton of the module
    static std::string library_description()
    {
      return "This library implements the shape functions.";
    }

    /// Gets the Class name
    static std::string type_name() { return "LibSF"; }

    /// initiate library
    virtual void initiate();

    /// terminate library
    virtual void terminate();
  }; // end LibSF

////////////////////////////////////////////////////////////////////////////////

} // SF
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_LibSF_hpp
