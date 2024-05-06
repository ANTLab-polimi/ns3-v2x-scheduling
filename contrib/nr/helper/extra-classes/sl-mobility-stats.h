/*
 * ef-mac-sr-stats.h
 *
 *  Created on: Jan 21, 2022
 *      Author: ugjeci
 */

#ifndef SL_CONTRIB_HELPER_MOBILITY_H_
#define SL_CONTRIB_HELPER_MOBILITY_H_

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>

#include <ns3/core-module.h>

namespace ns3 {

/**
 * \brief Class to collect and store the MAC Scheduling request values obtained from a simulation
 *
 * The class is meant to store in a database the Uplink Tb Size values from UE or GNB during
 * a simulation. The class contains a cache, that after some time is written
 * to the disk.
 *
 * \see SetDb
 * \see SaveSlMacSRStats
 * \see EmptyCache
 */
class SlMobilityStats
{
public:
  /**
   * \brief Constructor
   */
	SlMobilityStats ();

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
  void SetDb (SQLiteOutput *db, const std::string& tableName = "mobility");

  /**
   * \brief Save the slot statistics
   * \param [in] pos bandwidth id
   * \param [in] vel rnti
   */
  void SaveSlMobilityStats (Vector pos, Vector vel, uint64_t id, std::string deviceType);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct MobilityCache
  {
    Time timeInstance;
    Vector pos;
    Vector vel;
    uint64_t id;
    std::string deviceType;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<MobilityCache> m_mobilityCache;         //!< Result cache
  std::string m_tableName;                    //!< Table name
};

} // namespace ns3




#endif /* CONTRIB_ELEPHANT_FLOW_HELPER_EF_MAC_SR_STATS_H_ */




