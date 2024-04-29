/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "nr-dl-scheduling-stats.h"
#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlDlSchedulingStats::SlDlSchedulingStats ()
{
}

void
SlDlSchedulingStats::SetDb (SQLiteOutput *db, const std::string & tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName +
                        "(Frame INTEGER NOT NULL,"
                        "SubFrame INTEGER NOT NULL,"
                        "Slot INTEGER NOT NULL,"
                        "CellId INTEGER NOT NULL,"
                        "BwpId INTEGER NOT NULL,"
                        "TbSize INTEGER NOT NULL,"
                        "Mcs INTEGER NOT NULL,"
                        "Rnti INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	"Run INTEGER NOT NULL,"
		  	  	  	  	"TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlDlSchedulingStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlDlSchedulingStats::SaveSlDlSchedulingStats (uint16_t frame, uint8_t subframe, uint16_t slot, uint16_t cellId, uint32_t tbSize, uint8_t mcs,
                      uint16_t rnti, uint16_t bwpId)
{
  SlDlSchedulingCache c;
  c.timeInstance = Simulator::Now();
  c.frame = frame;
  c.subframe = subframe;
  c.slot = slot;
  c.tbSize = tbSize;
  c.mcs = mcs;
  c.rnti = rnti;
  c.bwpId = bwpId;
  c.cellId = cellId;

  m_dlSchedulingCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_dlSchedulingCache.size () * sizeof (SlDlSchedulingCache) > 10)
    {
      WriteCache ();
    }
}

void
SlDlSchedulingStats::EmptyCache()
{
  WriteCache ();
}

void
SlDlSchedulingStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlDlSchedulingStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_dlSchedulingCache)
    {
      sqlite3_stmt *stmt;
      ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?);");
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 1, static_cast<uint32_t> (v.frame));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 2, static_cast<uint32_t> (v.subframe));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 3, static_cast<uint32_t> (v.slot));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 4, v.cellId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 5, v.bwpId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 6, v.tbSize);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 7, static_cast<uint32_t> (v.mcs));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, v.rnti);
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
  m_dlSchedulingCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3
