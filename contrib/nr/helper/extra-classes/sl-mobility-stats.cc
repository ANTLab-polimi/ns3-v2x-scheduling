/*
 * ef-mac-sr-stats.cc
 *
 *  Created on: Jan 21, 2022
 *      Author: ugjeci
 */

#include "sl-mobility-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlMobilityStats::SlMobilityStats ()
{
}

void
SlMobilityStats::SetDb (SQLiteOutput *db, const std::string & tableName, uint32_t writeSize)
{
  m_db = db;
  m_tableName = tableName;
  m_writeSize = writeSize;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName +
                        "(PosX INTEGER NOT NULL,"
                        "PosY INTEGER NOT NULL,"
                        "VelX INTEGER NOT NULL,"
                        "VelY INTEGER NOT NULL,"
                        "id INTEGER NOT NULL,"
                        "deviceType VARCHAR(30) NOT NULL,"
                        "Seed INTEGER NOT NULL,"
                        "Run INTEGER NOT NULL,"
                        "TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlMobilityStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlMobilityStats::SaveSlMobilityStats (Vector pos, Vector vel, uint64_t id, std::string deviceType)
{
	MobilityCache c;
  c.timeInstance = Simulator::Now();
  c.pos = pos;
  c.vel = vel;
  c.id = id;
  c.deviceType = deviceType;

  m_mobilityCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_mobilityCache.size () * sizeof (MobilityCache) > m_writeSize) // 1000000
    {
      WriteCache ();
    }
}

void
SlMobilityStats::EmptyCache()
{
  WriteCache ();
}

void
SlMobilityStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlMobilityStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_mobilityCache)
	{
	  sqlite3_stmt *stmt;
	  ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?);");
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 1, static_cast<int> (v.pos.x));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 2, static_cast<int> (v.pos.y));
    NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 3, static_cast<int> (v.vel.x));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 4, static_cast<int> (v.vel.y));
    NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 5, static_cast<uint32_t> (v.id));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 6, v.deviceType);
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 7, RngSeedManager::GetSeed ());
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 8, static_cast<uint32_t> (RngSeedManager::GetRun ()));
	  // NS_ASSERT (ret);
    // ret = m_db->Bind (stmt, 4, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
    NS_ASSERT (ret);
	  // insert the timestamp
	  ret = m_db->Bind (stmt, 9, v.timeInstance);
	  NS_ASSERT (ret);

	  ret = m_db->SpinExec (stmt);
	  NS_ASSERT (ret);
	}
  m_mobilityCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3


