/*
 * ef-mac-sr-stats.cc
 *
 *  Created on: Jan 21, 2022
 *      Author: ugjeci
 */

#include "sl-mac-sr-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlMacSRStats::SlMacSRStats ()
{
}

void
SlMacSRStats::SetDb (SQLiteOutput *db, const std::string & tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName +
                        "(BWDID INTEGER NOT NULL,"
                        "RNTI INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	"Run INTEGER NOT NULL,"
		  	  	  	  	"TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlMacSRStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlMacSRStats::SaveSlMacSRStats (uint8_t bwdId, uint16_t rnti)
{
	MacSRCache c;
  c.timeInstance = Simulator::Now();
  c.bwdId = bwdId;
  c.rnti = rnti;

  m_macSRCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_macSRCache.size () * sizeof (MacSRCache) > 10) // 1000000
    {
      WriteCache ();
    }
}

void
SlMacSRStats::EmptyCache()
{
  WriteCache ();
}

void
SlMacSRStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
                              uint32_t run, const std::string &table)
{
  bool ret;
  sqlite3_stmt *stmt;
  ret = p->SpinPrepare (&stmt, "DELETE FROM \"" + table + "\" WHERE SEED = ? AND RUN = ?;");
  NS_ABORT_IF (ret == false);
  ret = p->Bind (stmt, 1, seed);
  NS_ABORT_IF (ret == false);
  ret = p->Bind (stmt, 2, run);

  ret = p->SpinExec (stmt);
  NS_ABORT_IF (ret == false);
}

void SlMacSRStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_macSRCache)
	{
	  sqlite3_stmt *stmt;
	  ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?);");
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 1, static_cast<uint8_t> (v.bwdId));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 2, static_cast<uint16_t> (v.rnti));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 3, RngSeedManager::GetSeed ());
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 4, static_cast<uint32_t> (RngSeedManager::GetRun ()));
	  // NS_ASSERT (ret);
    // ret = m_db->Bind (stmt, 4, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
    NS_ASSERT (ret);
	  // insert the timestamp
	  ret = m_db->Bind (stmt, 5, v.timeInstance);
	  NS_ASSERT (ret);

	  ret = m_db->SpinExec (stmt);
	  NS_ASSERT (ret);
	}
  m_macSRCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3


