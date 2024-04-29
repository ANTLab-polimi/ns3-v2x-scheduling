

#ifndef SL_EQUILATERAL_TRIANGLE_GRID_SCENARIO_HELPER_H
#define SL_EQUILATERAL_TRIANGLE_GRID_SCENARIO_HELPER_H

#include "node-distribution-scenario-interface.h"
#include <ns3/vector.h>
#include <ns3/random-variable-stream.h>

namespace ns3 {
    /**
 * @brief The HexagonalGridScenarioHelper class
 *
 * TODO: Documentation, tests
 */
class EquilateralTriangleGridScenarioHelper : public NodeDistributionScenarioInterface
{
public:

  /**
   * \brief EquilateralTriangleGridScenarioHelper
   */
  EquilateralTriangleGridScenarioHelper ();

  /**
   * \brief ~EquilateralTriangleGridScenarioHelper
   */
  virtual ~EquilateralTriangleGridScenarioHelper () override;

  /**
   * \brief Sets the number of outer rings of sites around the central site
   */
  void SetNumRings (uint8_t numRings);

  /**
   * \brief Gets the radius of the EquilateralTriangle cell
   * \returns Cell radius in meters
   */
  double GetEquilateralTriangleCellRadius () const;

  /**
   * \brief Returns the cell center coordinates
   * \param sitePos Site position coordinates
   * \param cellId Cell Id
   * \param numSecors The number of sectors of a site
   * \param hexagonRadius Radius of the EquilateralTriangle cell
   */
  Vector GetEquilateralTriangleCellCenter (const Vector &sitePos,
                                 uint16_t cellId) const;
  
  // inherited
  virtual void CreateScenario () override;

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

private:
  uint8_t m_numRings {0};  //!< Number of outer rings of sites around the central site
  Vector m_centralPos {Vector (0,0,0)};     //!< Central site position
  double m_equilateralTriangleRadius {0.0};  //!< Cell radius

  static std::vector<double> siteDistances;
  static std::vector<double> siteAngles;

  Ptr<UniformRandomVariable> m_r; //!< random variable used for the random generation of the radius
  Ptr<UniformRandomVariable> m_theta; //!< random variable used for the generation of angle
};
}


#endif // HEXAGONAL_GRID_SCENARIO_HELPER_H