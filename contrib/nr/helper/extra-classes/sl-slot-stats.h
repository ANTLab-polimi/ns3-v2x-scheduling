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
#ifndef SL_PHY_SLOT_STATS_H
#define SL_PHY_SLOT_STATS_H

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/sfnsf.h>

#include <ns3/core-module.h>

namespace ns3 {

/**
 * \brief Class to collect and store the SINR values obtained from a simulation
 *
 * The class is meant to store in a database the slot stats during
 * a simulation. The class contains a cache, that after some time is written
 * to the disk.
 *
 * \see SetDb
 * \see SaveSinr
 * \see EmptyCache
 */
class SlPhySlotStats
{
public:
  /**
   * \brief Constructor
   */
  SlPhySlotStats ();

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
   * - "BwpId INTEGER NOT NULL,"
   * - "CellId INTEGER NOT NULL,"
   * - "rnti INTEGER NOT NULL,"
   * - "dataSym INTEGER NOT NULL,"
   * - "dataReg INTEGER NOT NULL,"
   * - "ctrlSym INTEGER NOT NULL,"
   * - "ctrlReg INTEGER NOT NULL,"
   * - "AvailableRb INTEGER NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "phySlotStats");

  /**
   * \brief Save the slot statistics
   * \param [in] sfnSf Slot number
   * \param [in] rnti The ue rnti
   * \param [in] dataReg Used Resource Element Group (1 sym x 1 RB)
   * \param [in] dataSym Used symbols
   * \param [in] ctrlReg Used Resource Element Group (1 sym x 1 RB)
   * \param [in] ctrlSym Used symbols
   * \param [in] availableRb Available RBs
   * \param [in] bwpId BWP ID
   * \param [in] cellId Cell ID
   */
  void SaveSlPhySlotStats (const SfnSf &sfnSf, uint16_t rnti, 
                        uint32_t dataSym, uint32_t dataReg,
                        uint32_t ctrlSym, uint32_t ctrlreg,
                        uint32_t availableRbs, uint16_t bwpId,
                        uint16_t cellId);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlPhySlotCache
  {
    SfnSf sfnSf;
//    uint64_t timeInstance;
    Time timeInstance;
    uint16_t rnti;
    uint32_t dataSym;
    uint32_t dataReg;
    uint32_t ctrlSym;
    uint32_t ctrlReg;
    uint32_t availableRb;
    uint16_t bwpId;
    uint16_t cellId;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SlPhySlotCache> m_phySlotCache;         //!< Result cache
  std::string m_tableName;                    //!< Table name
};

} // namespace ns3

#endif // PHY_SLOT_STATS_H
