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
#ifndef SL_DL_SCHEDULING_STATS_H
#define SL_DL_SCHEDULING_STATS_H

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/sfnsf.h>

#include <ns3/core-module.h>

namespace ns3 {

/**
 * \brief Class to collect and store the SINR values obtained from a simulation
 *
 * The class is meant to store in a database the SINR values from UE or GNB during
 * a simulation. The class contains a cache, that after some time is written
 * to the disk.
 *
 * \see SetDb
 * \see SaveSinr
 * \see EmptyCache
 */
class SlDlSchedulingStats
{
public:
  /**
   * \brief Constructor
   */
  SlDlSchedulingStats ();

  /**
   * \brief Install the output dabase.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   *  The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "(Frame INTEGER NOT NULL, "
   * - "SubFrame INTEGER NOT NULL,"
   * - "Slot INTEGER NOT NULL,"
   * - "CellId INTEGER NOT NULL,"
   * - "TbSize INTEGER NOT NULL,"
   * - "Mcs INTEGER NOT NULL,"
   * - "Rnti INTEGER NOT NULL,"
   * - "BwpId INTEGER NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "slDlSchedulingStats", uint32_t writeSize = 100000);

  /**
   * \brief Save the slot statistics
   * \param [in] sfnSf Slot number
   * \param [in] TbSize Transport block size
   * \param [in] Mcs Modulation & Coding Scheme
   * \param [in] Rnti rnti
   * \param [in] bwpId BWP ID
   * \param [in] cellId Cell ID
   */
  void SaveSlDlSchedulingStats (uint16_t frame, uint8_t subframe, uint16_t slot, uint16_t cellId, uint32_t tbSize, uint8_t mcs,
                      uint16_t rnti, uint16_t bwpId);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlDlSchedulingCache
  {
    uint16_t frame;
    uint8_t subframe;
    uint16_t slot;
//    uint64_t timeInstance;
    Time timeInstance;
    uint32_t tbSize;
    uint8_t mcs;
    uint16_t rnti;
    uint16_t bwpId;
    uint16_t cellId;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SlDlSchedulingCache> m_dlSchedulingCache;         //!< Result cache
  std::string m_tableName;                    //!< Table name
  uint32_t m_writeSize;
};

} // namespace ns3

#endif // DL_SCHEDULING_STATS_H
