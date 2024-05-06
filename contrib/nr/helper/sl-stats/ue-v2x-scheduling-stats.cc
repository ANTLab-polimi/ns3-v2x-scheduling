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
#include "ue-v2x-scheduling-stats.h"

#include <ns3/core-module.h>

namespace ns3 {

UeV2XScheduling::UeV2XScheduling ()
{
}

void
UeV2XScheduling::SetDb (SQLiteOutput *db, const std::string &tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  ret = db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "timeMs DOUBLE NOT NULL, "
                      "imsi INTEGER NOT NULL,"
                      "rnti INTEGER NOT NULL,"
                      "frame INTEGER NOT NULL,"
                      "subFrame INTEGER NOT NULL,"
                      "slot INTEGER NOT NULL,"
                      "ndi INTEGER NOT NULL,"
                      "rv INTEGER NOT NULL,"
                      "lcid INTEGER NOT NULL,"
                      "tbSize INTEGER NOT NULL,"
                      "priority INTEGER NOT NULL,"
                      "mcs INTEGER NOT NULL,"
                      "dstL2Id INTEGER NOT NULL,"
                      "symStart INTEGER NOT NULL,"
                      "symLen INTEGER NOT NULL,"
                      "sbChStart INTEGER NOT NULL,"
                      "sbChLen INTEGER NOT NULL,"
                      "maxNumPerReserve INTEGER NOT NULL,"
                      "numSlPscchRbs INTEGER NOT NULL,"
                      "slPscchSymStart INTEGER NOT NULL,"
                      "slPscchSymLength INTEGER NOT NULL,"
                      "txSciA INTEGER NOT NULL,"
                      "slotNumInd INTEGER NOT NULL,"
                      "cReselCounter INTEGER NOT NULL,"
                      "slResoReselCounter INTEGER NOT NULL,"
                      "prevSlResoReselCounter INTEGER NOT NULL,"
                      "nrSlHarqId INTEGER NOT NULL,"
                      "nSelected INTEGER NOT NULL,"
                      "tbTxCounter INTEGER NOT NULL,"
                      "plmnId VARCHAR(30) NOT NULL,"
                      "currframe INTEGER NOT NULL,"
                      "currsubFrame INTEGER NOT NULL,"
                      "currslot INTEGER NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");

  NS_ABORT_UNLESS (ret);

  UeV2XScheduling::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                       RngSeedManager::GetRun(), tableName);
}

void
UeV2XScheduling::Save (uint64_t ueId, NrSlGrantInfo nrSlGrantInfo, std::string plmnId, const SfnSf & sfnsf)
{
    UeV2XSchedulingParameters data (ueId);

    data.timeMs = Simulator::Now().GetMilliSeconds();
    data.nrSlGrantInfo = nrSlGrantInfo;
    data.plmnId = plmnId;
    data.currSfnSf = sfnsf;
    m_v2xXAppSchedulingCache.emplace_back (data);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_v2xXAppSchedulingCache.size () * sizeof (UeV2XSchedulingParameters) > 1000)
    {
      WriteCache ();
    }
}

void
UeV2XScheduling::SaveUeSched (uint64_t ueId, NrSlGrantInfo nrSlGrantInfo)
{
    UeV2XSchedulingParameters data (ueId);

    data.timeMs = Simulator::Now().GetMilliSeconds();
    data.nrSlGrantInfo = nrSlGrantInfo;
    data.plmnId = "";
    data.currSfnSf = SfnSf();
    m_v2xXAppSchedulingCache.emplace_back (data);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_v2xXAppSchedulingCache.size () * sizeof (UeV2XSchedulingParameters) > 1000)
    {
      WriteCache ();
    }
}

void
UeV2XScheduling::EmptyCache()
{
  WriteCache ();
}

void
UeV2XScheduling::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_v2xXAppSchedulingCache)
    {
        // iterate over different nrslgrant
        for (std::set<ns3::NrSlSlotAlloc>::iterator nrSlSlotAllocIt = v.nrSlGrantInfo.slotAllocations.begin();
            nrSlSlotAllocIt!=v.nrSlGrantInfo.slotAllocations.end(); ++nrSlSlotAllocIt){
            // iterate over the sl rlc pdu
            for (const auto & slRlcPduInfo : nrSlSlotAllocIt->slRlcPduInfo){
                sqlite3_stmt *stmt;
                m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
                ret = m_db->Bind (stmt, 1, v.timeMs);
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 2, static_cast<uint32_t> (v.imsi));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 3, static_cast<uint32_t> (v.imsi));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 4, static_cast<uint32_t> (nrSlSlotAllocIt->sfn.GetFrame()));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 5, static_cast<uint32_t> (nrSlSlotAllocIt->sfn.GetSubframe()));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 6, static_cast<uint32_t> (nrSlSlotAllocIt->sfn.GetSlot()));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 7, static_cast<uint32_t>(nrSlSlotAllocIt->ndi));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 8, static_cast<uint32_t>(nrSlSlotAllocIt->rv));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 9, static_cast<uint32_t>(slRlcPduInfo.lcid));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 10, static_cast<uint32_t> (slRlcPduInfo.size));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 11, static_cast<uint32_t>(nrSlSlotAllocIt->priority));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 12, static_cast<uint32_t>(nrSlSlotAllocIt->mcs));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 13, static_cast<uint32_t>(nrSlSlotAllocIt->dstL2Id));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 14, static_cast<uint32_t>(nrSlSlotAllocIt->slPsschSymStart));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 15, static_cast<uint32_t>(nrSlSlotAllocIt->slPsschSymLength));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 16, static_cast<uint32_t>(nrSlSlotAllocIt->slPsschSubChStart));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 17, static_cast<uint32_t>(nrSlSlotAllocIt->slPsschSubChLength));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 18, static_cast<uint32_t>(nrSlSlotAllocIt->maxNumPerReserve));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 19, static_cast<uint32_t>(nrSlSlotAllocIt->numSlPscchRbs));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 20, static_cast<uint32_t>(nrSlSlotAllocIt->slPscchSymStart));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 21, static_cast<uint32_t>(nrSlSlotAllocIt->slPscchSymLength));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 22, static_cast<uint32_t>(nrSlSlotAllocIt->txSci1A));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 23, static_cast<uint32_t>(nrSlSlotAllocIt->slotNumInd));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 24, static_cast<uint32_t>(v.nrSlGrantInfo.cReselCounter));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 25, static_cast<uint32_t>(v.nrSlGrantInfo.slResoReselCounter));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 26, static_cast<uint32_t>(v.nrSlGrantInfo.prevSlResoReselCounter));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 27, static_cast<uint32_t>(v.nrSlGrantInfo.nrSlHarqId));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 28, static_cast<uint32_t>(v.nrSlGrantInfo.nSelected));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 29, static_cast<uint32_t>(v.nrSlGrantInfo.tbTxCounter));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 30, v.plmnId);
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 31, static_cast<uint32_t> (v.currSfnSf.GetFrame()));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 32, static_cast<uint32_t> (v.currSfnSf.GetSubframe()));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 33, static_cast<uint32_t> (v.currSfnSf.GetSlot()));
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 34, RngSeedManager::GetSeed ());
                NS_ABORT_UNLESS (ret);
                ret = m_db->Bind (stmt, 35, static_cast<uint32_t> (RngSeedManager::GetRun ()));
                NS_ABORT_UNLESS (ret);
                ret = m_db->SpinExec (stmt);
                NS_ABORT_UNLESS (ret);
            }
        }
    }

  m_v2xXAppSchedulingCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ABORT_UNLESS (ret);
}

void
UeV2XScheduling::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

} // namespace ns3
