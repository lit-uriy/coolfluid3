// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_TSshInformation_h
#define CF_GUI_Client_Core_TSshInformation_h

////////////////////////////////////////////////////////////////////////////

#include "GUI/Client/Core/LibClientCore.hpp"

class QString;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

////////////////////////////////////////////////////////////////////////////////

  /// @brief Reprensents information needed to connect to the server and
  /// launch it (if needed) through an SSH connection.

  /// @author Quentin Gasper.

  struct ClientCore_API TSshInformation
  {
    public :

    /// @brief Remote machine hostname.
    QString m_hostname;

    /// @brief Socket port number.
    quint16 m_port;

    /// @brief Constructor.

    /// Provided for convinience.

    /// @param hostname Remote machine hostname.
    /// @param port Socket port number.
    /// @param launchServer If @c true, the user requests to
    /// launch a new server m_instance.
    /// @param username Username to use to authenticate to the remote
    /// machine.
    TSshInformation(const QString & hostname = QString("hostname"),
                    quint16 port = 62784)
    {
      this->m_hostname = hostname;
      this->m_port = port;
    }
  }; // struct TSshInformation

////////////////////////////////////////////////////////////////////////////////

} // ClientCore
} // GUI
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Client_Core_TSshInformation_h
