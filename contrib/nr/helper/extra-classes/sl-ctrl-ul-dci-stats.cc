/*
 * ef-slot-ctrl-output-stats.cc
 *
 *  Created on: Mar 23, 2022
 *      Author: ugjeci
 */


// #include "ef-slot-ctrl-output-stats.h"
#include "sl-ctrl-ul-dci-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlCtrlUlDciStats::SlCtrlUlDciStats ()
{
}

void
SlCtrlUlDciStats::SetDb (SQLiteOutput *db, const std::string & tableName, uint32_t writeSize)
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
                        "symStart INTEGER NOT NULL,"
                        "numSym INTEGER NOT NULL,"
                        // "mcs INTEGER NOT NULL,"
                        "mcs VARCHAR(400) NOT NULL,"
                        "harqProcess INTEGER NOT NULL,"
                        "rbgBitmask VARCHAR(500) NOT NULL,"
                        "dciFormat VARCHAR(10) NOT NULL,"
                        "typeMessage VARCHAR(10) NOT NULL,"
                        "ndi VARCHAR(100) NOT NULL,"
                        "rv VARCHAR(100) NOT NULL,"
                        "tpc INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	"Run INTEGER NOT NULL,"
		  	  	  	  	"TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlCtrlUlDciStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlCtrlUlDciStats::SaveSlCtrlUlDciStats (uint16_t cellId, uint16_t rnti, const SfnSf &sfnsf, uint8_t bwpid, 
		uint8_t symStart, uint8_t numSym, std::vector<uint8_t> mcs, uint8_t harqProcess, 
		std::string rbgBitmask, std::string dciFormat, 
		std::string ttiType, std::vector<uint8_t> ndi, std::vector<uint8_t> rv, uint8_t tpc)
{
  SlCtrlUlDciCache c;
  c.timeInstance = Simulator::Now();
  c.cellId = cellId;
  c.rnti = rnti;
  c.sfnSf = sfnsf;
  c.bwpId = bwpid;
  c.symStart= symStart;
  c.numSym = numSym;
  c.harqProcess = harqProcess;
  c.rbgBitmask = rbgBitmask;
  c.dciFormat = dciFormat;
  c.varTtiType = ttiType;
  // c.ndi = ndi;
  // c.rv = rv;
  c.tpc = tpc;
  // c.mcs = mcs;
  std::ostringstream oss;
  
  if (!mcs.empty()){
    std::vector<uint32_t> mcsInt32Vec(mcs.begin(), mcs.end());
    // Convert all but the last element to avoid a trailing ","
    std::copy(mcsInt32Vec.begin(), mcsInt32Vec.end()-1,
        std::ostream_iterator<uint32_t>(oss, ","));
    // Now add the last element with no delimiter
    oss << mcsInt32Vec.back();
  }
  c.mcs = oss.str();
  oss.str("");
  oss.clear();

  if (!ndi.empty()){
    std::vector<uint32_t> ndiInt32Vec(ndi.begin(), ndi.end());
    // Convert all but the last element to avoid a trailing ","
    std::copy(ndiInt32Vec.begin(), ndiInt32Vec.end()-1,
        std::ostream_iterator<uint32_t>(oss, ","));
    // Now add the last element with no delimiter
    oss << ndiInt32Vec.back();
  }
  c.ndi = oss.str();
  oss.str("");
  oss.clear();

  if (!rv.empty()){
    std::vector<uint32_t> rvInt32Vec(rv.begin(), rv.end());
    // Convert all but the last element to avoid a trailing ","
    std::copy(rvInt32Vec.begin(), rvInt32Vec.end()-1,
        std::ostream_iterator<uint32_t>(oss, ","));
    // Now add the last element with no delimiter
    oss << rvInt32Vec.back();
  }
  c.rv = oss.str();


  m_ctrlUlDciCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_ctrlUlDciCache.size () * sizeof (SlCtrlUlDciCache) > m_writeSize)// 00000
    {
      WriteCache ();
    }
}

void
SlCtrlUlDciStats::EmptyCache()
{
  WriteCache ();
}

void
SlCtrlUlDciStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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
SlCtrlUlDciStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_ctrlUlDciCache)
    {
      sqlite3_stmt *stmt;
      ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,? );");
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
      ret = m_db->Bind (stmt, 7, static_cast<uint32_t> (v.symStart));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, static_cast<uint32_t> (v.numSym));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 9, v.mcs);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 10, static_cast<uint32_t> (v.harqProcess));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 11, v.rbgBitmask);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 12, v.dciFormat);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 13, v.varTtiType);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 14, v.ndi);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 15, v.rv);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 16, static_cast<uint32_t> (v.tpc));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 17, RngSeedManager::GetSeed ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 18, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      // NS_ASSERT (ret);
      // ret = m_db->Bind (stmt, 18, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
      NS_ASSERT (ret);
      // insert the timestamp
      ret = m_db->Bind (stmt, 19, v.timeInstance);
      NS_ASSERT (ret);

      ret = m_db->SpinExec (stmt);
      NS_ASSERT (ret);
    }
  m_ctrlUlDciCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3

