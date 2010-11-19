// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_ConcreteProvider_hpp
#define CF_Common_ConcreteProvider_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CF.hpp"

#include "Common/Provider.hpp"
#include "Common/Factory.hpp"


namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

/// number of arguments in the constructor
enum NB_ARGS { NB_ARGS_0 = 0, NB_ARGS_1 = 1, NB_ARGS_2 = 2 };

/// @brief Concrete provider class for all types which the constructor takes zero arguments
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE, int = NB_ARGS_0 >
class ConcreteProvider : public Common::Provider<BASE> {
public:

  /// Constructor
  explicit ConcreteProvider(const std::string& name) :  Common::Provider<BASE>(name) {}

  /// Virtual destructor
  virtual ~ConcreteProvider() {}

  /// Polymorphic function to create objects of dynamical type BASE
  /// @return boost::shared_ptr olding the created object
  virtual boost::shared_ptr<BASE> create() = 0;

}; // ConcreteProvider

////////////////////////////////////////////////////////////////////////////////

/// @brief Concrete provider class for all types which the constructor takes one argument
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE>
class ConcreteProvider<BASE,1> : public Common::Provider<BASE> {
public:

  typedef BASE BASE_TYPE;
  typedef typename BASE::ARG1 BASE_ARG1;

  /// Constructor
  explicit ConcreteProvider(const std::string& name) :  Common::Provider<BASE>(name)  {}

  /// Virtual destructor
  virtual ~ConcreteProvider() {}

  /// Polymorphic function to create objects of dynamical type BASE
  /// @param arg1 first parameter
  /// @return boost::shared_ptr olding the created object
  virtual boost::shared_ptr<BASE> create(BASE_ARG1 arg1) = 0;

}; // ConcreteProvider

////////////////////////////////////////////////////////////////////////////////

/// @brief Concrete provider class for all types which the constructor takes two argument
/// @author Andrea Lani
/// @author Tiago Quintino
template <class BASE>
class ConcreteProvider<BASE,2> : public Common::Provider<BASE> {
public:

  typedef BASE BASE_TYPE;
  typedef typename BASE::ARG1 BASE_ARG1;
  typedef typename BASE::ARG2 BASE_ARG2;

  /// Constructor
  explicit ConcreteProvider(const std::string& name) : Common::Provider<BASE>(name) {}

  /// Virtual destructor
  virtual ~ConcreteProvider() {}

  /// Polymorphic function to create objects of dynamical type BASE
  /// @param arg1 first parameter
  /// @param arg2 first parameter
  /// @return boost::shared_ptr olding the created object
  virtual boost::shared_ptr<BASE> create(BASE_ARG1 arg1, BASE_ARG2 arg2) = 0;

}; // ConcreteProvider

////////////////////////////////////////////////////////////////////////////////

  } // Common

} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_ConcreteProvider_hpp
