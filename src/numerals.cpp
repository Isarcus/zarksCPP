#include <zarks/io/numerals.h>
#include <zarks/internal/zmath_internals.h>

using std::string;

namespace zmath
{
	static const string numTable = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const int maxNum = numTable.size();

	string ConvertBase(int val, int toBase, int minWidth)
	{
		if (toBase < 2 || toBase > maxNum) return string();

		string result;
		int placeValue = 1;
		while (val > 0)
		{
			int placeNum = val % (placeValue*toBase);
			val -= placeNum;

			placeNum /= placeValue;
			result += numTable[placeNum];

			placeValue *= toBase;
		}

		while ((int)result.size() < minWidth) result += '0';
		reverse(result);

		return result;
	}

	string ConvertBase(std::string val, int fromBase, int toBase, int minWidth)
	{
		if (fromBase > maxNum) return string();

		int decimalVal = 0;
		int placeVal = 1;
		for (int i = val.size() - 1; i >= 0; i--)
		{
			int numeralVal = numTable.find(val[i]);
			if (numeralVal < 0 || numeralVal >= fromBase) return string();

			decimalVal += numeralVal * placeVal;

			placeVal *= fromBase;
		}

		return ConvertBase(decimalVal, toBase, minWidth);
	}

	std::string DToS(int val)
	{
		return ConvertBase(val, 10, 0);
	}


	BaseConverter::BaseConverter(int fromBase, int toBase, int minWidth)
		: fromBase(fromBase)
		, toBase(toBase)
		, minW(minWidth)
	{}

	std::string BaseConverter::operator() (std::string val, int minWidth)
	{
		if (minWidth == -1) minWidth = minW;

		return ConvertBase(val, fromBase, toBase, minWidth);
	}

	std::string BaseConverter::operator() (int val, int minWidth)
	{
		if (minWidth == -1) minWidth = minW;

		return ConvertBase(val, toBase, minWidth);
	}
}
