/*
 * ef-mac-rlc-buffer-status.cc
 *
 *  Created on: Sep 7, 2022
 *      Author: ugjeci
 */



#include "sl-ue-bsr-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlMacUeBsrStats::SlMacUeBsrStats ()
{
}

void
SlMacUeBsrStats::SetDb (SQLiteOutput *db, const std::string & tableName, uint32_t writeSize)
{
  m_db = db;
  m_tableName = tableName;
  m_writeSize = writeSize;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName +
                        "(Frame INTEGER NOT NULL, "
                        "SubFrame INTEGER NOT NULL,"
                        "Slot INTEGER NOT NULL,"
                        "cellId INTEGER NOT NULL,"
                        "rnti INTEGER NOT NULL,"
                        "lcid INTEGER NOT NULL,"
                        "bsrid INTEGER NOT NULL,"
                        "buffersize INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	"Run INTEGER NOT NULL,"
		  	  	  	  	"TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");

  NS_ASSERT (ret);

  SlMacUeBsrStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlMacUeBsrStats::SaveSlMacUeBsrStats (uint16_t frame, uint8_t subframe, uint16_t slot, uint16_t cellId, uint16_t rnti, 
  uint8_t lcid, uint8_t bsrid, uint32_t bufferSize)
{
	SlMacUeBsrCache c;
  c.frame = frame;
  c.subframe = subframe;
  c.slot = slot;
  c.timeInstance = Simulator::Now();
  c.cellId = cellId;
  c.rnti = rnti;
  c.lcid = lcid;
  c.bsrid = bsrid;
  c.buffSize = bufferSize;

  m_macUeBsrCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_macUeBsrCache.size () * sizeof (SlMacUeBsrCache) > m_writeSize) // 1000000
    {
      WriteCache ();
    }
}

void
SlMacUeBsrStats::EmptyCache()
{
  WriteCache ();
}

void
SlMacUeBsrStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlMacUeBsrStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_macUeBsrCache)
	{
	  sqlite3_stmt *stmt;
	  ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?);");
    NS_ASSERT (ret);
    ret = m_db->Bind (stmt, 1, static_cast<uint32_t> (v.frame ));
    NS_ASSERT (ret);
    ret = m_db->Bind (stmt, 2, static_cast<uint32_t> (v.subframe));
    NS_ASSERT (ret);
    ret = m_db->Bind (stmt, 3, static_cast<uint32_t> (v.slot));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 4, static_cast<uint32_t> (v.cellId));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 5, static_cast<uint32_t> (v.rnti));
	  NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 6, static_cast<uint32_t> (v.lcid));
	  NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 7, static_cast<uint32_t> (v.bsrid));
	  NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, static_cast<uint32_t> (v.buffSize));
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 9, RngSeedManager::GetSeed ());
	  NS_ASSERT (ret);
	  ret = m_db->Bind (stmt, 10, static_cast<uint32_t> (RngSeedManager::GetRun ()));
	  // NS_ASSERT (ret);
    // ret = m_db->Bind (stmt, 10, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
      NS_ASSERT (ret);
	  // insert the timestamp
	  ret = m_db->Bind (stmt, 11, v.timeInstance);
	  NS_ASSERT (ret);

	  ret = m_db->SpinExec (stmt);
	  NS_ASSERT (ret);
	}
  m_macUeBsrCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3

