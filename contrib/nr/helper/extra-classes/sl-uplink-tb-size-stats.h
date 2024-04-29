/*
 * ef-uplink-tb-size.h
 *
 *  Created on: Nov 24, 2021
 *      Author: ugjeci
 */

#ifndef SL_CONTRIB_HELPER_UPLINK_TB_SIZE_STATS_H_
#define SL_CONTRIB_HELPER_UPLINK_TB_SIZE_STATS_H_

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>

#include <ns3/core-module.h>

namespace ns3 {

/**
 * \brief Class to collect and store the SINR values obtained from a simulation
 *
 * The class is meant to store in a database the Uplink Tb Size values from UE or GNB during
 * a simulation. The class contains a cache, that after some time is written
 * to the disk.
 *
 * \see SetDb
 * \see SaveUlTbSizeStats
 * \see EmptyCache
 */
class SlUplinkTbSizeStats
{
public:
  /**
   * \brief Constructor
   */
	SlUplinkTbSizeStats ();

  /**
   * \brief Install the output dabase.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   *  The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "(Imsi INTEGER NOT NULL,"
   * - "tb INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "ulTbSize");

  /**
   * \brief Save the slot statistics
   * \param [in] Imsi IMSI
   * \param [in] tb Tb size
   */
  void SaveUlTbSizeStats (uint64_t imsi, uint64_t tb);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlUplinkTbSizeCache
  {
    Time timeInstance;
    uint64_t imsi;
    uint64_t tb;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SlUplinkTbSizeCache> m_uplinkTbSizeCache;         //!< Result cache
  std::string m_tableName;                    //!< Table name
};

} // namespace ns3



#endif /* CONTRIB_ELEPHANT_FLOW_HELPER_EF_UPLINK_TB_SIZE_STATS_H_ */
