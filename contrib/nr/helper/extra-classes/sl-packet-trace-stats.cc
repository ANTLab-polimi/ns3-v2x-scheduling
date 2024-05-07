/*
 * ef-packet-trace.cc
 *
 *  Created on: Apr 5, 2022
 *      Author: ugjeci
 */

#include "sl-packet-trace-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlPacketTraceStats::SlPacketTraceStats ()
{
}

void
SlPacketTraceStats::SetDb (SQLiteOutput *db, const std::string & tableName, uint32_t writeSize)
{
  m_db = db;
  m_tableName = tableName;
  m_writeSize = writeSize;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
		  	  	  	  	    "Direction VARCHAR(10) NOT NULL,"
                        "Frame INTEGER NOT NULL,"
                        "Subframe INTEGER NOT NULL,"
                        "Slot INTEGER NOT NULL,"
						            "Symstart INTEGER NOT NULL,"
						            "Numsym INTEGER NOT NULL,"
						            "Cellid INTEGER NOT NULL,"
		  	  	  	  	    "Rnti INTEGER NOT NULL,"
						            "Tbsize INTEGER NOT NULL,"
                        "Mcs INTEGER NOT NULL,"
                        "Rv INTEGER NOT NULL,"
                        "Sinr DOUBLE NOT NULL,"
                        "Corrupt INTEGER NOT NULL,"
                        "Bler DOUBLE NOT NULL,"
                        "Bwpid INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	    "Run INTEGER NOT NULL,"
		  		  	  	      "TimeStamp DOUBLE NOT NULL"
		  	  	  	  	");"
		  );
//                        "Run INTEGER NOT NULL);");
//  "rbAssignedNum INTEGER NOT NULL,"

  NS_ASSERT (ret);

  SlPacketTraceStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                 RngSeedManager::GetRun(), 
                                //  static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                 tableName);
}

void
SlPacketTraceStats::SavePacketTrace(std::string direction, RxPacketTraceParams params)
{

  SlPacketTraceCache c;
  c.timeInstance = Simulator::Now();
  c.packetTrace = params;
  c.direction = direction;
//  c.direction = 1;

  m_packetTraceCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_packetTraceCache.size () * sizeof (SlPacketTraceCache) > m_writeSize)
    {
      WriteCache ();
    }
}

void
SlPacketTraceStats::EmptyCache()
{
  WriteCache ();
}

void
SlPacketTraceStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlPacketTraceStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_packetTraceCache)
    {
      sqlite3_stmt *stmt;
      ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 1, v.direction);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 2, v.packetTrace.m_frameNum);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 3, v.packetTrace.m_subframeNum);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 4, v.packetTrace.m_slotNum);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 5, v.packetTrace.m_symStart);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 6, v.packetTrace.m_numSym);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 7, static_cast<uint16_t> (v.packetTrace.m_cellId));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, v.packetTrace.m_rnti);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 9, v.packetTrace.m_tbSize);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 10, v.packetTrace.m_mcs);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 11, v.packetTrace.m_rv);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 12, v.packetTrace.m_sinr);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 13, static_cast<uint32_t> (v.packetTrace.m_corrupt));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 14, v.packetTrace.m_tbler);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 15, v.packetTrace.m_bwpId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 16, RngSeedManager::GetSeed ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 17, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      // NS_ASSERT (ret);
      // ret = m_db->Bind (stmt, 17, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
      NS_ASSERT (ret);
      // insert the timestamp
      ret = m_db->Bind (stmt, 18, v.timeInstance);
      NS_ASSERT (ret);

      ret = m_db->SpinExec (stmt);
      NS_ASSERT (ret);
    }
  m_packetTraceCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3




