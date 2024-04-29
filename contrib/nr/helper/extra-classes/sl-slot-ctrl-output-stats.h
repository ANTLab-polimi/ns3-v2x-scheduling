/*
 * ef-slot-ctrl-output-stats.h
 *
 *  Created on: Mar 23, 2022
 *      Author: ugjeci
 */

#ifndef SL_CONTRIB_HELPER_SLOT_CTRL_OUTPUT_STATS_H_
#define SL_CONTRIB_HELPER_SLOT_CTRL_OUTPUT_STATS_H_

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
class SlSlotCtrlOutputStats
{
public:
  /**
   * \brief Constructor
   */
	SlSlotCtrlOutputStats ();

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
   * - "ActiveUe INTEGER NOT NULL,"
   * - "UsedReg INTEGER NOT NULL,"
   * - "AvailableRb INTEGER NOT NULL,"
   * - "AvailableSym INTEGER NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "slotCtrlStats");

  /**
   * \brief Save the slot statistics
   * \param [in] sfnSf Slot number
   * \param [in] scheduledUe The number of scheduled UE in the slot
   * \param [in] usedReg Used Resource Element Group (1 sym x 1 RB)
   * \param [in] usedSym Used symbols
   * \param [in] availableRb Available RBs
   * \param [in] availableSym Available symbols
   * \param [in] bwpId BWP ID
   * \param [in] cellId Cell ID
   */
  void SaveSlotStats (const SfnSf &sfnSf, uint32_t scheduledUe, uint32_t usedReg,
                      uint32_t usedSym, uint32_t availableRb,
                      uint32_t availableSym, uint16_t bwpId,
                      uint16_t cellId);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlSlotCache
  {
    SfnSf sfnSf;
//    uint64_t timeInstance;
    Time timeInstance;
    uint32_t scheduledUe;
    uint32_t usedReg;
    uint32_t usedSym;
    uint32_t availableRb;
    uint32_t availableSym;
    uint16_t bwpId;
    uint16_t cellId;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SlSlotCache> m_slotCache;         //!< Result cache
  std::string m_tableName;                    //!< Table name
};

} // namespace ns3



#endif /* CONTRIB_ELEPHANT_FLOW_HELPER_EF_SLOT_CTRL_OUTPUT_STATS_H_ */
