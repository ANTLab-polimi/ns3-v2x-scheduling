/*
 * ef-slot-ctrl-output-stats.cc
 *
 *  Created on: Mar 23, 2022
 *      Author: ugjeci
 */

#include "sl-ctrl-msgs-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlCtrlMsgsStats::SlCtrlMsgsStats ()
{
}

void
SlCtrlMsgsStats::SetDb (SQLiteOutput *db, const std::string & tableName, uint32_t writeSize)
{
  m_db = db;
  m_tableName = tableName;
  m_writeSize = writeSize;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName +
                        "(frame INTEGER NOT NULL,"
                        "subframe INTEGER NOT NULL,"
                        "slot INTEGER NOT NULL,"
                        "cellId INTEGER NOT NULL,"
                        "rnti INTEGER NOT NULL,"
                        "bwpId INTEGER NOT NULL,"
                        "ctrlMsgType VARCHAR(30) NOT NULL,"
                        "layer VARCHAR(30) NOT NULL,"
                        "entity VARCHAR(30) NOT NULL,"
                        "Seed INTEGER NOT NULL,"
                        "Run INTEGER NOT NULL,"
                        "TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlCtrlMsgsStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(),
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()), 
                                tableName);
}

void
SlCtrlMsgsStats::SaveCtrlMsgStats (std::string layer, std::string entity, uint16_t cellId, uint16_t rnti, const SfnSf &sfnsf, uint8_t bwpid, 
		std::string msg)
{
  SlCtrlMsgsCache c;
  c.timeInstance = Simulator::Now();
  c.cellId = cellId;
  c.rnti = rnti;
  c.sfnSf = sfnsf;
  c.bwpId = bwpid;
  c.msg = msg;
  c.layer = layer;
  c.entity = entity;

  m_ctrlMsgsCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_ctrlMsgsCache.size () * sizeof (SlCtrlMsgsStats) > m_writeSize) // 00000
    {
      WriteCache ();
    }
}

void
SlCtrlMsgsStats::EmptyCache()
{
  WriteCache ();
}

void
SlCtrlMsgsStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void 
SlCtrlMsgsStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_ctrlMsgsCache)
    {
      sqlite3_stmt *stmt;
      ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?);");
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 1, v.sfnSf.GetFrame ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 2, v.sfnSf.GetSubframe ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 3, v.sfnSf.GetSlot ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 4, v.cellId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 5, v.rnti);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 6, static_cast<uint32_t> (v.bwpId));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 7, v.msg);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, v.layer);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 9, v.entity);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 10, RngSeedManager::GetSeed ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 11, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      // NS_ASSERT (ret);
      // ret = m_db->Bind (stmt, 11, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
      NS_ASSERT (ret);
      // insert the timestamp
      ret = m_db->Bind (stmt, 12, v.timeInstance);
      NS_ASSERT (ret);
      ret = m_db->SpinExec (stmt);
      NS_ASSERT (ret);
    }
  m_ctrlMsgsCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3

