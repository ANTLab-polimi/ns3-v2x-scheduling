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
#include "sl-rb-output-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlRbOutputStats::SlRbOutputStats ()
{
}

void
SlRbOutputStats::SetDb (SQLiteOutput *db, const std::string & tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName +
                        "(Frame INTEGER NOT NULL, "
                        "SubFrame INTEGER NOT NULL,"
                        "Slot INTEGER NOT NULL,"
                        "Symbol INTEGER NOT NULL,"
                        "RBIndexActive INTEGER NOT NULL,"
                        "BwpId INTEGER NOT NULL,"
                        "CellId INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	"Run INTEGER NOT NULL,"
						"TimeStamp DOUBLE NOT NULL);");
//                      "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlRbOutputStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                              RngSeedManager::GetRun(), 
                              // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                              tableName);
}

void
SlRbOutputStats::SaveRbStats (const SfnSf &sfnSf, uint8_t sym, const std::vector<int> rbUsed,
                            uint16_t bwpId, uint16_t cellId)
{
  SlRbCache c;
  c.sym = sym;
  c.timeInstance = Simulator::Now();
  c.sfnSf = sfnSf;
  c.rbUsed = rbUsed;
  c.bwpId = bwpId;
  c.cellId = cellId;

  m_slotCache.emplace_back (c);

  // Let's wait until ~500 KB of entries before storing it in the database.
  // the entries value is an approximation, as the vector of RB size can
  // vary.
  if (m_slotCache.size () * c.GetSize () > 100) // 1000000
    {
      WriteCache ();
    }
}

void
SlRbOutputStats::EmptyCache()
{
  WriteCache ();
}

void
SlRbOutputStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlRbOutputStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_slotCache)
    {
      for (const auto & rb : v.rbUsed)
        {
          sqlite3_stmt *stmt;
          ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?);");
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 1, v.sfnSf.GetFrame ());
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 2, v.sfnSf.GetSubframe ());
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 3, v.sfnSf.GetSlot ());
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 4, v.sym);
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 5, rb);
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 6, v.bwpId);
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 7, v.cellId);
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 8, RngSeedManager::GetSeed ());
          NS_ASSERT (ret);
          ret = m_db->Bind (stmt, 9, static_cast<uint32_t> (RngSeedManager::GetRun ()));
          // NS_ASSERT (ret);
          // ret = m_db->Bind (stmt, 9, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
          NS_ASSERT (ret);
          // insert the timestamp
          ret = m_db->Bind (stmt, 10, v.timeInstance);
          NS_ASSERT (ret);

          ret = m_db->SpinExec (stmt);
          NS_ASSERT (ret);
        }
    }
  m_slotCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3
