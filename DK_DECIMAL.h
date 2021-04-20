#pragma once

#include <boost/multiprecision/cpp_dec_float.hpp>

typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float<10> > DK_DECIMAL; //boost multiprecision

//////////////////////////////////////////////////////////////////////////////////////
inline CString ToString(DK_DECIMAL decimal, int nPrecision)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(nPrecision) << decimal;
	return CString(ss.str().c_str());
	//The C++ Standard Library P.698 참조
	//return CString(decimal.str(14).c_str());
}

inline CString ToString(DK_DECIMAL decimal)
{
	std::stringstream ss;
	ss << decimal;
	return CString(ss.str().c_str());
	//The C++ Standard Library P.698 참조
	//return CString(decimal.str(14).c_str());
}

//////////////////////////////////////////////////////////////////////////////////////
inline DK_DECIMAL ToDecimal(const VARIANT& var)
{
	DK_DECIMAL val = var.decVal.Lo64;
	val *= (var.decVal.sign == 128)? -1 : 1;
	val /= pow(10.0, var.decVal.scale); 
	return val;
}

template<class T>
inline T DecimalTo(DK_DECIMAL dVal)
{
	return dVal.convert_to<T>();
}

//////////////////////////////////////////////////////////////////////////////////////
inline DK_DECIMAL ToDecimal(const CStringA& str)
{
	if(str.IsEmpty())
		return 0;

	DK_DECIMAL val;
	try
	{
		val.assign(str);
	}
	catch (std::runtime_error&)
	{
		TRACE(_T("ToDecimal invalid expression(%s)\n"), str);
		ASSERT(FALSE);
		return 0;
	}
	return val;
}


inline DK_DECIMAL ToDecimal(const CStringW& str)
{
	if (str.IsEmpty())
		return 0;

	DK_DECIMAL val;
	return ::ToDecimal(CStringA(str));
}


inline DK_DECIMAL RoundUp(DK_DECIMAL val, double dig)
{
	return (std::floor(val.convert_to<double>() * pow(10, dig) + 0.5) / pow(10, dig));
}

