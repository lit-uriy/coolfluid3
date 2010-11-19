// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_TaggedObject_hpp
#define CF_Common_TaggedObject_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"
#include "Common/CommonAPI.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

  //////////////////////////////////////////////////////////////////////////

  /// Manages tags
  class Common_API TaggedObject
  {
  public:

    /// Constructor
    TaggedObject();

    /// Check if this component has a given tag assigned
    bool has_tag(const std::string& tag) const;

    /// add tag to this component
    void add_tag(const std::string& tag);

    /// @return tags in a vector
    std::vector<std::string> get_tags();

  private:

    std::string m_tags;

  }; // class TaggedObject

  //////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_TaggedObject_hpp
