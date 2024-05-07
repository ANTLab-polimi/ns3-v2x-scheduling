

#ifndef SL_UE_PSS_RECEIVED_STATS_H
#define SL_UE_PSS_RECEIVED_STATS_H

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/sfnsf.h>

#include <ns3/core-module.h>

namespace ns3 {
class SlUePssReceivedStats{

public:

    SlUePssReceivedStats();

    /**
   * \brief Install the output dabase.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   *  The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "CellId INTEGER NOT NULL, "
   * - "BwpId INTEGER NOT NULL,"
   * - "Rnti INTEGER NOT NULL,"
   * - "AvgSinr DOUBLE NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "uePSSReceived", uint32_t writeSize = 100000);

  /**
   * \brief Store the SINR values
   * \param cellId Cell ID
   * \param rnti RNTI
   * \param power power (unused)
   * \param power The sum of received power in the entire bandwidth
   * \param nrb the number of rbs upon which the pss is received
   *
   * The method saves the result in a cache, and if it is necessary, writes the
   * cache to disk before emptying it.
   */
  void SavePss (const SfnSf &sfnsf, uint16_t cellId, uint16_t bwpId, uint16_t rnti, uint64_t imsi, uint16_t generatingCellId, double power, uint16_t nRb, double rsrq, uint16_t nRb_rsrq);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlUePssCache
  {

    Time timeInstance;
	uint16_t cellId;
    uint16_t generatingCellId;
	uint16_t bwpId;
	uint16_t rnti;
    uint64_t imsi;
    SfnSf sfnsf;
	uint16_t nrb;
	double rsrp;
  double rsrq; 
  uint16_t nRb_rsrq;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SlUePssCache> m_uePssCache;   //!< Result cache
  std::string m_tableName;                    //!< Table name
  uint32_t m_writeSize;
};
}

#endif // UE_PSS_RECEIVED_STATS_H