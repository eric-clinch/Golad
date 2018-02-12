
#ifndef STATS_H
#define STATS_H

#include <math.h>
#include <assert.h>

namespace Stats {
	double getProbabilitySuccessfulBernoulli(int positiveSamples, int numSamples);
	double getStandardDeviation(int positiveSamples, int numSamples);
	double normalCDF(double z);
}

// we will call a Bernoulli distribution X "successful" if the probability p = P[X=1]
// is greater than .5; that is, if it is more likely we'll get a positve result than
// a negative result. Given that we've taken n samples from X and x of those were positive
// samples, this function computes the probability that X is a successful Bernoulli using
// the Central Limit Theorem. (x, n) = (positiveSamples, numSamples)
double Stats::getProbabilitySuccessfulBernoulli(int positiveSamples, int numSamples) {
	double sampleMean = (double)positiveSamples / numSamples;
	double sampleStd = getStandardDeviation(positiveSamples, numSamples);
	double cdfInput = (sampleMean - 0.5) * sqrt(numSamples) / sampleStd;
	return normalCDF(cdfInput);
}

// computes the standard deviation of n samples from a Bernoulli distribution,
// where x of these samples were positive. (x, n) = (positiveSamples, numSamples)
double Stats::getStandardDeviation(int positiveSamples, int numSamples) {
	assert(numSamples > 1);
	double standardMean = (double)positiveSamples / numSamples;
	double positiveDifference = 1 - standardMean; // difference between a positive example and the mean
	double negativeDifference = 0 - standardMean; // difference between a negative example and the mean
	double positiveDifferenceSquaredSum = positiveSamples * (positiveDifference * positiveDifference);
	double negativeDifferenceSquaredSum = (numSamples - positiveSamples) * (negativeDifference * negativeDifference);
	double differenceSquaredSum = positiveDifferenceSquaredSum + negativeDifferenceSquaredSum;
	double result = sqrt(differenceSquaredSum / (numSamples - 1));
	return result;
}

// computes the cdf of the standard normal distribution
// taken from https://stackoverflow.com/questions/2328258/cumulative-normal-distribution-function-in-c-c
double Stats::normalCDF(double z) {
	return 0.5 * erfc(-z / sqrt(2));
}

#endif // !STATS
