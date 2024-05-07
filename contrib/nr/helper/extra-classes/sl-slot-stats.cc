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
#include "sl-slot-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>


// #include "ns3/mpi-interface.h"

namespace ns3 {

SlPhySlotStats::SlPhySlotStats ()
{
}

void
SlPhySlotStats::SetDb (SQLiteOutput *db, const std::string & tableName, uint32_t writeSize)
{
  m_db = db;
  m_tableName = tableName;
  m_writeSize = writeSize;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName +
                        "(Frame INTEGER NOT NULL,"
                        "SubFrame INTEGER NOT NULL,"
                        "Slot INTEGER NOT NULL,"
                        "rnti INTEGER NOT NULL,"
                        "BwpId INTEGER NOT NULL,"
                        "CellId INTEGER NOT NULL,"
                        "dataReg INTEGER NOT NULL,"
                        "dataSym INTEGER NOT NULL,"
                        "ctrlReg INTEGER NOT NULL,"
                        "ctrlSym INTEGER NOT NULL,"
                        "AvailableRb INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	"Run INTEGER NOT NULL,"
		  	  	  	  	"TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlPhySlotStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlPhySlotStats::SaveSlPhySlotStats (const SfnSf &sfnSf, uint16_t rnti,
                                uint32_t dataSym, uint32_t dataReg,
                                uint32_t ctrlSym, uint32_t ctrlReg,
                                uint32_t availableRb, uint16_t bwpId, uint16_t cellId)
{
  SlPhySlotCache c;
  c.timeInstance = Simulator::Now();
  c.sfnSf = sfnSf;
  c.rnti = rnti;
  c.dataReg = dataReg;
  c.dataSym = dataSym;
  c.ctrlSym = ctrlSym;
  c.ctrlReg = ctrlReg;
  c.availableRb = availableRb;
  c.bwpId = bwpId;
  c.cellId = cellId;

  m_phySlotCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_phySlotCache.size () * sizeof (SlPhySlotCache) > m_writeSize) // 1000000
    {
      WriteCache ();
    }
}

void
SlPhySlotStats::EmptyCache()
{
  WriteCache ();
}

void
SlPhySlotStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlPhySlotStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_phySlotCache)
    {
      sqlite3_stmt *stmt;
      ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 1, v.sfnSf.GetFrame ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 2, v.sfnSf.GetSubframe ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 3, v.sfnSf.GetSlot ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 4, v.rnti);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 5, v.bwpId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 6, v.cellId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 7, v.dataReg);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, v.dataSym);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 9, v.ctrlReg);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 10, v.ctrlSym);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 11, v.availableRb);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 12, RngSeedManager::GetSeed ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 13, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      NS_ASSERT (ret);
      // ret = m_db->Bind (stmt, 13, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
      // NS_ASSERT (ret);
      // insert the timestamp
      ret = m_db->Bind (stmt, 14, v.timeInstance);
      NS_ASSERT (ret);

      ret = m_db->SpinExec (stmt);
      NS_ASSERT (ret);
    }
  m_phySlotCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3
