/*
 * ef-uplink-tb-size.cc
 *
 *  Created on: Nov 24, 2021
 *      Author: ugjeci
 */

#include "sl-downlink-tb-size-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlDownlinkTbSizeStats::SlDownlinkTbSizeStats ()
{
}

void
SlDownlinkTbSizeStats::SetDb (SQLiteOutput *db, const std::string & tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName +
                        "(IMSI INTEGER NOT NULL,"
                        "TB INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	"Run INTEGER NOT NULL,"
		  	  	  	  	"TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlDownlinkTbSizeStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlDownlinkTbSizeStats::SaveDlTbSizeStats (uint64_t imsi, uint64_t tb)
{
	DlTbSizeCache c;
  c.timeInstance = Simulator::Now();
  c.imsi = imsi;
  c.tb = tb;

  m_dlTbSizeCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_dlTbSizeCache.size () * sizeof (DlTbSizeCache) > 100) // 0000
    {
      WriteCache ();
    }
}

void
SlDownlinkTbSizeStats::EmptyCache()
{
  WriteCache ();
}

void
SlDownlinkTbSizeStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlDownlinkTbSizeStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_dlTbSizeCache)
	{
	  sqlite3_stmt *stmt;
	  ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?);");
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 1, static_cast<uint32_t> (v.imsi));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 2, static_cast<uint32_t> (v.tb));
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
  m_dlTbSizeCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3


