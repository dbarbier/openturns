//                                               -*- C++ -*-
/**
 *  @brief Class for the InverseNataf transformation evaluation for elliptical
 *
 *  Copyright 2005-2017 Airbus-EDF-IMACS-Phimeca
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "openturns/InverseNatafEllipticalCopulaEvaluation.hxx"
#include "openturns/PersistentObjectFactory.hxx"

BEGIN_NAMESPACE_OPENTURNS

/*
 * @class InverseNatafEllipticalCopulaEvaluation
 *
 * This class offers an interface for the InverseNataf function for elliptical copula
 */

CLASSNAMEINIT(InverseNatafEllipticalCopulaEvaluation)

static const Factory<InverseNatafEllipticalCopulaEvaluation> Factory_InverseNatafEllipticalCopulaEvaluation;

/* Default constructor */
InverseNatafEllipticalCopulaEvaluation::InverseNatafEllipticalCopulaEvaluation()
  : EvaluationImplementation()
  , standardDistribution_()
  , cholesky_()
{
  // Nothing to do
}

/* Parameter constructor */
InverseNatafEllipticalCopulaEvaluation::InverseNatafEllipticalCopulaEvaluation(const Distribution & standardDistribution,
    const TriangularMatrix & cholesky)
  : EvaluationImplementation()
  , standardDistribution_(standardDistribution)
  , cholesky_(cholesky)
{
  Description description(Description::BuildDefault(cholesky_.getDimension(), "x"));
  description.add(Description::BuildDefault(cholesky_.getDimension(), "y"));
  setDescription(description);
}

/* Virtual constructor */
InverseNatafEllipticalCopulaEvaluation * InverseNatafEllipticalCopulaEvaluation::clone() const
{
  return new InverseNatafEllipticalCopulaEvaluation(*this);
}

/* String converter */
String InverseNatafEllipticalCopulaEvaluation::__repr__() const
{
  OSS oss;
  oss << "class=" << InverseNatafEllipticalCopulaEvaluation::GetClassName()
      << " description=" << getDescription()
      << " standardDistribution=" << standardDistribution_
      << " cholesky=" << cholesky_;

  return oss;
}

String InverseNatafEllipticalCopulaEvaluation::__str__(const String & offset) const
{
  OSS oss(false);
  oss << offset << InverseNatafEllipticalCopulaEvaluation::GetClassName()
      << "("  << standardDistribution_ << "->Copula(cholesky=" << cholesky_ << ", E=" << standardDistribution_.getMarginal(0) << "))";

  return oss;
}

/*
 * Evaluation
 * This function transforms a standard elliptical distribution into an elliptical copula of the same kind with a correlation matrix R:
 * Z(u) = L.u, where L is the Cholesky factor of R: L.L^t = R, L is lower triangular
 * Si(u) = F(Zi), where F is the CDF of the standard elliptical distribution
 */
Point InverseNatafEllipticalCopulaEvaluation::operator () (const Point & inP) const
{
  const UnsignedInteger dimension = getInputDimension();
  // First, correlate the components
  Point result(cholesky_ * inP);
  const Distribution standardMarginal(standardDistribution_.getMarginal(0));
  // Second, apply the commmon marginal distribution
  for (UnsignedInteger i = 0; i < dimension; ++i) result[i] = standardMarginal.computeCDF(Point(1, result[i]));
  ++callsNumber_;
  if (isHistoryEnabled_)
  {
    inputStrategy_.store(inP);
    outputStrategy_.store(result);
  }
  return result;
}

Sample InverseNatafEllipticalCopulaEvaluation::operator () (const Sample & inSample) const
{
  const UnsignedInteger dimension = getInputDimension();
  const UnsignedInteger size = inSample.getSize();
  // First, correlate the components
  const Matrix inSampleMatrix(inSample.getDimension(), size, inSample.getImplementation()->getData());
  const Matrix resultMatrix(cholesky_ * inSampleMatrix);
  SampleImplementation result(size, dimension);
  result.setData(*resultMatrix.getImplementation());
  const Distribution standardMarginal(standardDistribution_.getMarginal(0));
  // Second, apply the commmon marginal distribution
  for (UnsignedInteger i = 0; i < dimension; ++i)
  {
    const Point resultI(standardMarginal.computeCDF(result.getMarginal(i)).asPoint());
    for (UnsignedInteger j = 0; j < size; ++j)
      result(j, i) = resultI[j];
  }
  callsNumber_ += size;
  if (isHistoryEnabled_)
  {
    inputStrategy_.store(inSample);
    outputStrategy_.store(result);
  }
  return result;
}

/* Gradient according to the marginal parameters. Currently, the dependence parameter are not taken into account. */

Matrix InverseNatafEllipticalCopulaEvaluation::parameterGradient(const Point & inP) const
{
  return Matrix(0, getInputDimension());
}

/* Accessor for input point dimension */
UnsignedInteger InverseNatafEllipticalCopulaEvaluation::getInputDimension() const
{
  return cholesky_.getDimension();
}

/* Accessor for output point dimension */
UnsignedInteger InverseNatafEllipticalCopulaEvaluation::getOutputDimension() const
{
  return cholesky_.getDimension();
}

/* Method save() stores the object through the StorageManager */
void InverseNatafEllipticalCopulaEvaluation::save(Advocate & adv) const
{
  EvaluationImplementation::save(adv);
  adv.saveAttribute( "standardDistribution_", standardDistribution_ );
  adv.saveAttribute( "cholesky_", cholesky_ );
}

/* Method load() reloads the object from the StorageManager */
void InverseNatafEllipticalCopulaEvaluation::load(Advocate & adv)
{
  EvaluationImplementation::load(adv);
  adv.loadAttribute( "standardDistribution_", standardDistribution_ );
  adv.loadAttribute( "cholesky_", cholesky_ );
}

END_NAMESPACE_OPENTURNS
