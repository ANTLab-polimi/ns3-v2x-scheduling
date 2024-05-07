/*
 * ef-ctrl-ul-dci-stats.h
 *
 *  Created on: Mar 23, 2022
 *      Author: ugjeci
 */

#ifndef SL_CONTRIB_HELPER_CTRL_UL_DCI_STATS_H_
#define SL_CONTRIB_HELPER_CTRL_UL_DCI_STATS_H_

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
class SlCtrlUlDciStats
{
public:
  /**
   * \brief Constructor
   */
	SlCtrlUlDciStats ();

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
   * - "rnti INTEGER NOT NULL,"
   * - "BwpId INTEGER NOT NULL,"
   * - "symStart INTEGER NOT NULL,"
   * - "numSym INTEGER NOT NULL,"
   * - "mcs INTEGER NOT NULL,"
   * - "harqProcess INTEGER NOT NULL,"
   * - "harqProcess INTEGER NOT NULL,"
   * - "harqProcess INTEGER NOT NULL,"
   * - "ndi INTEGER NOT NULL,"
   * - "rv INTEGER NOT NULL,"
   * - "tpc INTEGER NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "ctrlUlDciStats", uint32_t writeSize = 100000);

  /**
   * \brief Save the slot statistics
   * \param [in] sfnSf Slot number
   * \param [in] rnti rnti
   * \param [in] bwpId BWP ID
   * \param [in] cellId Cell ID
   */
  void SaveSlCtrlUlDciStats (uint16_t cellId, uint16_t rnti, const SfnSf &sfnsf, uint8_t bwpid, 
		uint8_t symStart, uint8_t numSym, std::vector<uint8_t> mcs, uint8_t harqProcess, 
		std::string rbgBitmask, std::string dciFormat, 
		std::string ttiType, std::vector<uint8_t> ndi, std::vector<uint8_t> rv, uint8_t tpc);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlCtrlUlDciCache
  {
    SfnSf sfnSf;
    Time timeInstance;
    uint16_t cellId;
    uint16_t rnti;
    uint8_t bwpId;
    uint8_t symStart;
    uint8_t numSym;
    uint32_t tbSize;
    uint8_t harqProcess;
    std::string rbgBitmask;
    std::string dciFormat;
    std::string varTtiType;
    uint8_t tpc;
    
    // uint8_t mcs;
    //  uint8_t ndi;
    // uint8_t rv;
    std::string mcs;
    std::string ndi;
    std::string rv;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SlCtrlUlDciCache> m_ctrlUlDciCache;         //!< Result cache
  std::string m_tableName;                    //!< Table name
  uint32_t m_writeSize;
};

} // namespace ns3



#endif /* CONTRIB_ELEPHANT_FLOW_HELPER_EF_CTRL_UL_DCI_STATS_H_ */
