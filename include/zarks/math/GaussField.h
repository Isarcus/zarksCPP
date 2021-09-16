#pragma once

#include <zarks/math/VecT.h>

#include <vector>

namespace zmath
{
	class GaussField
	{
	public:
		GaussField(Vec sigma, double amplitude, Vec center = Vec());
		GaussField(double sigma, double amplitude, Vec center = Vec());

		void SetAmplitude(const double& val);
		void SetSigma(const double& val);
		void SetSigma(const Vec& val);
		void SetCenter(const Vec& val);

		double Sample(const Vec& pos) const;
		double Sample(const double& x, const double& y) const;

		std::vector<std::pair<Vec, double>> Points(double radius, int resolution) const;
		std::vector<std::pair<VecInt, double>> Points(int radius) const;
		void Points(std::vector<std::pair<Vec, double>>& points, double radius, int resolution) const;
		void Points(std::vector<std::pair<VecInt, double>>& points, int radius) const;
		int PointsLen(int radius, int resolution) const;
		int PointsLen(double radius) const;

	private:
		Vec center;
		Vec sigma;
		double amplitude;

		Vec computeDimensionalWeights(const double& x, const double& y) const;
		static double computeWeight(const double& distance, const double& sigma);
	};
}
