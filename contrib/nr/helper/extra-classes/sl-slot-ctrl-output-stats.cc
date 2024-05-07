/*
 * ef-slot-ctrl-output-stats.cc
 *
 *  Created on: Mar 23, 2022
 *      Author: ugjeci
 */


#include "sl-slot-ctrl-output-stats.h"

#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>
#include <ns3/core-module.h>
#include <ns3/nr-module.h>
#include <ns3/sqlite-output.h>

// #include "ns3/mpi-interface.h"

namespace ns3 {

SlSlotCtrlOutputStats::SlSlotCtrlOutputStats ()
{
}

void
SlSlotCtrlOutputStats::SetDb (SQLiteOutput *db, const std::string & tableName, uint32_t writeSize)
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
                        "BwpId INTEGER NOT NULL,"
                        "CellId INTEGER NOT NULL,"
                        "ScheduledUe INTEGER NOT NULL,"
                        "UsedReg INTEGER NOT NULL,"
                        "UsedSym INTEGER NOT NULL,"
                        "AvailableRb INTEGER NOT NULL,"
                        "AvailableSym INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
		  	  	  	  	"Run INTEGER NOT NULL,"
		  	  	  	  	"TimeStamp DOUBLE NOT NULL);");
//                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  SlSlotCtrlOutputStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                RngSeedManager::GetRun(), 
                                // static_cast<uint32_t> (MpiInterface::GetSystemId ()),
                                tableName);
}

void
SlSlotCtrlOutputStats::SaveSlotStats (const SfnSf &sfnSf,
                                uint32_t scheduledUe, uint32_t usedReg,
                                uint32_t usedSym, uint32_t availableRb,
                                uint32_t availableSym, uint16_t bwpId, uint16_t cellId)
{
  SlSlotCache c;
  c.timeInstance = Simulator::Now();
  c.sfnSf = sfnSf;
  c.scheduledUe = scheduledUe;
  c.usedReg = usedReg;
  c.usedSym = usedSym;
  c.availableRb = availableRb;
  c.availableSym = availableSym;
  c.bwpId = bwpId;
  c.cellId = cellId;

  m_slotCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_slotCache.size () * sizeof (SlSlotCache) > m_writeSize) // 1000000
    {
      WriteCache ();
    }
}

void
SlSlotCtrlOutputStats::EmptyCache()
{
  WriteCache ();
}

void
SlSlotCtrlOutputStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void SlSlotCtrlOutputStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_slotCache)
    {
      sqlite3_stmt *stmt;
      ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);");
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 1, v.sfnSf.GetFrame ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 2, v.sfnSf.GetSubframe ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 3, v.sfnSf.GetSlot ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 4, v.bwpId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 5, v.cellId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 6, v.scheduledUe);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 7, v.usedReg);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, v.usedSym);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 9, v.availableRb);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 10, v.availableSym);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 11, RngSeedManager::GetSeed ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 12, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      // NS_ASSERT (ret);
      // ret = m_db->Bind (stmt, 12, static_cast<uint32_t> (MpiInterface::GetSystemId ()));
      NS_ASSERT (ret);
      // insert the timestamp
      ret = m_db->Bind (stmt, 13, v.timeInstance);
      NS_ASSERT (ret);

      ret = m_db->SpinExec (stmt);
      NS_ASSERT (ret);
    }
  m_slotCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3

