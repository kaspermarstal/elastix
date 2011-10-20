/*======================================================================

  This file is part of the elastix software.

  Copyright (c) University Medical Center Utrecht. All rights reserved.
  See src/CopyrightElastix.txt or http://elastix.isi.uu.nl/legal.php for
  details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE. See the above copyright notices for more information.

======================================================================*/

#ifndef __elxOptimizerBase_hxx
#define __elxOptimizerBase_hxx

#include "elxOptimizerBase.h"

#include "itkSingleValuedNonLinearOptimizer.h"
#include "itk_zlib.h"


namespace elastix
{
using namespace itk;

/**
 * ****************** Constructor ***********************************
 */

template <class TElastix>
OptimizerBase<TElastix>
::OptimizerBase()
{
  this->m_NewSamplesEveryIteration = false;

} // end Constructor


/**
 * ****************** SetCurrentPositionPublic ************************
 *
 * Add empty SetCurrentPositionPublic, so it is known everywhere.
 */

template <class TElastix>
void
OptimizerBase<TElastix>
::SetCurrentPositionPublic( const ParametersType & /** param */ )
{
  xl::xout["error"] << "ERROR: This function should be overridden or just "
    << "not used.\n";
  xl::xout["error"] << "  Are you using BSplineTransformWithDiffusion in "
    << "combination with another optimizer than the "
    << "StandardGradientDescentOptimizer? Don't!" << std::endl;

  /** Throw an exception if this function is not overridden. */
  itkExceptionMacro( << "ERROR: The SetCurrentPositionPublic method is not "
    << "implemented in your optimizer" );

} // end SetCurrentPositionPublic()


/**
 * ****************** BeforeEachResolutionBase **********************
 */

template <class TElastix>
void
OptimizerBase<TElastix>
::BeforeEachResolutionBase( void )
{
  /** Get the current resolution level. */
  unsigned int level
    = this->GetRegistration()->GetAsITKBaseType()->GetCurrentLevel();

  /** Check if after every iteration a new sample set should be created. */
  this->m_NewSamplesEveryIteration = false;
  this->GetConfiguration()->ReadParameter( this->m_NewSamplesEveryIteration,
    "NewSamplesEveryIteration", this->GetComponentLabel(), level, 0 );

} // end BeforeEachResolutionBase()


/**
 * ****************** AfterRegistrationBase **********************
 */

template <class TElastix>
void
OptimizerBase<TElastix>
::AfterRegistrationBase( void )
{
  typedef long IntParametersValueType;
  typedef Array<IntParametersValueType> IntParametersType; 

  /** Get the final parameters, round to six decimals and store as int. */
  ParametersType finalTP = this->GetAsITKBaseType()->GetCurrentPosition();
  const unsigned long N = finalTP.GetSize();
  IntParametersType intTP( N );
  for (unsigned int i = 0; i < N; ++i )
  {
    intTP[i] = static_cast<IntParametersValueType>( itk::Math::Round( finalTP[i] * 1e6 ) );
  }
 
  /** Compute the crc checksum using zlib crc32 function. */
  const unsigned char * crcInputData = reinterpret_cast<const unsigned char *>( intTP.data_block() );
  uLong crc = crc32(0L, Z_NULL, 0);
  crc = crc32(crc, crcInputData, intTP.Size()* sizeof(IntParametersValueType) );
  
  elxout << "\nRegistration result checksum: "
    << crc
    << std::endl;

} // end AfterRegistrationBase()


/**
 * ****************** SelectNewSamples ****************************
 */

template <class TElastix>
void
OptimizerBase<TElastix>
::SelectNewSamples( void )
{
  /** Force the metric to base its computation on a new subset of image samples.
   * Not every metric may have implemented this.
   */
  for ( unsigned int i = 0; i < this->GetElastix()->GetNumberOfMetrics(); ++i )
  {
    this->GetElastix()->GetElxMetricBase(i)->SelectNewSamples();
  }

} // end SelectNewSamples()


/**
 * ****************** GetNewSamplesEveryIteration ********************
 */

template <class TElastix>
bool
OptimizerBase<TElastix>
::GetNewSamplesEveryIteration( void ) const
{
  /** itkGetConstMacro Without the itkDebugMacro. */
  return this->m_NewSamplesEveryIteration;

} // end GetNewSamplesEveryIteration()


/**
 * ****************** SetSinusScales ********************
 */

template <class TElastix>
void
OptimizerBase<TElastix>
::SetSinusScales( double amplitude, double frequency,
  unsigned long numberOfParameters )
{
  typedef typename ITKBaseType::ScalesType ScalesType;

  const double nrofpar = static_cast<double>( numberOfParameters );
  ScalesType scales( numberOfParameters );

  for ( unsigned long i = 0; i < numberOfParameters; ++i )
  {
    const double x = static_cast<double>( i ) / nrofpar * 2.0
      * vnl_math::pi * frequency;
    scales[ i ] = vcl_pow( amplitude, vcl_sin( x ) );
  }
  this->GetAsITKBaseType()->SetScales( scales );

} // end SetSinusScales()


} // end namespace elastix

#endif // end #ifndef __elxOptimizerBase_hxx
