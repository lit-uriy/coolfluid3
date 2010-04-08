#include "Common/CF.hh"
#include "Common/PE.hh"
#include "Common/MPI/PE_MPI.hh"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

//////////////////////////////////////////////////////////////////////////////

PE& PE::getInstance ()
{
  static PE pe_instance;
  return pe_instance;
}

//////////////////////////////////////////////////////////////////////////////

PEInterface& PE::interface ()
{
  PEInterface * pinterface = PE::getInstance().m_pe_interface;

  cf_assert ( pinterface != CFNULL );

  return * pinterface;
}

//////////////////////////////////////////////////////////////////////////////

PE::PE () :
    m_pe_interface (CFNULL),
    m_is_init (false),
    m_current_status (WorkerStatus::NOT_RUNNING)
{
}

//////////////////////////////////////////////////////////////////////////////

bool PE::is_init ()
{
  return m_is_init;
}

//////////////////////////////////////////////////////////////////////////////

Uint PE::get_rank () const
{
  if ( m_is_init ) return interface().get_rank();
  else return 0;
}

//////////////////////////////////////////////////////////////////////////////

void PE::init (int * argc, char *** args)
{
  cf_assert ( !m_is_init );
  cf_assert ( m_pe_interface == CFNULL);

  m_pe_interface = new MPI::PE_MPI(argc, args);

  cf_assert (m_pe_interface != CFNULL);

  m_is_init = true;
}

//////////////////////////////////////////////////////////////////////////////

void PE::finalize ()
{
  cf_assert ( m_is_init );
  cf_assert ( m_pe_interface != CFNULL );

  // must be first to ensure all destructors dependent on PE see MPI is down
  m_is_init = false;
  delete_ptr(m_pe_interface);
}

//////////////////////////////////////////////////////////////////////////////

void PE::change_status (WorkerStatus::Type status)
{
  cf_assert ( WorkerStatus::Convert::is_valid(status) );
  m_current_status = status;
}

//////////////////////////////////////////////////////////////////////////////

WorkerStatus::Type PE::status()
{
  return m_current_status;
}

//////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

