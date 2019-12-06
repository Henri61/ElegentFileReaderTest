#pragma once

#include <intrin.h>
#include <stdint.h>

namespace score {

/// Various Caculation States for an Attribute that will calculate itself (Column and Scalar).
/// Type specified to be safe - forward declared in Scalar.h
enum CalculationState : uint32_t {
	NotCalculated = 0xfff80000,
	NotCalculatedSaturn = 0xfff00001,  // Used for Score-mode attributes invoked from Saturn
	NoAvg,
	Busy
};


/// CalculationState bit patterns stored in the 8 bytes of a double.
/// When the bit pattrn is stored in a double it will have the representation of Nan.
/// - This union relies on double being 8 bytes.
union DoubleState {
public:
	/// Fill the data with the NotCalculated CalculationState bit pattern.
	/// The bit pattern goes to the high order dword for 64 bits and to both dwords for 32 bits.
	static __forceinline void _vectorcall set_not_calculated(double *values, size_t size, CalculationState value) {
#ifdef _WIN64
		__stosq(
			reinterpret_cast<unsigned long long*>(values),
			DoubleState(value).as_uint64_state_in_high_dword(),
			sizeof(double) / sizeof(uint64_t) * size);
#else
		__stosd(
			reinterpret_cast<unsigned long*>(values),
			value,
			sizeof(double) / sizeof(uint32_t) * size);
#endif // WIN64
	}
	static inline void _vectorcall set_not_calculated_score(double *values, size_t size) {
		set_not_calculated(values, size, CalculationState::NotCalculated);
	}
	static inline void _vectorcall set_not_calculated_saturn(double *values, size_t size) {
		set_not_calculated(values, size, CalculationState::NotCalculatedSaturn);
	}

	/// Extract the CalculationState bit pattern from a double.
	static const CalculationState _vectorcall get_state(const double& value) {
		return reinterpret_cast<const DoubleState*>(&value)->v.state_;
	}

	// Test for a specific state - 
	// At the moment, we could code 'get_state == State' but in my experimentation, 
	// get_state might get more complicated to check for calulated 1st. Then we need this special is<State>() function - I just left this in for now
	template<CalculationState State>
	static const bool is(const double& value) {
		return reinterpret_cast<const DoubleState*>(&value)->v.state_ == State;
	}

    /// Store a CalculationState in double value as a Nan
	template<CalculationState State>
	static void _vectorcall set(double& value) {
		reinterpret_cast<DoubleState*>(&value)->v.state_ = State;
	}

    /// Get the CalculationState as a double, with the state in the high order dword of the double.
    double _vectorcall as_double_state_in_high_dword() { return dValue_; }
    /// Get the CalculationState as an unsigned 64 bit int, with the state in the high order dword of the uint64.
    uint64_t as_uint64_state_in_high_dword() { return i64Value_; }

    /// Set the CalculationState in the high order dword.
	DoubleState(CalculationState state) {
		v.spacer_ = 0;
		v.state_ = state;
	}
	
private:
    /// Remember: DoubleSate a union
    uint64_t i64Value_;
	double dValue_;
	struct {
        uint32_t spacer_;
        union {
            CalculationState state_;
            uint32_t lValue_;
        };
	} v;
};

}