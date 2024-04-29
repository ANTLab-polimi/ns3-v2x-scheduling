/*
 * ef-mac-rlc-buffer-status.h
 *
 *  Created on: Sep 7, 2022
 *      Author: ugjeci
 */

#ifndef SL_CONTRIB_HELPER_UE_BSR_STATS_H_
#define SL_CONTRIB_HELPER_UE_BSR_STATS_H_

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>

#include <ns3/core-module.h>

namespace ns3 {

    /**
 * \brief Class to collect and store the MAC buffer status report coming from Rlc
 *
 *
 * \see SetDb
 * \see SaveMacBaffurStatusReportStats
 * \see EmptyCache
 */
class SlMacUeBsrStats{
    public:
  /**
   * \brief Constructor
   */
	SlMacUeBsrStats ();

    /**
   * \brief Install the output dabase.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   *  The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "(*Frame INTEGER NOT NULL, "
   * - "SubFrame INTEGER NOT NULL,"
   * - "Slot INTEGER NOT NULL,"
   * - "cellId INTEGER NOT NULL,"
   * - "rnti INTEGER NOT NULL,"
   * - "currenttxqueuesize INTEGER NOT NULL,"
   * - "headoflinedelaynewtx INTEGER NOT NULL,"
   * - "sizependingstatusmsg INTEGER NOT NULL,"
   * - "currentrtxqueuesize INTEGER NOT NULL,"
   * - "headoflinedelayrtx INTEGER NOT NULL,"
   * - "lcid INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "macUeBsr");

  /**
   * \brief Save the slot statistics
   * \param [in] bwdid band
   * \param [in] tb Tb size
   */
  void SaveSlMacUeBsrStats (uint16_t frame, uint8_t subframe, uint16_t slot, uint16_t cellId, uint16_t rnti, 
  uint8_t lcid, uint8_t bsrid, uint32_t bufferSize);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlMacUeBsrCache
  {
    Time timeInstance;
    uint16_t frame; uint8_t subframe; uint16_t slot;
    uint16_t cellId; 
    uint16_t rnti;
    uint8_t lcid;
    uint8_t bsrid;
    uint32_t buffSize;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SlMacUeBsrCache> m_macUeBsrCache;         //!< Result cache
  std::string m_tableName;  


};

}



#endif /* CONTRIB_ELEPHANT_FLOW_HELPER_EF_MAC_RLC_BUFFER_STATUS_H_ */
