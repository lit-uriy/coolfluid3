#include "Common/BasicExceptions.hpp"
#include "Common/ObjectProvider.hpp"
#include "Common/CommonLib.hpp"

#include "Common/CLink.hpp"

namespace CF {
namespace Common {

////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < CLink, Component, CommonLib, NB_ARGS_1 >
CLink_Provider ( CLink::getClassName() );

////////////////////////////////////////////////////////////////////////////////

CLink::CLink ( const CName& name) : Component ( name )
{
  BUILD_COMPONENT;
  m_is_link = true;
}

////////////////////////////////////////////////////////////////////////////////

CLink::~CLink()
{
}

////////////////////////////////////////////////////////////////////////////////

Component::Ptr CLink::get ()
{
  return m_link_component.lock();
}

Component::ConstPtr CLink::get () const
{
  return m_link_component.lock();
}

////////////////////////////////////////////////////////////////////////////////

void CLink::link_to ( Component::Ptr lnkto )
{
  if (lnkto->is_link())
    throw SetupError(FromHere(), "Cannot link a CLink to another CLink");

  m_link_component = lnkto;
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
