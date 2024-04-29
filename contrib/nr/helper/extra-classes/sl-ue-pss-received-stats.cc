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
#include "sl-ue-pss-received-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlUePssReceivedStats::SlUePssReceivedStats ()
{
}

void
SlUePssReceivedStats::SetDb (SQLiteOutput *db, const std::string & tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  // drop table 
  // ret = m_db->SpinExec("DROP TABLE IF EXISTS " + tableName + ";");
  // NS_ASSERT (ret);

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName + " (" +
                        "frame INTEGER NOT NULL,"
                        "subframe INTEGER NOT NULL,"
                        "slot INTEGER NOT NULL,"
					              "cellid INTEGER NOT NULL, "
						            "bwpid INTEGER NOT NULL,"
					              "rnti INTEGER NOT NULL,"
                        "imsi INTEGER NOT NULL,"
                        "generatingCellId INTEGER NOT NULL,"
						            "rsrp DOUBLE NOT NULL,"
					              "nrRbs INTEGER NOT NULL,"
                        "rsrq DOUBLE NOT NULL,"
					              "nrRbs_rsrq INTEGER NOT NULL,"
					              "Seed INTEGER NOT NULL,"
						            "Run INTEGER NOT NULL,"
					              "TimeStamp DOUBLE NOT NULL);");
		  //                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlUePssReceivedStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlUePssReceivedStats::SavePss (const SfnSf &sfnsf, uint16_t cellId, uint16_t bwpId, uint16_t rnti, uint64_t imsi, uint16_t generatingCellId, double power, uint16_t nRb, double rsrq, uint16_t nRb_rsrq)
{
	SlUePssCache c;
	c.timeInstance = Simulator::Now();
	c.bwpId = bwpId;
	c.cellId = cellId;
	c.rnti = rnti;
	c.rsrp = power;
  c.rsrq = rsrq;
  c.imsi=imsi;
  c.sfnsf = sfnsf;
  c.nrb = nRb;
  c.nRb_rsrq = nRb_rsrq;
  c.generatingCellId = generatingCellId;

	m_uePssCache.emplace_back (c);

//  m_sinrCache.emplace_back (SinrResultCache (cellId, bwpId, rnti, avgSinr));

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_uePssCache.size () * sizeof (SlUePssCache) > 100) // 0000
    {
      WriteCache ();
    }
}

void
SlUePssReceivedStats::EmptyCache()
{
  WriteCache ();
}

void
SlUePssReceivedStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlUePssReceivedStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_uePssCache)
    {
      sqlite3_stmt *stmt;
      ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 1, v.sfnsf.GetFrame ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 2, static_cast<uint32_t> (v.sfnsf.GetSubframe ()));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 3, v.sfnsf.GetSlot ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 4, v.cellId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 5, v.bwpId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 6, v.rnti);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 7, static_cast<uint32_t> (v.imsi));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, static_cast<uint32_t> (v.generatingCellId));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 9, v.rsrp); // static_cast<double> (
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 10, v.nrb);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 11, v.rsrq); // static_cast<double> (
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 12, v.nRb_rsrq);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 13, RngSeedManager::GetSeed ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 14, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      // NS_ASSERT (ret);
      // ret = m_db->Bind (stmt, 14, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
      NS_ASSERT (ret);
      // 
      // insert the timestamp
      ret = m_db->Bind (stmt, 15, v.timeInstance);
      NS_ASSERT (ret);
      ret = m_db->SpinExec (stmt);
      NS_ASSERT (ret);
    }
  m_uePssCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3
