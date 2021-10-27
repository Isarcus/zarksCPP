#pragma once

#include <zarks/math/VecT.h>

#include <vector>

namespace zmath
{
	class GaussField
	{
	public:
		GaussField(const Vec& sigma, double amplitude, const Vec& center = Vec());
		GaussField(double sigma, double amplitude, const Vec& center = Vec());

		void SetAmplitude(double val);
		void SetSigma(double val);
		void SetSigma(const Vec& val);
		void SetCenter(const Vec& val);

		double Sample(const Vec& pos) const;
		double Sample(double x, double y) const;

		std::vector<std::pair<Vec, double>> Points(double radius, int resolution) const;
		std::vector<std::pair<VecInt, double>> Points(int radius) const;

	private:
		Vec center;
		Vec sigma;
		double amplitude;

		Vec computeDimensionalWeights(double x, double y) const;
		static double computeWeight(double distance, double sigma);
	};
}
